# typed: strict

class Parent
  extend T::Sig

  sig { params(arg0: Integer).void }
  def takes_positional(arg0)
    p arg0
  end

  sig { params(arg0: Integer).void }
  def takes_keyword(arg0:)
    p arg0
  end

  sig { params(arg0: Integer, arg1: Integer).void }
  def takes_two_keyword(arg0:, arg1:)
    p arg0
    p arg1
  end

  sig { params(arg0: Integer, arg1: Integer).void }
  def takes_positional_then_keyword(arg0, arg1:)
    p arg0
    p arg1
  end

  sig { params(args: T.untyped).void }
  def takes_rest(*args)
    p args
  end

  sig { params(args: T.untyped).void }
  def takes_keyword_rest(**args)
    p args
  end

  sig { params(blk: T.proc.void).void }
  def takes_block(&blk)
    p blk
  end

  sig { void }
  def zsuper_inside_block
  end
end

class Child < Parent
  sig { params(arg0: Integer).void }
  def takes_positional(arg0)
    super
  end

  sig { params(arg0: Integer).void }
  def takes_keyword(arg0:)
    super
  end

  sig { params(arg0: Integer, arg1: Integer).void }
  def takes_two_keyword(arg0:, arg1:)
    super
  end

  sig { params(arg0: Integer, arg1: Integer).void }
  def takes_positional_then_keyword(arg0, arg1:)
    super
  end

  sig { params(args: T.untyped).void }
  def takes_rest(*args)
    super
  end

  sig { params(args: T.untyped).void }
  def takes_keyword_rest(**args)
    super
  end

  sig { params(blk: T.proc.void).void }
  def takes_block(&blk)
    super
  end

  sig { void }
  def zsuper_inside_block
    1.times do |i|
      super
    end
  end
end

module M
  extend T::Sig

  sig { void }
  def super_inside_module
    super
  end
end
