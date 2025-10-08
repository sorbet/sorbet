# typed: true

def foo(*, ...)
      # ^ error: ... after rest argument
  [1,2,3].each { |x| _1 }
                   # ^^ error: can't use anonymous parameters when ordinary parameters are defined
end
