# typed: true
extend T::Sig

sig { params(x: Integer).returns(String) }
def returns_string(x); x.to_s; end

sig { params(f: T.proc.returns(Integer)).void }
def takes_void_proc(f); end

f = -> () { 0 }
T.reveal_type(f) # error: T.proc.returns(Integer)

takes_void_proc(f)

f = -> (x) { 0 }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(Integer)

f = -> (x) { x }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(T.untyped)

f = -> (x) { returns_string(x) }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(String)

f = -> (x) {
  if x
    return false
  end

  true
}
T.reveal_type(f) # error: `T.proc.params(arg0: T.untyped).returns(T::Boolean)`

f = -> (x) {
  if x
    next false
  end

  true
}
T.reveal_type(f) # error: `T.proc.params(arg0: T.untyped).returns(T::Boolean)`
