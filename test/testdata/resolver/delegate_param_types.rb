# typed: true
# enable-experimental-delegate-return-types: true
# disable-stress-incremental: true

class DelegateParamTypesTarget
  extend T::Sig

  sig { params(x: Integer, y: String).returns(Float) }
  def pos(x, y)
    0.0
  end

  sig { params(a: Integer, b: String).returns(String) }
  def kw(a:, b:)
    b * a
  end

  sig { params(x: Integer, blk: T.proc.params(s: String).returns(Integer)).returns(Integer) }
  def takes_block(x, &blk)
    blk.call(x.to_s)
  end
end

class DelegateParamTypesDelegator
  extend T::Sig

  delegate :pos, to: :target
  delegate :kw, to: :target
  delegate :takes_block, to: :target
  delegate :size, to: :@str
  delegate :length, to: :@nilable_str, allow_nil: true

  sig { returns(DelegateParamTypesTarget) }
  def target
    T.unsafe(nil)
  end

  sig { void }
  def initialize
    @str = T.let("a string", String)
    @nilable_str = T.let(nil, T.nilable(String))
  end
end

d = DelegateParamTypesDelegator.new

T.assert_type!(d.pos(1, "x"), Float)
d.pos("nope", "x") # error: Expected `Integer` but found `String("nope")` for argument `x`
d.pos(1, 2) # error: Expected `String` but found `Integer(2)` for argument `y`

T.assert_type!(d.kw(a: 2, b: "hi"), String)
d.kw(a: "nope", b: "hi") # error: Expected `Integer` but found `String("nope")` for argument `a`
d.kw(a: 2, b: 10) # error: Expected `String` but found `Integer(10)` for argument `b`

T.assert_type!(d.takes_block(5) {|s| s.size }, Integer)
d.takes_block("nope") {|s| s.size } # error: Expected `Integer` but found `String("nope")` for argument `x`
d.takes_block(5) {|s| s } # error: Expected `Integer` but found `String` for block result type

T.assert_type!(d.size, Integer)
T.reveal_type(d.length) # error: `T.untyped`
