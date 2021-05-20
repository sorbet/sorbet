# Compiler Caveats

## Typechecking

The compiler will generate code that unconditionally checks signatures for
compiled methods and type annotations in the body of a compiled method. This is
true even of `sig` blocks that specify `checked(:never)`. Sorbet
unconditionally checks types because those same checks are used to enable fast
execution paths within methods that the compiler is aware of.


## Runtime

Loading shared objects produced by the compiler requires a little bit of setup.
As it's not possible for us to bake-in the path to the ruby source file at
compile time, we expect that this is passed through to the shared object via the
`$__sorbet_ruby_realpath` global. The effect of this is is that you either need
to patch `require` to supply this value at runtime, or provide it directly
before loading the shared object.

In our tests, we patch require and provide this value before loading the shared
object: https://github.com/stripe/sorbet_llvm/blob/c6f55f98/test/patch_require.rb#L31.

The compiler assumes that you have a monkey patch for `BasicObject#nil?` that
behaves identically to `Kernel#nil?`. If you do not have this, calling `.nil?`
on an object that does respond to `nil?` or defines it in some other way will
not throw an exception (as the interpreter would do) but will instead evaluate
to whether the receiver is `nil` or not.

## Thread safety

The compiler currently assumes that the artifacts it produces are used in a single-threaded context.
The most significant place that this assumption is leveraged is during shared object
initialization, where both global values are shared as weak symbols and static
values linked into the vm are initialized on first access.

This can be mitigated by ensuring that only one shared object is loaded at a time,
which Stripe enforces with a lock around `require` statements in the autoloader. As
this assumption is present in Stripe's environment, we don't currently have plans to
make initialization of shared objects work in the context of multiple threads.

## The ruby stack

There are a few different features that cause compiled code to interact directly
with the ruby stack:

* Accurate line numbers in stack traces
* Closure variable storage
* Method arguments for sends that will go through the vm

In order to to support these features, we edit the top of the ruby stack when a
compiled function is entered, making space for locals and populating the iseq
pointer. The iseq pointer that is written into the stack it is allocated when
the compiled module is first loaded, and has enough fields filled out to inform
the vm of the stack layout required, and how to reconstruct a line number at a
given point in time.

One problem with this approach is that all of the functions emitted by the
compiler now assume that it's fine to edit the top of the ruby stack. In the
case where the function has been called through the vm this assumption is valid,
but if the function is called directly from some other context a stack frame
must be pushed to avoid corrupting the caller. This comes up in a few different
cases:

1. Class and method `<static-init>` functions, which don't correspond to real
   functions that the vm will ever call, but that get called during compiled
   module initialization:
   * https://github.com/stripe/sorbet_llvm/blob/d963311f/compiler/IREmitter/NameBasedIntrinsics.cc#L98-L101
   * https://github.com/stripe/sorbet_llvm/blob/d963311f/compiler/IREmitter/Payload/codegen-payload.c#L1213-L1219
2. Final methods that are called directly would edit the frame of the calling
   context and corrupt any locals of that context if a frame isn't pushed:
   * https://github.com/stripe/sorbet_llvm/blob/d963311f/compiler/IREmitter/sends.cc#L131
   * https://github.com/stripe/sorbet_llvm/blob/d963311f/compiler/IREmitter/Payload/codegen-payload.c#L1199-L1211
3. Blocks that are inlined into the body of intrinsics that support direct block
   calls will also need their own frames, as raising exceptions from that
   context without them would put the vm into a strange state.
   * https://github.com/stripe/sorbet_llvm/blob/d963311f/compiler/IREmitter/Payload/codegen-payload.c#L671-L689
4. Block functions extracted from `rescue` blocks need to have a special frame
   pushed that contains enough space for one local that holds `$!`:
   * https://github.com/stripe/sorbet_llvm/blob/d963311f/compiler/IREmitter/Payload/patches/vm_insnhelper.c#L21-L22
