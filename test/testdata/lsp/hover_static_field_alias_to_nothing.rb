# typed: strict

SomeConstant = DoesNotExist
#              ^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`

p(SomeConstant)
#  ^ hover: jez
