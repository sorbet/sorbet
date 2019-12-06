# typed: true

class Box
  extend T::Generic
  Elem = type_member
end

class MyStruct < T::Struct
  prop :foo, String
end

class MyEnum < T::Enum
  enums do
    Val = new
  end
end

module MMM
  YYY = nil
  TTT = T.type_alias {Integer}
end

module Iface
  extend T::Helpers
  interface!
end

# -- isClassOrModule --

p MyEnu # error: Unable to resolve
#      ^ completion: MyEnum
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

# -- extras --

# TODO(jez) We show duplicate results because of how the T::Enum DSL pass works
p MyEnum::Va # error: Unable to resolve
#           ^ completion: Val, Val
p MyStruc # error: Unable to resolve
#        ^ completion: MyStruct
