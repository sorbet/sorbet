# typed: true
extend T::Sig

sig { returns(T::Boolean) }
def bool = false

sig { void }
def example
  -> () { return }
  -> () { return 1 }

  lambda { return }
  lambda { return 1 }

  proc { return }
  proc { return 2 }

  2.times { return }
  2.times { return 3 }

  return if bool
  return 4 if bool

  return nil if bool
  return T.unsafe(nil) if bool
  nil
end

sig { void }
def implicit_return
end

sig { void }
def implicit_return_conditional
  if bool
    1
  else
    2
  end
end
