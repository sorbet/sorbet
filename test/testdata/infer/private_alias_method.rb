# typed: true

# TODO(jez) Refactor alias_method support from resolver into namer.

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
class B
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

class C
  private def private_internal_api; end
  alias_method :__private_internal_api_PUBLIC_FOR_TESTING, :private_internal_api
end

C.new.private_internal_api # error: Non-private call to private method `private_internal_api`
C.new.__private_internal_api_PUBLIC_FOR_TESTING
