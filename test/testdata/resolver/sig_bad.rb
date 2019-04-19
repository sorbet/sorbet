# typed: true
class T1; end
class T2; end

class T::Private::Methods::SigBuilder
  def unsupported
  end
end

class A
  extend T::Sig

  def self.unsupported; end

  sig do
    params(
      a: unsupported, # error: Unknown type syntax
      b: T.enum, # error: Not enough arguments provided for method `T.enum`. Expected: `1`, got: `0`
      c: T.enum(1),
      d: T.enum([]), # error: enum([]) is invalid
      e: T.enum([unsupported]), # error: Unsupported type literal
      f: 0, # error: Unsupported type syntax
      g: T.any(*[Integer, String]), # error: Splats are unsupported by the static checker
      h: T.junk, # error: Unsupported method `T.junk`
       # ^^^^^^ error: Method `junk` does not exist on `T.class_of(T)`
      i: T.class_of(T1, T2), # error: Too many arguments provided for method `T.class_of`
      j: T.class_of(T.nilable(Integer)), # error: T.class_of needs a Class as its argument
      k: T.class_of(1), # error: T.class_of needs a Class as its argument
      l: {[] => String}, # error: Shape keys must be literals
      m: {foo: 0}, # error: Unsupported type syntax
      n: T.all, # error: Not enough arguments provided for method
      o: T.any, # error: Not enough arguments provided for method
    )
    .returns(T2)
  end
  def bad(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o)
    T2.new
  end
end
