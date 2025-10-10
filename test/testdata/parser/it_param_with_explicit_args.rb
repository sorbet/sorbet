# typed: false

# Cannot use 'it' with explicit parameters
proc { |x| it + x } # error: can't use anonymous parameters when ordinary parameters are defined
