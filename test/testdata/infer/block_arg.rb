# typed: strict
class A
  def _; end

  sig(blk: T.proc(a: Integer, b: Float).returns(String)).returns(String)
  def yields(&blk)
    blk.call(13, 1.0)
  end

  sig.returns(String)
  def callit
    v = yields do |x, y|
      T.assert_type!(x, Integer)
      T.assert_type!(y, Float)
      (x + y).to_s
    end
    T.assert_type!(v, String)
  end

  sig.returns(NilClass)
  def badnext
    yields do
      if _
        4 # error: Returning value that does not conform to block result type
      else
        next 5 # error: Returning value that does not conform to block result type
      end
    end

    nil
  end

  sig.returns(NilClass)
  def noblock
  end

  sig.returns(NilClass)
  def extra_block
    noblock do |x|
      x + 1 # Should be typed as untyped
    end
  end

  class NoConstructor
  end

  class ConstructorBlock
    sig(blk: T.proc(s: Symbol).returns(String)).returns(NilClass)
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

  sig.returns(Integer)
  def return_from_block
    yields do # error: Returning value that does not conform to method result type
      if _
        return 7
      else
        return :hi # error: Returning value that does not conform to method result type
      end
    end
  end
end
