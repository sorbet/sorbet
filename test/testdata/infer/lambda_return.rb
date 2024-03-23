# typed: true
extend T::Sig

sig { params(x: T.anything).returns(String) }
def returns_string(x)
  ''
end

f = -> () { 0 }
T.reveal_type(f) # error: T.proc.returns(Integer)

f = -> (x) { 0 }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(Integer)

f = -> (x) { x }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(T.untyped)

f = -> (x) { returns_string(x) }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(String)
