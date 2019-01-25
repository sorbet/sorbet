# typed: true

def _; end

class Box
  extend T::Generic

  A = type_member

  def initialize
    @value = T.let(T.unsafe(nil), A)
  end

   sig {params(value: A).returns(NilClass)}
   def store(value)
     @value = value
     nil
   end

   sig {returns(A)}
   def read
     @value
   end
end

class BoxChild < Box
  extend T::Generic

  A = type_member

  sig {params(v: A).returns(A)}
  def put(v)
    store(v)
    @value
  end
end

class Generics0
   extend T::Sig

   sig {params().returns(NilClass)}
   def create()
     s = Box[String].new
     s.store("foo")
     T.assert_type!(s.read, String)

     child = BoxChild[Integer].new
     child.store(17)
     T.assert_type!(child.read, Integer)
     T.assert_type!(child.put(99), Integer)

     nil
   end
end
