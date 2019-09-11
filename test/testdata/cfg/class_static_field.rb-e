# typed: true

class A
  extend T::Sig

  @@x = T.let(nil, T.nilable(String))

  sig {returns(String)}
  def self.foo
    @@x # error: Returning value that does not conform to method result type
  end
end
