# typed: false
# disable-fast-path: true
class A
  B = T.type_alias # error: Unable to resolve right hand side of type alias `A::B`
  include B
end
