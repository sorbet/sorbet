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
object: https://github.com/sorbet/sorbet/blob/aa8434150/test/patch_require.rb#L31.

The compiler assumes that you have a monkey patch for `BasicObject#nil?` that
behaves identically to `Kernel#nil?`. If you do not have this, calling `.nil?`
on an object that does respond to `nil?` or defines it in some other way will
not throw an exception (as the interpreter would do) but will instead evaluate
to whether the receiver is `nil` or not.

## Thread safety

The compiler currently assumes that the artifacts it produces are used in a
single-threaded context.  The most significant place that this assumption is
leveraged is during shared object initialization, where both global values are
shared as weak symbols and static values linked into the vm are initialized on
first access.

This can be mitigated by ensuring that only one shared object is loaded at a
time, which Stripe enforces with a lock around `require` statements in the
autoloader. As this assumption is present in Stripe's environment, we don't
currently have plans to make initialization of shared objects work in the
context of multiple threads.

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

1.  Class and method `<static-init>` functions, which don't correspond to real
    functions that the vm will ever call, but that get called during compiled
    module initialization:
    - https://github.com/sorbet/sorbet/blob/aa84341504/compiler/IREmitter/NameBasedIntrinsics.cc#L109-L112
    - https://github.com/sorbet/sorbet/blob/aa84341504/compiler/IREmitter/Payload/codegen-payload.c#L1213-L1219
2.  Final methods that are called directly would edit the frame of the calling
    context and corrupt any locals of that context if a frame isn't pushed:
    - https://github.com/sorbet/sorbet/blob/aa84341504/compiler/IREmitter/sends.cc#L134-L135
    - https://github.com/sorbet/sorbet/blob/aa84341504/compiler/IREmitter/Payload/codegen-payload.c#L1895-L1907
3.  Blocks that are inlined into the body of intrinsics that support direct
    block calls will also need their own frames, as raising exceptions from that
    context without them would put the vm into a strange state.
    - https://github.com/sorbet/sorbet/blob/aa84341504/compiler/IREmitter/Payload/codegen-payload.c#L785-L803
4.  Block functions extracted from `rescue` blocks need to have a special frame
    pushed that contains enough space for one local that holds `$!`:
    - https://github.com/sorbet/sorbet/blob/aa84341504/compiler/IREmitter/Payload/patches/vm_insnhelper.c#L21-L22

## Patches to the Ruby VM

We maintain a set of patches to the Ruby VM, which are necessary to support the
compiler, in Stripe's Ruby repo:

http://go/git/stripe-private-oss-forks/ruby

However, in order to keep the the Sorbet Compiler build self-contained and
runnable outside of Stripe infrastructure, we do not reference the internal git
repository directly from our build. Instead, we generate diffs against vanilla
Ruby 2.7.2, and place them in `third-party/ruby`. The patches are applied in
`third_party/externals.bzl`.

### Standards for review of Ruby VM patches

Patching the Ruby VM is a somewhat risky business, for the following reasons:

1. It is large, complex, and sometimes sparsely documented.
2. Bad VM changes can sometimes have very subtle effects, and may only manifest
   in obscure corner cases that, no matter how much testing we do up front, we
   may not catch until things are in production.

Thus we apply the following general standards for pull requests that modify the
VM.

**If a VM patch very obviously does not affect the behavior of the VM except in
the presence of compiled Ruby code**, then it does not require any special
scrutiny beyond what we would normally apply in PR review. The terms "obviously"
and "behavior" are not precisely defined here; when in doubt, make sure to
discuss the patch with the team.

Here is an example of a good argument that something "obviously does not affect
behavior" when the compiler is not in use: the patch that introduces
`VM_METHOD_TYPE_SORBET` only extends an existing internal enum for method types,
and adds cases to various functions for that enum value. Nowhere in the VM is a
method of type `VM_METHOD_TYPE_SORBET` ever actually constructed; this only
happens in code generated by the compiler. Therefore, the VM's behavior should
be totally unchanged.

**If a VM patch _does_ affect the behavior of the VM even in the absence of
compiled Ruby code**, then the PR must clearly explain why we believe the patch
is safe, _and_ the PR author should verify that Ruby's own tests
(`make test-all`) still pass with the patch applied. (Note, however, that
"`make test-all` passes" is not a sufficient argument in itself; in the past,
buggy VM patches have managed to slip by this.)
