# typed: true
extend T::Sig

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

{
  :baz => nil,
  'baz' => nil
}

sig {params(x: {y: Integer, y: Integer}).void}
                          # ^ error: Hash key `y` is duplicated
def foo(x)
end


sig {params(_: Integer, _: Integer).returns(String)} 
                      # ^ error: Hash key `_` is duplicated
def self.x(_, _)
  ""
end