# typed: true

class A
  extend T::Sig

  klass = Class.new do
    sig { params(blk: T.proc.void) .void }
    def initialize(&blk)
    end
  end
  T.reveal_type(klass) # error: `T::Class[Object]`

  blk = ->(){}

  inst = klass.new
  T.reveal_type(inst) # error: `Object`

  inst = klass.new(0) # error: Wrong number of arguments for constructor. Expected: `0`, got: `1`
  T.reveal_type(inst) # error: `Object`

  inst = klass.new(&blk) # error: `BasicObject#initialize` does not take a block
  T.reveal_type(inst) # error: `Object`
end
