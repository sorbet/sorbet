# typed: false
sig do
  params(
    build_group: String,
    type
  # ^^^^ error: positional arg "type" after keyword arg
  # ^^^^ error: Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>
  # ^^^^ error: Unknown parameter name `type`
    filter: T.nilable(String),
  )
  .returns(T::Array[String])
end
def call(build_group, filter:)
end
