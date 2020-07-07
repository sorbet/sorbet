# typed: strict

class Foo
  extend T::Sig

  sig {returns(::Integer)}
  def self.one(); end
end

class ::GlobalBar
  extend T::Sig

  sig {returns(Foo)}
  def self.foo(); end
end
