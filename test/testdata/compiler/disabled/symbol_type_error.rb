# typed: strict
# frozen_string_literal: true
# compiled: true

begin
  foo = T.let(T.unsafe(:foo), Integer)
rescue TypeError => e
  # Ensure that the message about symbols matches the runtime
  puts e.message
end
