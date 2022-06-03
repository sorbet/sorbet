# typed: true

class AliasContainer
  ContainedThing = T.type_alias {T.any(Float, Symbol)}
end
