# typed: strict

class ActiveSupport::TestCase
end

class MyTest < ActiveSupport::TestCase
  extend T::Sig
  # Helper instance method
  sig { params(test: T.untyped).returns(T::Boolean) }
  def assert(test)
    test ? true : false
  end

  # Helper method to direct calls to `test` instead of Kernel#test
  sig { params(args: T.untyped, block: T.nilable(T.proc.void)).void }
  def self.test(*args, &block)
  end

  setup do
    @a = T.let(1, Integer)
    foo # error: Method `foo` does not exist on `MyTest`
  end

  setup do
    bar # error: Method `bar` does not exist on `MyTest`
  end

  # Method calls that shouldn't be rewritten
  tesst "invalid", "method name" do # error: Method `tesst` does not exist on `T.class_of(MyTest)`
  end

  test "invalid", "parameter count" do
  end

  test "no block argument"

  test :not_a_string do
  end

  test :not_a_string do
    assert true # error: Method `assert` does not exist on `T.class_of(MyTest)`
  end
  # end unmodified method calls

  test "valid method call" do
  end

  test "block is evaluated in the context of an instance" do
    assert true
  end

  teardown do
    fiz # error: Method `fiz` does not exist on `MyTest`
  end

  teardown do
    baz # error: Method `baz` does not exist on `MyTest`
  end
end

class NoMatchTest < ActiveSupport::TestCase
  extend T::Sig

  sig { params(block: T.proc.void).void }
  def self.setup(&block); end

  sig { params(block: T.proc.void).void }
  def self.teardown(&block); end

  setup do
    foo
  # ^^^ error: Method `foo` does not exist on `T.class_of(NoMatchTest)`
  end

  teardown do
    bar
  # ^^^ error: Method `bar` does not exist on `T.class_of(NoMatchTest)`
  end
end

class NoParentClass
  extend T::Sig

  sig { params(block: T.proc.void).void }
  def self.setup(&block); end

  sig { params(block: T.proc.void).void }
  def self.teardown(&block); end

  sig { params(a: T.untyped, b: T.untyped).void }
  def assert_equal(a, b); end

  setup do
    @a = T.let(1, Integer)
  end

  test "it works" do
    assert_equal 1, @a
  end

  teardown do
    @a = 5
  end
end
