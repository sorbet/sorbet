# typed: true

chain1 = Enumerator::Chain.new([1,2], [:foo,:bar])

T.assert_type!(chain1, T::Enumerator::Chain[T.any(Integer, Symbol)])
T.assert_type!(chain1, T::Enumerable[T.any(Integer, Symbol)])

chain2 = [1,2].chain([:foo,:bar])

T.assert_type!(chain2, T::Enumerator::Chain[T.any(Integer, Symbol)])
T.assert_type!(chain2, T::Enumerable[T.any(Integer, Symbol)])
