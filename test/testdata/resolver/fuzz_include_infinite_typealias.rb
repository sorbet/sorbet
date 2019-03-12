# typed: false
class A
  B = T.type_alias # error: Type alias `A::B` expands to to an infinite type
  include B
end
