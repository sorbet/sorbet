# typed: true

m { |foo = proc { 1 + foo }| }  # error: circular argument reference foo
