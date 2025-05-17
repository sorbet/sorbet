# typed: false

[1,2,3].map { |x| _1 }
                # ^^ parser-error: can't use numbered params

_1
