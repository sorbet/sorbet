# typed: true

class SimpleReturns
  extend T::Sig

  sig {params(x: Integer).returns(Integer).on_failure(:soft, notify: 'sorbet')}
  #                       ^^^^^^^ error: The initialize method should always return void
  def initialize(x)
    @x = x
  end
end

class SimpleMultiLineReturns
  extend T::Sig

  sig do
    params(
      x: Integer
    )
    .returns(Integer)
    #^^^^^^^ error: The initialize method should always return void
  end
  def initialize(x)
    @x = x
  end
end

class MultiLineReturnsWithCombinators
  extend T::Sig

  sig do
    params(
      x: T::Array[T.any(String, T::Enum)]
    )
    .returns(T::Array[T.any(String, T::Enum)])
    #^^^^^^^ error: The initialize method should always return void
  end
  def initialize(x)
    @x = x
  end
end

class SingleLineReturnsWithCombinators
  extend T::Sig

  sig {params(x: T.nilable(Integer)).returns(T.nilable(Integer)).on_failure(:soft, notify: 'sorbet')}
  #                                  ^^^^^^^ error: The initialize method should always return void
  def initialize(x)
    @x = x
  end
end

class SingleLineNoAfterStatements
  extend T::Sig

  sig {params(x: T.any(Integer, String)).returns(T.any(Integer, String))}
  #                                      ^^^^^^^ error: The initialize method should always return void
  def initialize(x)
    @x = x
  end
end

class ClassMethodInitializeIsIgnored
  extend T::Sig

  sig {returns(ClassMethodInitializeIsIgnored)}
  def self.initialize
    new
  end
end

class LineBreakAfterReturns
  extend T::Sig

  sig do
    params(
      path: String,
      key: String
    )
    .returns(T.self_type)
    #^^^^^^^ error: The initialize method should always return void
    .checked(:tests)
  end
  def initialize(path, key)
    self
  end
end

class LineBreakOnlyAtEnd
  extend T::Sig

  sig do
    params(
      path: String,
      key: String
    )
    .returns(T.self_type).checked(:tests)
    #^^^^^^^ error: The initialize method should always return void
  end
  def initialize(path, key)
    self
  end
end

class TProcReturnsInInitializeSig
  extend T::Sig
  sig { params(blk: T.proc.returns(T.untyped)).returns(T.anything) }
  #                                            ^^^^^^^ error: The initialize method should always return void
  def initialize(&blk)
  end
end
