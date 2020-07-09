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
