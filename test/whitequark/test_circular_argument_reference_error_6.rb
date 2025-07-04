# typed: false

m { |foo = proc { 1 + foo }| }  # parser-error: circular argument reference foo
