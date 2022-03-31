# typed: true

class A < T::Struct

  # If you're seeing this in an error message, it means you printed loc instead
  # of declLoc somewhere.

end

  A.new(0)
#       ^ error: Too many arguments
