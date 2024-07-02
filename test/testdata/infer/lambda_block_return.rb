# typed: true
extend T::Sig

sig { returns(String) }
def syntactic_lambda
  f = ->() {
    return 0
  }
  f.call.to_s
end

sig { returns(String) }
def kernel_lambda
  f = Kernel.lambda {
    return 0
  }
  f.call.to_s
end

sig { returns(String) }
def kernel_lambda_raises
  f = Kernel.lambda {
    raise
  }
  T.reveal_type(f) # error: `T.proc.returns(T.noreturn)`
  f.call.to_s # error: This code is unreachable
end

sig { returns(String) }
def plain_lambda
  # This is wrong now because we don't treat `self.lambda` as a lambda, and
  # thus choose the `return` keyword to be a method return, not a lambda
  # return.
  f = lambda {
    return 0 # error: Expected `String` but found `Integer(0)` for method result type
  }
  T.reveal_type(f) # error: `T.proc.returns(T.noreturn)`
  f.call.to_s # error: This code is unreachable
end

sig { returns(String) }
def proc_to_lambda
  p = proc {
    return 0 # error: Expected `String` but found `Integer(0)` for method result type
  }
  f = Kernel.lambda(&p)
  f.call.to_s # error: This code is unreachable
end

sig { params(blk: T.proc.returns(NilClass)).void }
def takes_nil_block(&blk)
  yield
end

sig { returns(String) }
def block_inside_lambda
  f = ->() {
    takes_nil_block do
      # At runtime, this returns from the lambda, but Sorbet treats it like it
      # returns from the enclosing block. This would be nice to fix, because
      # there is no workaround for this except manually raising and catching
      # exceptions. (Should be no error.)
      return 0 # error: Expected `NilClass` but found `Integer(0)` for block result type
    end

    nil
  }

  # Should be `T.proc.returns(T.nilable(Integer))`
  T.reveal_type(f) # error: `T.proc.returns(NilClass)`
  f.call.to_s
end
