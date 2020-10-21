# typed: true

class T1; end
class T2; end
class T3; end

# Check keyword parameters ordering
#
# Required keyword parameters must be before the optional ones.
class A
  extend T::Sig

  sig {params(x: T1, y: T2, z: T3).void}
  def f1(x:, y:, z: T3.new); end

  sig {params(x: T1, y: T2, z: T3).void}
  def f2(x:, y: T2.new, z:); end
  #                     ^^ error: Malformed `sig`. Required parameter `z` must be declared before all the optional ones
end

# Check parameters order match between signature and declaration
class B
  extend T::Sig

  sig { params(a: Integer, b: String).void }
  def f1(a, b); end

  sig { params(a: Integer, b: String).void }
  def f2(b, a); end
  #      ^ error: Bad parameter ordering for `b`, expected `a` instead
  #         ^ error: Bad parameter ordering for `a`, expected `b` instead

  sig {params(v: T1, w: T2, x: T1, y: T2, z: T3).void}
  def f3(v, w = T2.new, *y, z:, x: T1.new); end
  #                      ^ error: Bad parameter ordering for `y`, expected `x` instead
  #                         ^^ error: Bad parameter ordering for `z`, expected `y` instead
  #                             ^^ error-with-dupes: Bad parameter ordering for `x`, expected `z` instead
end
