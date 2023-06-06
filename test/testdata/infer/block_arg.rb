# typed: true
class A
  extend T::Sig

  def _; end

  sig {params(blk: T.proc.params(a: Integer, b: Float).returns(String)).returns(String)}
  def yields(&blk)
    blk.call(13, 1.0)
  end

  sig {returns(String)}
  def callit
    v = yields do |x, y|
      T.assert_type!(x, Integer)
      T.assert_type!(y, Float)
      (x + y).to_s
    end
    T.assert_type!(v, String)
  end

  sig {returns(NilClass)}
  def badnext
    yields do
      if _
        4 # error: Expected `String` but found `Integer(4)` for block result type
      else
        next 5 # error: Expected `String` but found `Integer(5)` for block result type
      end
    end

    nil
  end

  sig {returns(NilClass)}
  def noblock
  end

  class NoConstructor
  end

  class ConstructorBlock
    extend T::Sig

    sig {params(blk: T.proc.params(s: Symbol).returns(String)).void}
    def initialize(&blk)
    end
  end

  # Constructors are dispatched via a different code path; ensure that
  # it knows how to enter blocks.
  def test_constructors
    NoConstructor.new do |x|
      x + 1
    end

    ConstructorBlock.new do |sym|
      T.assert_type!(sym, Symbol)
      sym.to_s
    end

    _.new do |x, y|
    end
  end

  def test_dynamic
    _.each do |x|
      puts(x)
    end
  end

  sig {returns(Integer)}
  def return_from_block
    yields do # error: Expected `Integer` but found `String` for method result type
      if _
        return 7
      else
        return :hi # error: Expected `Integer` but found `Symbol(:hi)` for method result type
      end
    end
  end
end
