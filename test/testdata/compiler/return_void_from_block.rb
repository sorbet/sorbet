# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {void}
def void
  1.times do
    return :whoops
  end
end

p T.unsafe(void())
