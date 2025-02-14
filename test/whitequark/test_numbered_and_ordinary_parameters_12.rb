# typed: true

proc {|;a| _1} # parser-error: can't use numbered params when ordinary params were also defined
