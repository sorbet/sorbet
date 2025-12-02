# typed: false

# Regular assignment
regular[0] = 1

# Compound assignment operators
bitwise_and[0]     &= 2
bitwise_or[0]      |= 3
bitwise_xor[0]     ^= 4
shift_right[0]    >>= 5
shift_left[0]     <<= 6
add_assign[0]      += 7
subtract_assign[0] -= 8
modulo_assign[0]   %= 9
divide_assign[0]   /= 10
multiply_assign[0] *= 11
exponentiate_assign[0] **= 12

# Special cases
lazy_and_assign[0] &&= 13
lazy_or_assign[0]  ||= 14





### Multiple indices

# Regular assignment
regular[0] = 101

# Compound assignment operators
bitwise_and[0, 1]     &= 102
bitwise_or[0, 1]      |= 103
bitwise_xor[0, 1]     ^= 104
shift_right[0, 1]    >>= 105
shift_left[0, 1]     <<= 106
add_assign[0, 1]      += 107
subtract_assign[0, 1] -= 108
modulo_assign[0, 1]   %= 109
divide_assign[0, 1]   /= 110
multiply_assign[0, 1] *= 111
exponentiate_assign[0, 1] **= 112

# Special cases
lazy_and_assign[0, 1] &&= 113
lazy_or_assign[0, 1]  ||= 114

# Multi-target assignment
target[2, 3], target[4, 5] = 115, 116
target[] = 17 # Yes, this is valid. You can have `def []=(only_one_param)`.





# ### Using a block as an index
# # This is not valid Ruby, but Prism allows it.
# TODO: Uncomment once this is implemented
# 
# bitwise_and[&blk]     &= 202
# bitwise_or[&blk]      |= 203
# bitwise_xor[&blk]     ^= 204
# shift_right[&blk]    >>= 205
# shift_left[&blk]     <<= 206
# add_assign[&blk]      += 207
# subtract_assign[&blk] -= 208
# modulo_assign[&blk]   %= 209
# divide_assign[&blk]   /= 200
# multiply_assign[&blk] *= 201
# exponentiate_assign[&blk] **= 202
# 
# # Special cases
# lazy_and_assign[&blk] &&= 203
# lazy_or_assign[&blk]  ||= 204
# 
# # Multi-target assignment
# target[&blk], target[1] = 205
