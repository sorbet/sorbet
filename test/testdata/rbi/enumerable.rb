# typed: true

# You can return any Comparable in the block *_by (and String is one)
[1, 3, 20].min_by {|n| n.to_s}
[1, 3, 20].max_by {|n| n.to_s}
[1, 3, 20].minmax_by {|n| n.to_s}
[1, 3, 20].sort_by {|n| n.to_s}

# You can return an Array of Comparables in the block for *_by
[1, 3, 20].min_by {|n| [n.to_s, 1]}
[1, 3, 20].max_by {|n| [n.to_s, 1]}
[1, 3, 20].minmax_by {|n| [n.to_s, 1]}
[1, 3, 20].sort_by {|n| [n.to_s, 1]}

# You can return an Array with an Array of Comparables in the block for *_by
[1, 3, 20].min_by {|n| [n.to_s, [1, 2]]}
[1, 3, 20].max_by {|n| [n.to_s, [1, 2]]}
[1, 3, 20].minmax_by {|n| [n.to_s, [1, 2]]}
[1, 3, 20].sort_by {|n| [n.to_s, [1, 2]]}

T.assert_type!([1].lazy, Enumerator::Lazy[Integer])
T.assert_type!([1, 2].filter_map { |x| x.odd? ? x.to_f : x.to_s }, T::Array[T.any(Float, String)])
T.assert_type!([1, 2, "3", nil].tally, T::Hash[T.nilable(T.any(Integer, String)), Integer])

# There are 3 different ways to call all?, any? and none?
a = [1, 3, 20]
a.all?
a.all?(1)
a.all? { |i| i }
a.any?
a.any?(1)
a.any? { |i| i }
a.none?
a.none?(1)
a.none? { |i| i }
a.one?
a.one?(1)
a.one? { |i| i }

# detect
p = T.let(->{ 1 }, T.proc.returns(Integer))
T.reveal_type([1,2].detect) # error: Revealed type: `T::Enumerator[Integer]`
T.reveal_type([1,2].detect {|x| false}) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type([1,2].detect(-> {}) {|x| false}) # error: Revealed type: `T.untyped`
T.reveal_type([1,2].detect(-> {})) # error: Revealed type: `T::Enumerator[T.untyped]`
T.reveal_type([1,2].detect(p) {|x| false}) # error: Revealed type: `Integer`
T.reveal_type([1,2].detect(p)) # error: Revealed type: `T::Enumerator[Integer]`
