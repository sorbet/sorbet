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

  inst = klass.new(0)
  T.reveal_type(inst) # error: `Object`

  inst = klass.new(&blk)
  T.reveal_type(inst) # error: `Object`
end
