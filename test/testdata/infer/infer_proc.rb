# typed: true
extend T::Sig

sig { params(x: Integer).returns(String) }
def returns_string(x); x.to_s; end

sig { params(f: T.proc.returns(Integer)).void }
def takes_void_proc(f); end

f = proc { 0 }
T.reveal_type(f) # error: T.proc.returns(Integer)

takes_void_proc(f)

f = proc { |x| 0 }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(Integer)

f = proc { |x| x }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(T.untyped)

f = proc { |x| returns_string(x) }
T.reveal_type(f) # error: T.proc.params(arg0: T.untyped).returns(String)

def example
  f = proc do |x|
    if x
      return false # returns from `example`, not from the proc
    end

    true
  end
  T.reveal_type(f) # error: `T.proc.params(arg0: T.untyped).returns(TrueClass)`
end

f = proc { |x|
  if x
    next false
  end

  true
}
T.reveal_type(f) # error: `T.proc.params(arg0: T.untyped).returns(T::Boolean)`

f = proc {
  if T.unsafe(nil)
    next T.unsafe(nil)
  else
    next 0
  end
}
T.reveal_type(f) # error: `T.proc.returns(T.untyped)`

f = proc {
  raise
}
T.reveal_type(f) # error: `T.proc.returns(T.noreturn)`
