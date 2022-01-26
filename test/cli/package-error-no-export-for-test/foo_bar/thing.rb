# typed: strict

class Foo::Bar::Thing
  extend T::Sig

  sig {returns(String)}
  def foo_bar_thing; 'foo_bar_thing'; end
end
