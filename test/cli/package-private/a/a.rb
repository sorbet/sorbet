# typed: strict

class A
  extend T::Sig

  sig {returns(String)}
  package_private_class_method def self.foo
    '1'
  end

  sig {returns(String)}
  package_private def foo
    '1'
  end

  sig {void}
  def test
    self.class.foo
    foo
  end
end
