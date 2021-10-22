# typed: strict
# frozen_string_literal: true
# compiled: true

# begin
#   foo = T.let(T.unsafe(:foo), Integer)
# rescue TypeError => e
#   # Ensure that the message about symbols matches the runtime
#   # The runtime adds a `Caller:` line; this test just cares about whether we
#   # print the value in the message identically to the runtime.
#   puts e.message.split("\n")[0]
# end
