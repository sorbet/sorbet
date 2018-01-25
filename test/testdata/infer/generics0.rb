# @typed

class Box
  A = T.type

  declare_variables(
    :@value => A,
  )

   sig(value: A).returns(NilClass)
   def store(value)
     @value = value
     nil
   end

   sig.returns(A)
   def read
     @value
   end
end

class BoxChild < Box
  A = T.type

  sig(v: A).returns(A)
  def put(v)
    store(v)
    @value
  end
end

class Generics0
   sig().returns(NilClass)
   def create()
     s = Box[String].new
     s.store("foo")
     T.assert_type!(s.read, String)

     # NOTE(nelhage): broken right now
     # child = BoxChild[Integer].new
     # child.store(17)
     # T.assert_type!(child.read, Integer)
     # T.assert_type!(child.put(99), Integer)

     nil
   end
end
