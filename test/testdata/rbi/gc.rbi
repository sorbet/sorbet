# typed: true

T.assert_type!(
  GC.stat_heap,
  T::Hash[Integer, T::Hash[Symbol, Integer]]
)

T.assert_type!(
  GC.stat_heap(0),
  T::Hash[Symbol, Integer]
)

T.assert_type!(
  GC.stat_heap(0, :slot_size),
  Integer
)

all_stats = T.let({}, T::Hash[T.untyped, T.untyped])

T.assert_type!(
  GC.stat_heap(nil, all_stats),
  T::Hash[T.untyped, T.untyped]
)

stats = T.let({}, T::Hash[T.untyped, T.untyped])

T.assert_type!(
  GC.stat_heap(0, stats),
  T::Hash[T.untyped, T.untyped]
)
