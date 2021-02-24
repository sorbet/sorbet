# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {returns(T.nilable([Integer, Integer]))}
def foo
  nil
end

first, _ = foo
p first
