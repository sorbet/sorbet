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

## Thread safety

The compiler currently assumes that the artifacts it produces are used in a single-threaded context.
The most significant place that this assumption is leveraged is during shared object
initialization, where both global values are shared as weak symbols and static
values linked into the vm are initialized on first access.

This can be mitigated by ensuring that only one shared object is loaded at a time,
which Stripe enforces with a lock around `require` statements in the autoloader. As
this assumption is present in Stripe's environment, we don't currently have plans to
make initialization of shared objects work in the context of multiple threads.
