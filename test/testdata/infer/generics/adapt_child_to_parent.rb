 # @typed
 class Parent
    A = T.type
    sig(a: A).returns(A)
    def foo(a)
      a
    end
 end

 class Child < Parent
    A = T.type
 end

 module Foo
    sig(a: Integer).returns(Integer)
    def bar(a)
       s = Child[Integer].new
       s.foo(1)
    end
 end
