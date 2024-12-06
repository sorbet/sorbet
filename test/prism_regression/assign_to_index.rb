# typed: false

# Regular assignment
regular[0] = 1

# Compound assignment operators
bitwise_and[0] &= 2
bitwise_xor[0] ^= 4
shift_right[0] >>= 5
shift_left[0] <<= 6
subtract_assign[0] -= 7
module_assign[0] %= 8
bitwise_or[0] |= 9
divide_assign[0] /= 10
multiply_assign[0] *= 11
exponentiate_assign[0] **= 12

# Special cases
lazy_and_assign[0] &&= 13
lazy_or_assign[0] ||= 14

# Multi-target assignment
target[0], target[1] = 15, 16
target[2, 3], target[4, 5] = 17, 18, 19, 20
target[] = 21 # Yes, this is valid. You can have `def []=(only_one_param)`.

# Assign to index with a block
# Note: this is technically not valid Ruby, but Prism allows it.

bitwise_and[&blk] &= 2
bitwise_xor[&blk] ^= 4
shift_right[&blk] >>= 5
shift_left[&blk] <<= 6
subtract_assign[&blk] -= 7
module_assign[&blk] %= 8
bitwise_or[&blk] |= 9
divide_assign[&blk] /= 10
multiply_assign[&blk] *= 11
exponentiate_assign[&blk] **= 12

lazy_and_assign[&blk] &&= 13
lazy_or_assign[&blk] ||= 14


target[&blk], target[1] = 4
