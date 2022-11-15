# typed: true

class Wrapper
  ClassAlias = Integer

  StaticField = T.let('', String)

  T.reveal_type(ClassAlias)  # error: `T.class_of(Integer)`
  T.reveal_type(StaticField) # error: `String`
end
