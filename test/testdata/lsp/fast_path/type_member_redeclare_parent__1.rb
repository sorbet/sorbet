# typed: strict
# disable-fast-path: true
# This test currently exhbits this bug:
# https://github.com/sorbet/sorbet/issues/5941
# which we should fix at some point

class Parent
  extend T::Sig
  extend T::Generic

  MyElem = type_member
end
