# typed: strict

module MyModule
  extend T::Sig
  sig { returns(Integer) }
  def my_fun
    return 5
  end
end

class MyClass
  include MyModule
  extend T::Sig

  sig { returns(String) }
  def my_fun
    return "foo"
  end
end
