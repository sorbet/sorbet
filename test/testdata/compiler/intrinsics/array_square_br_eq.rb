# typed: true

x = T.let([1,2,3,4], T::Array[Integer])

# TODO: we are trying to use a static internal method from array.c
# (rb_ary_splice), and get a dynamic linker error at runtime.
# x[1..2] = 0
# puts x              # [1,0,4]
# x[1..2] = [5,5,5]
# puts x              # [1,5,5,5]
# x[0,2] = 0
# puts x              # [0,5,5]
# x[0,3] = [1,2,3,4]
# puts x              # [1,2,3,4]

x[1] = 0
puts x

x[2] = 1
puts x

x[10] = 1
puts x
