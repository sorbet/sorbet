# typed: false

class A
  B = T.type_alias # error: No block given to `T.type_alias`
  include B
end
