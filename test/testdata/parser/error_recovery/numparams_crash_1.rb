# typed: false

[1,2,3].map { |x| _1 }
                # ^^ error: can't use anonymous parameters when ordinary parameters are defined

_1
