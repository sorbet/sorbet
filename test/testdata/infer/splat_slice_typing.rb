# typed: true
# This test verifies that splat slice typing is precise for tuple literals.
# Prior to splatSlice, splat variables would get imprecise types from to_ary.
# With splatSlice, we can extract the exact tuple slice for the splat variable.

# Basic splat at end - extracts remaining elements as tuple
a, *b = [1, "hello", :world]
T.assert_type!(a, Integer)
T.reveal_type(b) # error: Revealed type: `[String("hello"), Symbol(:world)] (2-tuple)`

# Splat at start - extracts leading elements as tuple
*c, d = [1, "hello", :world]
T.reveal_type(c) # error: Revealed type: `[Integer(1), String("hello")] (2-tuple)`
T.assert_type!(d, Symbol)

# Splat in middle - extracts middle elements as tuple
e, *f, g = [1, 2, 3, 4, 5]
T.assert_type!(e, Integer)
T.reveal_type(f) # error: Revealed type: `[Integer(2), Integer(3), Integer(4)] (3-tuple)`
T.assert_type!(g, Integer)

# Mixed types with splat in middle
h, *i, j = [1, "two", :three, 4.0]
T.assert_type!(h, Integer)
T.reveal_type(i) # error: Revealed type: `[String("two"), Symbol(:three)] (2-tuple)`
T.assert_type!(j, Float)

# Empty splat when tuple exactly matches non-splat count
k, *l, m = [1, 2]
T.assert_type!(k, Integer)
T.reveal_type(l) # error: Revealed type: `[] (0-tuple)`
T.assert_type!(m, Integer)

# Splat grabs all elements when no surrounding elements
*n = [1, 2, 3]
T.reveal_type(n) # error: Revealed type: `[Integer(1), Integer(2), Integer(3)] (3-tuple)`
