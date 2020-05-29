# typed: true
class EnumProp
  include T::Props
  prop :enum_field, T.enum(["hi", "there"])
end

EnumProp.new.enum_field
