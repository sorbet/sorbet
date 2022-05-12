# typed: true

class OtherClass
  extend T::Sig

  sig {params(c: MyClass).void}
  #              ^^^^^^^ usage: MyClass
  def do_something(c); end

  sig {params(m: MyModule).void}
  #              ^^^^^^^^ usage: MyModule
  def do_something_else(m); end
end
