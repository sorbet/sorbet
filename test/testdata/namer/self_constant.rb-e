# typed: strict

class A
 self::B = 1
end

T.reveal_type(A::B) # error: Integer(1)

class C
  class self::D
  end
end

T.reveal_type(C::D) # error: T.class_of(C::D)
