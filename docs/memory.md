# Where the memory goes in a batch typecheck

Notes from a pass over Sorbet's peak resident memory on a 600k-file Ruby codebase (16 threads, no
`--cache-dir`, release build with mimalloc). They record what the memory is, what was tried, what
worked, and what did not, so that the next pass does not have to rediscover it. Every number here was
measured; the ones marked "estimated" were not.

## Shape of the run

Resident memory is monotone in a batch run: `main/main.cc` sets mimalloc's `purge_delay` to -1, so a
page the process has touched is never handed back to the OS before exit, and a page that was freed is
reused only for objects of the size class it last held. The peak is therefore the end of the last phase,
and a change lowers the peak only if it stops memory being committed in the first place. Freeing
something late buys nothing unless the phase after it allocates objects of the same sizes.

Peak resident memory when stopping after each phase, from the same binary:

| stopped after | peak RSS |
|---|---|
| namer | 15.87 GiB |
| resolver | 17.79 GiB |
| typecheck (the full run) | 18.00 GiB |

The typecheck phase holds a `const GlobalState`, so anything it adds is either a per-thread transient or
something read back from disk. It used to add 1.9 GiB; almost all of that was file text being read back
(items 4 and 5 below), and it now adds about 0.2 GiB.

Useful reflex when the peak grows in a phase that should not be allocating: build mimalloc with `MI_STAT=2`
(`MI_EXTRA_CPPDEFS` in its cmake rule), run with `MIMALLOC_SHOW_STATS=1`, and diff the per-size-class live bytes
against a run stopped one phase earlier. The size of the blocks says what they are. That is what found items
4 and 5: 3.1 GiB of live blocks the size of source files, appearing during typecheck.

## What the 22 GiB was made of

| term | bytes | how it was measured |
|---|---|---|
| ASTs, live from indexing to exit | 10-13 GiB | index-phase growth minus the terms below; estimated |
| file source text | 3.13 GiB | `types.input.bytes` counter: 3,360,691,094 over 600,283 files |
| prefaulted symbol and name tables | ~3.2 GiB | RSS at 4 seconds, before indexing had produced much |
| namer and resolver additions | ~2.0 GiB | difference between `--stop-after=namer` and `--stop-after=resolver` |
| typecheck transients and timings | ~1.9 GiB | difference between `--stop-after=resolver` and the full run |

Sizes that drive the arithmetic, from the `CheckSize` assertions in the headers: `Method` 136,
`ClassOrModule` 128, `Field` 56, `TypeParameter` 56, `File` 120, `UTF8Name` 16, `UniqueName` 12,
`ConstantName` 4, `NameHash::Bucket` 8; `ast::Send` 56, `MethodDef` 64, `ClassDef` 120, `InsSeq` 56,
`Hash` 56, `Array` 48, `Block` 40, `Local`/`Literal`/`ConstantLit` 16.

## What landed

In the order it was done; each figure is from runs interleaved with the state before it.

1. **Prefault only the part of each preallocated table the hint asked for, and size the name hash table
   for its load factor.** `preallocateTables` rounds every `--reserve-*-capacity` hint up to a power of
   two, and the prefault thread used to back the whole reservation with pages; a hint of 5.3M methods
   made 8.4M methods' worth of table resident. `preallocatedTableRanges` now covers the hinted prefix.
   The name hash table was sized at twice the next power of two above the name table's capacity, which
   left it between a quarter and half full: 64M eight-byte buckets for 19M names. `NameHash::sizeFor`
   now takes the next power of two above one and a half times the name count, so the table is at most
   two thirds full; bucket order is not observable. Together -542 MiB. (Pass the counted sizes as hints,
   not powers of two: a power-of-two hint gets nothing from the first half of this.)
2. **Drop each file's text once it has been indexed.** Sorbet used to hold every file's source from the
   moment it was read until exit. After indexing, the only thing that wants it is rendering an error in
   it, true of a few thousand files and not of all of them. `File::releaseSource` drops the text and
   `File::source()` reads it back on demand; everything that only wanted a length asks
   `File::sourceSize()`, which never reads anything back. Only text known to be exactly what is on disk
   is dropped, which is why the constructor takes that as a flag and why `FileSystem` says whether it
   answers from the path it was handed. -1.6 GiB of the 3.1 GiB of text: the phases after indexing take
   about half of what this frees, for the size-class reason above.
3. **Record what each file references only when something will read it.** The visibility checker kept,
   for every file, the symbols and packages it references; only `--gen-packages` and the language
   server's "add missing export" code action ever read them. -95 MiB.
4. **Compare three bytes instead of reading a file back.** To decide where an implicit return points, the
   CFG builder asks whether the last three bytes of a method's or block's loc are `end` or `}`. It did
   that through `Loc::source()`, and nearly every file has a method or a block with an empty body, so
   with item 2 in place nearly the whole codebase's text came back during typechecking.
   `Loc::sourceEquals` compares those bytes without materialising the text. -887 MiB.
5. **Compute two inference error locs only once there is an error to put them on.** `reportMissingKwargs`
   trimmed the argument list's parentheses before checking whether any keyword argument was missing, and
   the untyped-return error truncated its loc to the first line before asking whether that error is
   reported at this file's strictness. Both reads brought a whole file back. -873 MiB.

Together: **21.94 GiB to 18.00 GiB (-18.0%)**, user CPU within 1.5% and wall time no worse, over three
interleaved runs of each binary. Error output is byte-identical, checked over a 1,910-file subtree at
`--typed=strict` (121,746 errors with snippets) and over the whole codebase.

## Traps, so that they are not stepped in twice

- **Any call that materialises a file's text on a whole-corpus path undoes item 2.** After item 2,
  `Loc::source()`, `Loc::truncateToFirstLine()`, `Loc::position()` and `File::lineBreaks()` all read the
  file back in and keep it. Sorting the typecheck queue by file size did exactly that and read all 600k
  files back, which made item 2 look like it did nothing on its first measurement; items 4 and 5 are two
  more of these, found later and only by the allocator's statistics. `sortBySize`, the packager's
  empty-file check and the desugarers' receiver-loc check use `sourceSize()` instead.
- **Mapping each file instead of copying it aborts the run.** 600k live mappings exceed the kernel's
  `vm.max_map_count` (131,072 by default); `mmap` starts failing and mimalloc's new-handler aborts from
  whatever allocates next. Dropping the text and reading it back needs no mappings.
- **This machine's load drifts 10-15% over tens of minutes.** Wall time is only comparable between
  runs interleaved in the same window. Two of the rejections below were first "measured" as
  regressions that turned out to be drift.
- **Measure the binary you are about to ship.** One number in an earlier draft of these notes came from a
  build that still had a rejected experiment in it, and was 250 MiB too good.

## Measured and rejected

| change | result | why it does not help |
|---|---|---|
| free each AST after typechecking it (instead of the deliberate leak) | +85 MiB, +6% wall | RSS is monotone, so freeing in the last phase lowers nothing; the destructor walk is not free |
| mimalloc default purging instead of never | -112 MiB, +1.3% wall; with item 2: -180 MiB, +10% wall | purging madvises away pages the next phase wants back |
| immediate purging (`purge_delay=0`) | -0.6 GiB, +19% wall | same, more so |
| `mi_collect(true)` on every worker at the index boundary | 0 MiB, +4% wall | with purging off there is nothing for it to do; it only abandons pages |
| no arena eager commit | -200 MiB, +50% wall | |
| 64 MiB arena reserve instead of 1 GiB | -170 MiB, +20% wall | |
| no large OS pages | 0 MiB | |
| 8 threads instead of 16 | -350 MiB, +33% wall | per-thread state is not where the memory is |
| start an indexing thread over on a fresh GlobalState past 250k names | -140 MiB, +4.6% wall | the worker name tables are freed at the end of indexing anyway and the peak is at the end of typecheck |
| `InsSeq`/`Array` inline capacity 4 to 2 | -77 MiB, +6% wall | the spills cost more than the inline space saves |
| gate `Timer` args on `--web-trace-file` (only a trace reads them) | -16 MiB | the per-file timing records are not where the memory is |
| free a resolved constant's unresolved original when it was written without a scope | -250 MiB, +7% wall | the trees are leaked at exit, so the millions of frees are work the run does not otherwise do |

## Not tried, with the reason

- **A per-file AST arena with 32-bit handles** would remove most of the AST's per-node overhead and is the
  right long-term answer for the largest term above, but it touches every consumer of `ExpressionPtr`.
- **Typechecking each package stratum as soon as it is resolved** would move the peak, but the resolver
  is a global fixed point, so it changes which stubs exist and therefore the error set.
- **Collapsing resolved `ConstantLit`s to their `KnownSymbol` form after the resolver** (estimated
  ~0.9 GiB of `UnresolvedConstantLit` chains) changes what `cfg/builder` emits for a qualified constant's
  scope, so it is not output-preserving as it stands; the depth-one subset is, and is worth measuring.
- **`ClassOrModule::members_` as a sorted vector** puts `findMember` on the dispatch path and makes
  insertion linear on classes with thousands of members.
- **`Method::ParametersStore` inline capacity 2 to 1**: every method gets a block parameter, so a
  one-argument method already has two and the spill would be the common case.
