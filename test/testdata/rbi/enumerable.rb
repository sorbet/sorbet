# typed: true

# You can return any Comparable in the block for max_by (and String is one)
[1, 3, 20].min_by {|n| n.to_s}
[1, 3, 20].max_by {|n| n.to_s}
[1, 3, 20].minmax_by {|n| n.to_s}
