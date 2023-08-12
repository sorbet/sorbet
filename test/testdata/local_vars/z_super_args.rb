# typed: true

class Parent
  def takes_positional(arg0)
    p arg0
  end

  def takes_keyword(arg0:)
    p arg0
  end

  def takes_two_keyword(arg0:, arg1:)
    p arg0
    p arg1
  end

  def takes_positional_then_keyword(arg0, arg1:)
    p arg0
    p arg1
  end

  def takes_rest(*args)
    p args
  end

  def takes_keyword_rest(**args)
    p args
  end

  def takes_block(&blk)
    p blk
  end

  def zsuper_inside_block
  end
end

class Child < Parent
  def takes_positional(arg0)
    super
  end

  def takes_keyword(arg0:)
    super
  end

  def takes_two_keyword(arg0:, arg1:)
    super
  end

  def takes_positional_then_keyword(arg0, arg1:)
    super
  end

  def takes_rest(*args)
    super
  end

  def takes_keyword_rest(**args)
    super
  end

  def takes_block(&blk)
    super
  end

  def zsuper_inside_block
    1.times do |i|
      super
    end
  end
end

module M
  def super_inside_module
    super
  end
end
