# typed: false

# Cannot use 'it' with explicit parameters
proc { |x| it + x } # error: 'it' is not allowed when an ordinary parameter is defined
