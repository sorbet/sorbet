# typed: true
class Module; include T::Sig; end

module MyInterface
  extend T::Helpers

  sig {returns(Integer)}
  def some_instance_method; 0; end

  module ClassMethods
    sig {returns(String)}
    def some_class_method; ''; end
  end
  mixes_in_class_methods(ClassMethods)
end

class MyClass
  include MyInterface
end

sig {params(x: T.class_of(MyClass)).void}
def example1(x)
  x.new.some_instance_method  # ok
  x.some_class_method         # ok
end

example1(MyClass)             # ok

sig {params(x: T.class_of(MyInterface)).void}
def example2(x)
  x.new.some_instance_method  # error: `new` does not exist
  x.some_class_method         # error: `some_class_method` does not exist
end

example2(MyClass)             # error: Expected `T.class_of(MyInterface)` but found `T.class_of(MyClass)`

sig {params(x: T.all(Class, MyInterface::ClassMethods)).void}
def example3(x)
  x.new.some_instance_method  # error: `some_instance_method` does not exist
  x.some_class_method         # ok
end

example3(MyClass)             # ok

sig {params(x: T.all(T::Class[MyInterface], MyInterface::ClassMethods)).void}
def example4(x)
  x.new.some_instance_method  # ok
  x.some_class_method         # ok
end

example4(MyClass)             # ok

sig {params(x: T::Class[MyClass]).void}
def example5(x)
  x.new.some_instance_method  # ok
  x.some_class_method         # error: Method `some_class_method` does not exist on `T::Class[MyClass]`
end

example5(MyClass)             # ok

sig {params(x: T.all(T::Class[MyClass], T.class_of(MyClass))).void}
def example6(x)
  T.reveal_type(x)            # error: `T.class_of(MyClass)`
  x.new.some_instance_method  # ok
  x.some_class_method         # ok
end

example6(MyClass)             # ok
