# Rust dependencies

Rust dependencies are managed via [`cargo raze`](https://github.com/google/cargo-raze)
which has a lot of sharp edges because cargo and bazel fundamentally want to
manage dependencies in a different way. This guide both explains the baseline
mechanics of what an ideal upgrade looks like, and what sharp edges you might
encounter.

## Raze's `Cargo.toml`

Raze manages dependencies by reading `Cargo.toml` and `Cargo.lock` and then
spitting out `.BUILD` files for each crate under the `remote` directory. This
means that the `Cargo.toml` in this directory needs to contain the superset
of dependencies that our Rust dependencies have. Today, there are two crates
that have their dependencies vendored here:

* `librubyfmt` - the Rubyfmt library crate
* `ripper_deserialize` - a crate for making a Ruby C `VALUE` which represents a
   ripper tree deserializable by serde.

If you look at [`librubyfmt`'s Cargo.toml](https://github.com/penelopezone/rubyfmt/blob/trunk/librubyfmt/Cargo.toml#L11-L19)
and [`ripper_deserialize`'s Cargo.toml](https://github.com/penelopezone/rubyfmt/blob/trunk/librubyfmt/ripper_deserialize/Cargo.toml#L9-L11)
you will see that the `Cargo.toml` in this directory has dependencies that match,
with comments outline which crate each dependency comes from.

When `cargo raze` is run from this directory, this will cause crate `BUILD`
files to be generated for each entry in that dependencies section.

## Updating dependencies

This will typically become relevant at Rubyfmt upgrade time. It is worth noting
that Rubyfmt's dependencies change much more slowly than the library itself. In
particular we have a very low expectation that our Rust dependencies will change
for a good while going forward. Probably not until the 2021 edition, if we decide
we need that.

Say one of the above libraries' `Cargo.toml`s changes. The typical workflow would
be:
* to copy/paste the `[dependencies]` section from the Cargo.toml of that library to
  this directory's `Cargo.toml`
* run `cargo generate-lockfile` in this directory
* run `./tools/scripts/cargo_raze.sh` in the top level of the repo (note: this
  is not managed by bazel, you **must** have installed `cargo raze` in your local
  rust toolchain for this to work)

Rebuild rubyfmt to check everything is working.

There are however some gotchas

## The `raze.crates` section

Cargo raze by default enables all features for the crates it pulls in. This
means that most of the time things will "just work". However, some crates have
configurations that are needed that are not part of the `features` section of
their configuration. An example of this is:

```
[raze.crates.proc-macro2.'1.0.10']
additional_flags = [
  "--cfg=use_proc_macro",
]
```

Without this, we get a nasty compilation error while trying to build librubyfmt:

```
    |
108 |         Self::new2(stream.into())
    |                    ^^^^^^^^^^^^^ the trait `std::convert::From<proc_macro::TokenStream>` is not implemented for `proc_macro2::TokenStream`
    |
    = help: the following implementations were found:
              <proc_macro2::TokenStream as std::convert::From<proc_macro2::TokenTree>>
    = note: required because of the requirements on the impl of `std::convert::Into<proc_macro2::TokenStream>` for `proc_macro::TokenStream`

error[E0277]: the trait bound `proc_macro2::TokenStream: std::convert::From<proc_macro::TokenStream>` is not satisfied
    --> external/raze__syn__1_0_17/src/parse.rs:1102:21
     |
1102 |         self.parse2(proc_macro2::TokenStream::from(tokens))
     |                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ the trait `std::convert::From<proc_macro::TokenStream>` is not implemented for `proc_macro2::TokenStream`
     |
     = help: the following implementations were found:
               <proc_macro2::TokenStream as std::convert::From<proc_macro2::TokenTree>>
     = note: required by `std::convert::From::from`

error: aborting due to 2 previous errors

For more information about this error, try `rustc --explain E0277`.
```

All Rubyfmt release tars (which vendor Ruby, and are the only ones which can
be `http_archive`d for bazel builds in this repo) go through very extensive
testing. It is unlikely to be a real error in Rubyfmt's build, but instead a
missing rust compilation flag if you see an error like this.

In order to fix it, the cleanest way to solve this is to look at the
crate you're building (in this case, `proc_macro2`), and find it's compilation
flags in a normal `cargo build` of `librubyfmt` or `ripper_deserialize`.

You should:

* `git clone https://github.com/penelopezone/rubyfmt`
* `cd rubyfmt`
* `git checkout v(matching tag to the http archive)`
* `cd librubyfmt`
* `cargo build -v`

and see which flags are being omitted vs a bazel build.

This process, for example, will not work for *really* complicated libraries like
`jemallocator`, which I had to disable entirely for `librubyfmt` in bazel.
`jemallocator`'s `build.rs` is too complicated to build in the bazel environment
cleanly.

Fortunately, Rubyfmt's dependencies change rarely, and so this is unlikely to be
an ongoing problem.
