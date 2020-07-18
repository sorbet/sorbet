# typed: strict

class A
  extend T::Sig 

  sig {void}
  def main
    RBI::Foo.one
    GlobalBar.foo
  end
end
