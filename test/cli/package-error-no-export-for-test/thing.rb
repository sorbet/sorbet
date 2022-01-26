# typed: strict

class Foo::Thing
  extend T::Sig

  sig {returns(String)}
  def foo_thing; 'foo_thing'; end
end
