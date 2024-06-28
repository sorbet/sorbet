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
def plain_lambda
  f = lambda {
    return 0 # error: Expected `String` but found `Integer(0)` for method result type
  }
  f.call.to_s
end

sig { returns(String) }
def proc_to_lambda
  p = proc {
    return 0 # error: Expected `String` but found `Integer(0)` for method result type
  }
  f = Kernel.lambda(&p)
  f.call.to_s
end
