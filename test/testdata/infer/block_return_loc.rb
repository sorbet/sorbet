# typed: true
extend T::Sig

sig {params(blk: T.proc.returns(String)).void}
def example(&blk)
end

example {0}
#        ^  error: Expected `String` but found `Integer(0)` for block result type

example do 0 end
#          ^ error: Expected `String` but found `Integer(0)` for block result type

example do end
#          ^^^ error: Expected `String` but found `NilClass` for block result type

example do |x| end
#              ^^^ error: Expected `String` but found `NilClass` for block result type

example do if Random.rand(2).even?; 0; else 0; end end
#                                                  ^^^ error: Expected `String` but found `Integer(0)` for block result type

sig {params(blk: T.proc.params(x: Integer).returns(String)).void}
def example_blockpass(&blk)
end

example_blockpass(&:even?)
#                  ^^^^^^ error: Expected `String` but found `T::Boolean` for block result type
