# frozen_string_literal: true
# typed: strict
# compiled: false

extend T::Sig

module Base
  extend T::Helpers
  interface!
end

class A
  include Base
end

sig {returns(Base)}
def make_wrapped_a
  Base.wrap_instance(A.new)
end
