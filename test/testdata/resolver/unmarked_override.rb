# typed: true
# enable-suggest-unsafe: true

class A
  extend T::Sig

  sig { returns(Integer) }
  def foo; 0; end
end

class B < A
  extend T::Sig

  sig { returns(String) }
  def foo; ""; end
# ^^^^^^^ error: Return type `String` does not match return type of overridden method `A#foo`
end

# don't validate if no child sig
class C < A
  def foo(x); ""; end
end

class D < A
  extend T::Sig

  sig { params(x: Integer).returns(Integer) }
  def foo(x)
# ^^^^^^^^^^ error: Override of method `A#foo` must accept no more than `0` required argument(s)
    0
  end
end

class E < A
  extend T::Sig

  sig { returns(Integer).params(x: Integer) }
  def foo(x)
# ^^^^^^^^^^ error: Override of method `A#foo` must accept no more than `0` required argument(s)
    0
  end
end

class F < A
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { void.abstract }
  def foo; end
# ^^^^^^^ error: Method `F#foo` is marked as abstract, but overrides the non-abstract method `A#foo`
# ^^^^^^^ error: Return type `void` does not match return type of overridden method `A#foo`
end

class G < A
  extend T::Sig

  sig { returns(Integer) }
  private def foo; 0; end
#         ^^^^^^^ error: Method `foo` is private in `G` but not in `A`
end
