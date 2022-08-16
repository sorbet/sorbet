# typed: true

class FooGood
  include IFoo

  def foo(x); end
end

class FooBad # error: Missing definition for abstract method `IFoo#foo`
  include IFoo
end
