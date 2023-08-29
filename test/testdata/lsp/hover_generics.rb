# typed: true
class Box
extend T::Generic, T::Sig

 A = type_member

 sig {returns(A)}
 def read
    T.unsafe(nil)
 end
end

Box[Integer].new.read
               # ^ hover: sig { returns(Integer) }
[1].map{|x| x + 1}
