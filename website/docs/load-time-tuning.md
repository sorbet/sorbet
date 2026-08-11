---
id: load-time-tuning
title: Load-time Tuning
---

`sorbet-runtime` uses lazy initialization internally, sometimes to eliminate load-time cycles, and sometimes to keep development code loading fast, but some applications benefit from **eager loading**, instead of lazy loading.

These methods force that deferred work to happen eagerly:

```ruby
# Run all `sig {...}` blocks, redefining all associated methods with runtime checking
# (or dropping the first-call wrapper, for `.checked(:never)` sigs)
T::Utils.run_all_sig_blocks

# The same, but for `T.type_alias` blocks
T::Utils.run_all_type_alias_blocks

# Force all T::Struct classes to generate their specialized serialization methods
T::Utils.eagerly_define_all_lazy_props_methods!
```

For certain applications, like HTTP services, these methods mitigate first-call or first-request latency spikes. It's the same idea behind projects like [nakayoshi_fork] and patterns like zeitwerk's `eager_load`.

Call these methods **after** all application code has been loaded (e.g., in a Rails initializer or a `before_fork` hook) but **before** forking workers (so the initialized state is shared across processes via copy-on-write).

There is also a method that frees memory at that same point, instead of forcing work to happen early:

```ruby
# Free the signatures whose runtime checks never run
T::Utils.drop_unchecked_sigs!
```

[nakayoshi_fork]: https://github.com/ko1/nakayoshi_fork

## `T::Utils.run_all_sig_blocks`

```ruby
T::Utils.run_all_sig_blocks
```

Method signatures (`sig { ... }`) are lazily evaluated: the block inside a `sig` doesn't run until the method is first called. This means the first call to any sig'd method is slower than subsequent calls.

`run_all_sig_blocks` forces every pending sig block to evaluate immediately, so that the first real call to each method has no extra overhead.

## `T::Utils.run_all_type_alias_blocks`

```ruby
T::Utils.run_all_type_alias_blocks
```

Type aliases created with `T.type_alias { ... }` lazily compute the aliased type the first time it's needed (either for runtime type checking or for reflection). This method forces all type alias objects in the process to resolve that computation eagerly.

## `T::Utils.eagerly_define_all_lazy_props_methods!`

```ruby
T::Utils.eagerly_define_all_lazy_props_methods!
```

Classes that include `T::Props::Serializable` (including `T::Struct`) generate specialized `serialize` and `from_hash` methods using codegen for runtime performance (mostly: to avoid contention for VM-level method call caches). These specialized methods are usually generated lazily on the first call to the ser/de methods. This method eagerly generates these specialized methods.

> **Note**: The `serialize` and `from_hash` methods on `T::Struct` have a number of [gotchas and legacy behaviors](tstruct.md#serialize-and-from_hash-converting-tstruct-to-and-from-hash).

## `T::Utils.drop_unchecked_sigs!`

```ruby
T::Utils.drop_unchecked_sigs!
```

A `sig` that never checks anything at runtime still costs memory: `sorbet-runtime` keeps its `Signature` object so that tools can introspect the method. These are the sigs marked `.checked(:never)`, plus the sigs marked `.checked(:tests)` outside of a test environment. In a large application they can hold many megabytes.

This frees the signatures that `sorbet-runtime` holds already. There is no way back: nothing can rebuild a freed signature, because its `sig` block has already run. Signatures built after the call are recorded as usual: a `sig` block that runs later, either on the first call to its method or through `T::Utils.run_all_sig_blocks`, and the first call to a method through an alias. Call the method again to free those too:

```ruby
T::Utils.drop_unchecked_sigs!

# … the application autoloads more code …

T::Utils.run_all_sig_blocks   # evaluates the new sig blocks
T::Utils.drop_unchecked_sigs! # frees the ones that never check anything
```

Runtime dispatch never uses these signatures, so calls behave the same. Methods with such sigs keep no wrapper at all, so `Method#arity`, `Method#source_location` and `Method#parameters` report the same values as before. The option only trades away introspection:

- `T::Utils.signature_for_method` returns `nil` for those methods.
- Final method violations lose the `Made final here:` source location.

Signatures on `abstract` methods are never dropped, because `T::AbstractUtils` reads them at any time.

> **Note**: An application that prepends `T::CompatibilityPatches::MethodExtensions` must take the `Method` object after the `sig` block has run. A handle taken from the first-call wrapper before that reports the parameters of the wrapper, because the patch can no longer find the dropped signature.

> **Note**: Call this only after the application has loaded all of its code, in the same way as `T::Utils.run_all_sig_blocks`. A `sig` block that runs later cannot see a dropped signature on the method it overrides, so `sorbet-runtime` skips the override checks against that parent signature.
