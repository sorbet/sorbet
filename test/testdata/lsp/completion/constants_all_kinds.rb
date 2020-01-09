# typed: true

# docs for AAA
class AAA; end

# docs for Box
class Box
  extend T::Generic
  # docs for Box::Elem
  Elem = type_member
end

# docs for MyStruct
class MyStruct < T::Struct
  prop :foo, String
end

# docs for MyEnum
class MyEnum < T::Enum
  enums do
    # docs for MyEnum::Val
    Val = new
  end
end

# docs for MMM
module MMM
  # docs for YYY
  YYY = nil
  # docs for ZZZ
  ZZZ = AAA
  # docs for TTT
  TTT = T.type_alias {Integer}
end

# docs for Iface
module Iface
  extend T::Helpers
  interface!
end

# -- isClassOrModule --

p MyEnu # error: Unable to resolve
#      ^ completion: MyEnum
p AA # error: Unable to resolve
#   ^ completion: AAA
p Bo # error: Unable to resolve
#   ^ completion: Box
p Ifac # error: Unable to resolve
#     ^ completion: Iface
p MM # error: Unable to resolve
#   ^ completion: MMM

# -- isTypeMember --

p Box::Ele # error: Unable to resolve
#         ^ completion: Elem

# -- isStaticField --

p MMM::TT # error: Unable to resolve
#        ^ completion: TTT
p MMM::YY # error: Unable to resolve
#        ^ completion: YYY
p MMM::ZZ # error: Unable to resolve
#        ^ completion: ZZZ

# -- extras --

p MyEnum::Va # error: Unable to resolve
#           ^ completion: Val
p MyStruc # error: Unable to resolve
#        ^ completion: MyStruct
