# typed: strict

class ActiveSupport::TestCase
  extend T::Sig

  # Helper method to direct calls to `test` instead of Kernel#test
  sig { params(args: T.untyped, block: T.nilable(T.proc.bind(T.attached_class).void)).void }
  def self.test(*args, &block)
  end

  # Helper instance method
  sig { params(test: T.untyped).returns(T::Boolean) }
  def assert(test)
    test ? true : false
  end
end

class MyTest < ActiveSupport::TestCase
  extend T::Sig

  setup do
    @a = T.let(1, Integer)
    foo # error: Method `foo` does not exist on `MyTest`
  end

  setup do
    bar # error: Method `bar` does not exist on `MyTest`
  end

  test "valid method call" do
  end

  test "block is evaluated in the context of an instance" do
    assert true
  end
end

class NoMatchTest < ActiveSupport::TestCase
  extend T::Sig

  sig { params(block: T.proc.bind(T.attached_class).void).void }
  def self.setup(&block); end

  setup do
    @a = T.let(1, Integer)
  # ^^ error: The instance variable `@a` must be declared inside `initialize` or declared nilable
    foo
  # ^^^ error: Method `foo` does not exist on `NoMatchTest`
  end
end

class NoParentClass
  extend T::Sig

  sig { params(block: T.proc.bind(T.attached_class).void).void }
  def self.setup(&block); end

  sig { params(block: T.proc.bind(T.attached_class).void).void }
  def self.teardown(&block); end

  sig { params(name: String, block: T.proc.bind(T.attached_class).void).void }
  def self.test(name, &block); end

  sig { params(a: T.untyped, b: T.untyped).void }
  def assert_equal(a, b); end

  setup do
    @a = T.let(1, Integer)
  end

  test "it works" do
    assert_equal(1, @a)
  end
end
