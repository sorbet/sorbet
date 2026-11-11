# typed: strict
module Root::A
  class Foo
    extend T::Sig

    sig { void }
    def foo; end
    package_private :foo

    sig { void }
    def self.bar; end
    package_private_class_method :bar
  end
end
