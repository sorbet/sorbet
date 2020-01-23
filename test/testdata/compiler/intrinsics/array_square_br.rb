# frozen_string_literal: true
# typed: true
# compiled: true

x = T.let([1,2,3,4], T::Array[Integer])

# rb_ary_aref2
puts(x[1,2])

# rb_ary_entry
puts(x[1])

# rb_range_beg_len out of range
puts(x[5..10])

# rb_range_beg_len in range
puts(x[0..10])
puts(x[0..1])

# rb_ary_entry
puts(x[1.0])
