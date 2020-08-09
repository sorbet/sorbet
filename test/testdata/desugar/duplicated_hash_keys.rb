# typed: true

{
  "foo" => 1,
  "foo" => 2 
# ^^^^^ error: Hash key `foo` is duplicated
}

{
  :bar => 1,
  :bar => 2 
# ^^^^ error: Hash key `bar` is duplicated
}