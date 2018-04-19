 # typed: strict
 module MyEnumerable
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
       MyEnumerable.new() - MyEnumerable.new()
    end
 end
