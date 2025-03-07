# typed: true

m { |x = _1| } # parser-error: can't use numbered params when ordinary params were also defined
