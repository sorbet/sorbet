# typed: false

f (1) { |x| _1 }
          # ^^ error: can't use anonymous parameters when ordinary parameters are defined

_1
