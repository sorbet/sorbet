# typed: false

# Regular assignment
REGULAR = 1

# Compound assignment operators
BITWISE_AND     &= 2
BITWISE_OR      |= 3
BITWISE_XOR     ^= 4
SHIFT_RIGHT    >>= 5
SHIFT_LEFT     <<= 6
ADD_ASSIGN      += 7
SUBTRACT_ASSIGN -= 8
DIVIDE_ASSIGN   /= 9
MODULO_ASSIGN   %= 10
MULTIPLY_ASSIGN *= 11
EXPONENTIATE_ASSIGN **= 12

# Special cases
LAZY_AND_ASSIGN &&= 13
LAZY_OR_ASSGIN  ||= 14

# Multi-target assignment
TARGET1, TARGET2 = 15, 16





### Constant paths

# Regular assignment
ConstantPath::REGULAR = 101

# # Compound assignment operators
ConstantPath::BITWISE_AND     &= 102
ConstantPath::BITWISE_OR      |= 103
ConstantPath::BITWISE_XOR     ^= 104
ConstantPath::SHIFT_RIGHT    >>= 105
ConstantPath::SHIFT_LEFT     <<= 106
ConstantPath::ADD_ASSIGN      += 107
ConstantPath::SUBTRACT_ASSIGN -= 108
ConstantPath::MODULO_ASSIGN   %= 109
ConstantPath::DIVIDE_ASSIGN   /= 110
ConstantPath::MULTIPLY_ASSIGN *= 111
ConstantPath::EXPONENTIATE_ASSIGN **= 112

# Special cases
ConstantPath::LAZY_AND_ASSIGN &&= 113
ConstantPath::LAZY_OR_ASSGIN  ||= 114

# Multi-target assignment
ConstantPath::TARGET1, ConstantPath::TARGET2 = 115, 116

### Fully qualified constant paths

# Regular assignment

::FullyQualified::ConstantPath::REGULAR = 201

# # Compound assignment operators
::FullyQualified::ConstantPath::BITWISE_AND      &= 202
::FullyQualified::ConstantPath::BITWISE_OR       |= 203
::FullyQualified::ConstantPath::BITWISE_XOR      ^= 204
::FullyQualified::ConstantPath::SHIFT_RIGHT     >>= 205
::FullyQualified::ConstantPath::SHIFT_LEFT      <<= 206
::FullyQualified::ConstantPath::ADD_ASSIGN       += 207
::FullyQualified::ConstantPath::SUBTRACT_ASSIGN  -= 208
::FullyQualified::ConstantPath::MODULO_ASSIGN    %= 209
::FullyQualified::ConstantPath::DIVIDE_ASSIGN    /= 210
::FullyQualified::ConstantPath::MULTIPLY_ASSIGN  *= 211
::FullyQualified::ConstantPath::EXPONENTIATE_ASSIGN **= 312

# Special cases
::FullyQualified::ConstantPath::LAZY_AND_ASSIGN &&= 313
::FullyQualified::ConstantPath::LAZY_OR_ASSGIN  ||= 314

# Multi-target assignment
::FullyQualified::ConstantPath::TARGET1, ::FullyQualified::ConstantPath::TARGET2 = 315, 316





### Dynamic constant assignments

def method1
  # * Sorbet's previous parser would treat all of these as dynamic constant assignments.
  # * `ruby -c --parser=parse.y` will report dynamic constant assignments for all of these.
  # * `ruby -c --parser=prism`   will only report the first one.
  # * At runtime, Ruby will issue warnings for all of these.

  DynamicConstant = 1

  DynamicConstantBitwiseAnd     &= 2
  DynamicConstantBitwiseOr      |= 3
  DynamicConstantBitwiseXor     ^= 4
  DynamicConstantShiftRight     >>= 5
  DynamicConstantShiftLeft      <<= 6
  DynamicConstantAddAssign      += 7
  DynamicConstantSubtractAssign -= 8
  DynamicConstantModuloAssign   %= 9
  DynamicConstantDivideAssign   /= 10
  DynamicConstantMultiplyAssign *= 11
  DynamicConstantExponentiateAssign **= 12

  # Special cases
  DynamicConstantLazyAndAssign &&= 13
  DynamicConstantLazyOrAssgin  ||= 14

  DynamicConstantTarget1, DynamicConstantTarget2 = 15, 16
end

def method2
  # * `ruby -c --parser=parse.y` reports dynamic constant assignments for all of them.
  # * `ruby -c --parser=prism`   only reports the first one.
  # * At runtime, Ruby will issue warnings for all of these.

  ConstantPath::DynamicConstant2 = 1

  # Sorbet's previous parser would treat all of these as dynamic constant assignments.
  ConstantPath::DynamicConstantBitwiseAnd     &= 2
  ConstantPath::DynamicConstantBitwiseOr      |= 3
  ConstantPath::DynamicConstantBitwiseXor     ^= 4
  ConstantPath::DynamicConstantShiftRight     >>= 5
  ConstantPath::DynamicConstantShiftLeft      <<= 6
  ConstantPath::DynamicConstantAddAssign      += 7
  ConstantPath::DynamicConstantSubtractAssign -= 8
  ConstantPath::DynamicConstantModuloAssign   %= 9
  ConstantPath::DynamicConstantDivideAssign   /= 10
  ConstantPath::DynamicConstantMultiplyAssign *= 11
  ConstantPath::DynamicConstantExponentiateAssign **= 12

  # And / Or assignment; still should not raise a syntax error
  ConstantPath::DynamicConstantLazyAndAssign &&= 13
  ConstantPath::DynamicConstantLazyOrAssgin  ||= 14

  ConstantPath::DynamicConstantTarget1, ConstantPath::DynamicConstantTarget2 = 15, 16
end


def method3
  # * `ruby -c --parser=parse.y` reports dynamic constant assignments for all of them.
  # * `ruby -c --parser=prism`   only reports the first one.
  # * At runtime, Ruby will issue warnings for all of these.

  ::FullyQualified::ConstantPath::DynamicConstant2 = 1

  # Sorbet's previous parser would treat all of these as dynamic constant assignments.
  ::FullyQualified::ConstantPath::DynamicConstantBitwiseAnd     &= 2
  ::FullyQualified::ConstantPath::DynamicConstantBitwiseOr      |= 3
  ::FullyQualified::ConstantPath::DynamicConstantBitwiseXor     ^= 4
  ::FullyQualified::ConstantPath::DynamicConstantShiftRight     >>= 5
  ::FullyQualified::ConstantPath::DynamicConstantShiftLeft      <<= 6
  ::FullyQualified::ConstantPath::DynamicConstantAddAssign      += 7
  ::FullyQualified::ConstantPath::DynamicConstantSubtractAssign -= 8
  ::FullyQualified::ConstantPath::DynamicConstantModuloAssign   %= 9
  ::FullyQualified::ConstantPath::DynamicConstantDivideAssign   /= 10
  ::FullyQualified::ConstantPath::DynamicConstantMultiplyAssign *= 11
  ::FullyQualified::ConstantPath::DynamicConstantExponentiateAssign **= 12

  # And / Or assignment; still should not raise a syntax error
  ::FullyQualified::ConstantPath::DynamicConstantLazyAndAssign &&= 13
  ::FullyQualified::ConstantPath::DynamicConstantLazyOrAssgin  ||= 14

  ::FullyQualified::ConstantPath::DynamicConstantTarget1, ::FullyQualified::ConstantPath::DynamicConstantTarget2 = 15, 16
end
