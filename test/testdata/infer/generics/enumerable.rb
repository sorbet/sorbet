 # typed: true
 module MyEnumerable
    extend T::Generic

    A = type_member
    sig(a: MyEnumerable[A]).returns(MyEnumerable[A])
    def -(a)
      self
    end
 end

 class MySet
    include MyEnumerable
    A = type_member
 end

 module Foo
    def bar
       MySet.new() - MySet.new()
    end
 end
