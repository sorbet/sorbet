# typed: true
extend T::Sig

sig {params(blk: T.proc.returns(String)).void}
def example(&blk)
end

example {0}
#        ^ error: Expected `String` but found `Integer(0)` for block result type

example do 0 end
#          ^ error: Expected `String` but found `Integer(0)` for block result type

example do end
#          ^^^ error: Expected `String` but found `NilClass` for block result type

example do |x| end
#              ^^^ error: Expected `String` but found `NilClass` for block result type

example do if Random.rand(2).even?; 0; else 0; end end
#                                                  ^^^ error: Expected `String` but found `Integer(0)` for block result type

module M
  X = 0
end

example {M::X}
#        ^^^^ error: Expected `String` but found `Integer` for block result type

example {(M::X)}
#         ^^^^ error: Expected `String` but found `Integer` for block result type

example do
  x = 0
  M::X
# ^^^^ error: Expected `String` but found `Integer` for block result type
end

example {begin; 0; M::X; end}
#                  ^^^^ error: Expected `String` but found `Integer` for block result type

example {M::X.itself}
#        ^^^^^^^^^^^ error: Expected `String` but found `Integer` for block result type

example {::M::X}
#        ^^^^^^ error: Expected `String` but found `Integer` for block result type

module M
  module N
    Y = 0
  end
end

example {M::N::Y}
#        ^^^^^^^ error: Expected `String` but found `Integer` for block result type

sig {params(blk: T.proc.params(x: Integer).returns(String)).void}
def example_blockpass(&blk)
end

example_blockpass(&:even?)
#                  ^^^^^^ error: Expected `String` but found `T::Boolean` for block result type
