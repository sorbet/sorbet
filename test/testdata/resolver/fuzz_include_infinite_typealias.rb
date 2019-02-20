# typed: false
class A
  B = T.type_alias # error: Type alias expands to to an infinite type
  include B
end
