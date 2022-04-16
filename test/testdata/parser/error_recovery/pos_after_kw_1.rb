# typed: false
sig do
  params(
    build_group: String,
    type
    filter: T.nilable(String),
  )
  .returns(T::Array[String])
end
def call(build_group, filter:)
end
