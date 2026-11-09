# typed: true
class Module
  include T::Sig
end

module SomeModule
  extend T::Helpers

  module ClassMethods
    sig {returns(String)}
    def foo
      "foobar"
    end
  end

  mixes_in_class_methods(ClassMethods)
end

class SomeClass
  include SomeModule
end

sig {params(klass: T.class_of(SomeModule)).void}
def incorrect_type(klass)
  klass.foo # error: Method `foo` does not exist on `T.class_of(SomeModule)`
end

sig {params(klass: T.all(T::Class[SomeModule], SomeModule::ClassMethods)).void}
def correct_type(klass)
  klass.foo
end

correct_type(SomeClass)
