# typed: false

# Regular assignment
REGULAR = 1

# Compound assignment operators
BITWISE_AND &= 2
BITWISE_XOR ^= 4
SHIFT_RIGHT >>= 5
SHIFT_LEFT <<= 6
SUBTRACT_ASSIGN -= 7
MODULE_ASSIGN %= 8
BITWISE_OR |= 9
DIVIDE_ASSIGN /= 10
MULTIPLY_ASSIGN *= 11
EXPONENTIATE_ASSIGN **= 12

# Special cases
LAZY_AND_ASSIGN &&= 13
LAZY_OR_ASSGIN ||= 14

### Constant paths

# Regular assignment
ConstantPath::REGULAR = 15
::FullyQualified::ConstantPath::REGULAR = 16
