# typed: strict

T.assert_type!(GC.stat, T::Hash[Symbol, Integer])
T.assert_type!(GC.stat({}), T::Hash[Symbol, Integer])
T.assert_type!(GC.stat(:count), Integer)
T.assert_type!(GC.stat[:count], T.nilable(Integer))
