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

# Multi-target assignment
TARGET1, TARGET2 = 29, 30

### Constant paths

# Regular assignment
ConstantPath::REGULAR = 15
::FullyQualified::ConstantPath::REGULAR = 16

# # Compound assignment operators
ConstantPath::BITWISE_AND &= 17
ConstantPath::BITWISE_XOR ^= 18
ConstantPath::SHIFT_RIGHT >>= 19
ConstantPath::SHIFT_LEFT <<= 20
ConstantPath::SUBTRACT_ASSIGN -= 21
ConstantPath::MODULE_ASSIGN %= 22
ConstantPath::BITWISE_OR |= 23
ConstantPath::DIVIDE_ASSIGN /= 24
ConstantPath::MULTIPLY_ASSIGN *= 25
ConstantPath::EXPONENTIATE_ASSIGN **= 26

# Special cases
ConstantPath::LAZY_AND_ASSIGN &&= 27
ConstantPath::LAZY_OR_ASSGIN ||= 28

# Multi-target assignment
ConstantPath::TARGET1, ConstantPath::TARGET2 = 31, 32
::ConstantPath::TARGET1, ::ConstantPath::TARGET2 = 33, 34
