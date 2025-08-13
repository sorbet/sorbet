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

  class ::Global
    extend T::Sig
    sig { void }
    package_private def foo; end

    sig { void }
    package_private_class_method def self.foo; end

    sig { void }
    def bar; end

    sig { void }
    def self.bar; end

    package_private :bar
    package_private_class_method :bar
  end

  sig { void }
  def another_test
    Global.new.foo
    Global.foo
  end
end
