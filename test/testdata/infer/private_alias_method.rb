# typed: true

# This case matches what people actually tend to write
class A
  def foo(x); puts 'A#foo'; end

  alias_method :private_original_foo, :foo
  private :private_original_foo

  def foo(x)
    return "custom validation failed" unless x.even?
    private_original_foo
  end

  def inspect
    '#<A:...>'
  end
end

# This test doesn't, but I figure we may as well test what happens.
class A
  def foo(x); puts 'A#foo'; end

  private :private_original_foo
  alias_method :private_original_foo, :foo

  def foo(x)
    return "custom validation failed" unless x.even?
    private_original_foo
  end

  def inspect
    '#<A:...>'
  end
end
