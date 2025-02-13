# typed: true

def foo(*, ...)
      # ^ parser-error: ... after rest argument
  [1,2,3].each { |x| _1 }
                   # ^^ parser-error: can't use numbered
end
