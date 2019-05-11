# typed: strict

class A
 self::B = 1
end

T.reveal_type(A::B) # error: Integer(1)
