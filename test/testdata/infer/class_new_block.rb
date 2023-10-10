# typed: true

class A
  extend T::Sig

  klass = Class.new do
    sig { params(n: Integer, blk: T.proc.void).void }
    def initialize(n, &blk)
    end
  end
  T.reveal_type(klass) # error: `T::Class[T.untyped]`

  blk = ->(){}

  inst = klass.new
  T.reveal_type(inst) # error: `T.untyped`

  inst = klass.new(0)
  T.reveal_type(inst) # error: `T.untyped`

  inst = klass.new(0, &blk)
  T.reveal_type(inst) # error: `T.untyped`
end
