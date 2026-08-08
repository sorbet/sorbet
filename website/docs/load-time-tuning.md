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
