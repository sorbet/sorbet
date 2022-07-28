 # typed: true
 module MyEnumerable
    extend T::Generic
    extend T::Sig

    A = type_member
    sig {params(a: MyEnumerable[A]).returns(MyEnumerable[A])}
    def -(a)
      self
    end
 end

 class MySet
    extend T::Generic
    include MyEnumerable
    A = type_member
 end

 module Foo
    def bar
       MySet.new() - MySet.new()
       #     ^^^ error: `MySet` is a generic class and requires being instantiated with explicit type arguments
       #                   ^^^ error: `MySet` is a generic class and requires being instantiated with explicit type arguments
    end
 end
