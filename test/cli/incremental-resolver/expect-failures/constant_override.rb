#typed:true
B = e
module B
  extend T::Sig
  sig { returns(T.all(B,T)) }
  def foo; T.unsafe(nil); end
end
