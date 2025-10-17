# typed: true

def foo(*, ...)
      # ^ error: ... after rest argument
  [1,2,3].each { |x| _1 }
                   # ^^ error: numbered parameters are not allowed when an ordinary parameter is defined
end
