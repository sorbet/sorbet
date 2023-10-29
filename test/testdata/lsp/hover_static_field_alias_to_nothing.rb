# typed: strict

SomeConstant = DoesNotExist
#              ^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`

p(SomeConstant)
#  ^ hover: This constant is not defined
