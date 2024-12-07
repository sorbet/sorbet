# typed: false

# Regular assignment
regular[0] = 1

# # Compound assignment operators
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

# # Special cases
lazy_and_assign[0] &&= 13
lazy_or_assign[0] ||= 14

# Multi-target assignment
target[0], target[1] = 15, 16
target[2, 3], target[4, 5] = 17, 18, 19, 20
target[] = 21 # Yes, this is valid. You can have `def []=(only_one_param)`.
