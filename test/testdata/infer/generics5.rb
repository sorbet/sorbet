 # @typed
 module MyEnumerable
    A = T.type
    sig(a: MyEnumerable[A]).returns(MyEnumerable[A])
    def -(a)
      self
    end
 end

 class MySet
    include MyEnumerable
    A = T.type
 end

 module Foo
    def bar
       MyEnumerable.new() - MyEnumerable.new()
    end
 end
