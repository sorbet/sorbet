# @typed

#class String
#end


class Box
  A = T.type

   # declare_variables(
   #   :@value => T,
   # )

   sig(value: A).returns(NilClass)
   def store(value)
     adapt(value)
   end

   sig().returns(A)
   def read()
     adapt(1)
   end

   sig(f: T.untyped).returns(T.untyped)
   def adapt(f)
     f
   end
end

class Generics0
   sig().returns(String)
   def create()
     s = Box[String].new()
     s.store("foo")
     T.assert_type!(s.read, String)
   end
end
