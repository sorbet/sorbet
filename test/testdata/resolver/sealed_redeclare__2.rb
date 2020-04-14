# typed: strict

class B
  extend T::Helpers
  # This is an error in the runtime, but we haven't put the time to get an
  # error here statically because it's tricky to get to work in incremental
  # resolve.
  sealed!
end
