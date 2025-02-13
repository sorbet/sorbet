# typed: false

f (1) { |x| _1 }
          # ^^ parser-error: can't use numbered params

_1
