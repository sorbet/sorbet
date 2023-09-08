# typed: true

CompatibleOverride = Struct.new(:foo) do
  def initialize(foo)
    super
  end
end

MismatchKeywordInit = Struct.new(:foo, keyword_init: true) do
  def initialize(foo)
    super
  end
end

DifferentArity = Struct.new(:a, :b, keyword_init: true) do
  def initialize(a)
    super(a: a, b: "b")
  end
end

foo = DifferentArity.new("a")
