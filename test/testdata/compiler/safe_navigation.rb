# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(x: T.nilable(String)).void}
def foo(x)
  p(x&.to_s)
end

foo("hey")
foo(nil)
