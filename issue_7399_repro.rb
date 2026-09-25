# typed: true
extend T::Sig

module SomeModule
  extend T::Helpers

  module ClassMethods
    def foo
      "foobar"
    end
  end

  mixes_in_class_methods(ClassMethods)
end

sig {params(klass: T.class_of(SomeModule)).void}
def example(klass)
  klass.foo
end
