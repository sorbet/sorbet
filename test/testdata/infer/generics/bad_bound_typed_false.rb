# typed: false
#
class Animal; end
class Cat < Animal; end

class A
  extend T::Generic
  X = type_member(lower: Cat, upper: Animal)
end

class Test
  extend T::Sig

  # NOTE, there are two errors raised here, one for each bound not satisfied.
  sig {params(x: A[String]).void}
                 # ^^^^^^ error-with-dupes: `String` cannot be used for type member
  def foo(x); end
end
