 # typed: true
 class Parent
    extend T::Generic

    A = type_member
    sig {params(a: A).returns(A)}
    def foo(a)
      a
    end
 end

 class Child < Parent
    A = type_member
 end

 module Foo
    extend T::Helpers

    sig {params(a: Integer).returns(Integer)}
    def bar(a)
       s = Child[Integer].new
       s.foo(1)
    end
 end
