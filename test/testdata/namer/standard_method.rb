class T1; end
class T2; end

class A
  standard_method(
    {
      a: [T1, T2],
      b: T1,
      c: Opus::Types.nilable(T1),
      d: Opus::Types.any(T1, T2),
      e: unsupported, # error: Unknown type syntax
      f: Opus::Types.untyped,
      g: Opus::Types.array_of(T1),
      h: Opus::Types.hash_of(keys: T1, values: T2),
    }, returns: T2)
  def f(a, b, c, d, e, f, g, h)
  end

  standard_method({}, returns: Opus::Types.noreturn)
  def noreturn
  end
end
