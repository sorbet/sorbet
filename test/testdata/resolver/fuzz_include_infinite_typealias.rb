# typed: false

class A
  B = T.type_alias # error: No argument given to `T.type_alias`
  include B
end
