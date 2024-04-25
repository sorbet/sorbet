# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {returns(String)}
def str_uplus
  s = +''
  s
end

# This is sort of a hack to see if we eliminate the exit type test, which we should.


p str_uplus()
