class T1; end
class T2; end

class A
  standard_method(
    {
      a: [T1, T2],
      b: T1,
      c: Opus::Types.nilable(T1),
      d: Opus::Types.any(T1, T2),
      e: unsupported,
    }, returns: T2)
  def f(a, b, c, d, e)
  end
end
