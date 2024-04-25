# frozen_string_literal: true
# typed: true
# compiled: false

extend T::Sig

def yield_from_2
  yield
end

sig { returns(T.proc.returns(Integer)) }
def some_lambda
  -> () { return T.unsafe("this is not an integer but we shouldn't raise a TypeError anyway") }
end
