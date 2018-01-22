# @typed

class Generics
 sig(arr: T::Array[String]).returns(String)
 def read(arr)
   T.assert_type!(arr[0], String)
 end

 sig().returns(T::Array[String])
 def create()
   T.assert_type!(T::Array[String].new(), T::Array[String])
 end

 #def hit_me1
 #   Hash[1 => 2]
 #end

 #def hit_me2
 #  Array[1, 2, 3]
 #end
end
