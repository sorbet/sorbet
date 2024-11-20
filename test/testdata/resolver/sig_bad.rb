# typed: true
class T1; end
class T2; end

class T::Private::Methods::DeclBuilder
  def unsupported
  end
end

class A
  extend T::Sig

  def self.unsupported; end

  sig do
    params(
      a: unsupported, # error: Unknown type syntax
      b: T.enum,
      #    ^^^^ error: Method `enum` does not exist on `T.class_of(T)`
      c: T.enum(1),
      #  ^^^^^^^^^ error: `T.enum` has been renamed to `T.deprecated_enum`
      #    ^^^^    error: Method `enum` does not exist on `T.class_of(T)`
      c1: T.enum(1.0),
      #   ^^^^^^^^^^^ error: `T.enum` has been renamed to `T.deprecated_enum`
      #     ^^^^    error: Method `enum` does not exist on `T.class_of(T)`
      c2: T.enum("T"),
      #   ^^^^^^^^^^^ error: `T.enum` has been renamed to `T.deprecated_enum`
      #     ^^^^    error: Method `enum` does not exist on `T.class_of(T)`
      c3: T.enum(:T),
      #   ^^^^^^^^^^ error: `T.enum` has been renamed to `T.deprecated_enum`
      #     ^^^^    error: Method `enum` does not exist on `T.class_of(T)`
      d: T.enum([]),
      #  ^^^^^^^^^^ error: `T.enum` has been renamed to `T.deprecated_enum`
      #  ^^^^^^^^^^ error: enum([]) is invalid
      #    ^^^^     error: Method `enum` does not exist on `T.class_of(T)`
      e: T.enum([unsupported]),
      #  ^^^^^^^^^^^^^^^^^^^^^ error: `T.enum` has been renamed to `T.deprecated_enum`
      #          ^^^^^^^^^^^   error: Unsupported type literal
      #    ^^^^                error: Method `enum` does not exist on `T.class_of(T)`
      b1: T.deprecated_enum, # error: Not enough arguments provided for method `T.deprecated_enum`. Expected: `1`, got: `0`
      c11: T.deprecated_enum(1), # error: Expected `T.any(T::Set[T.anything], T::Array[T.anything])`
      c12: T.deprecated_enum(1.0), # error: Expected `T.any(T::Set[T.anything], T::Array[T.anything])`
      c13: T.deprecated_enum("d"), # error: Expected `T.any(T::Set[T.anything], T::Array[T.anything])`
      c14: T.deprecated_enum(:e), # error: Expected `T.any(T::Set[T.anything], T::Array[T.anything])`
      d1: T.deprecated_enum([]), # error: enum([]) is invalid
      e1: T.deprecated_enum([unsupported]), # error: Unsupported type literal
      f: 0, # error: Unsupported literal in type syntax
      g: T.any(*[Integer, String]), # error: splats cannot be used in types
      h: T.junk, # error: Unsupported method `T.junk`
      #    ^^^^ error: Method `junk` does not exist on `T.class_of(T)`
      i: T.class_of(T1, T2),
      #                 ^^ error: Too many arguments provided for method `T.class_of`
      j: T.class_of(T.nilable(Integer)), # error: `T.class_of` must wrap each individual class type, not the outer `T.any`
      k: T.class_of(1),
      #  ^^^^^^^^^^^^^ error: `T.class_of` needs a class or module as its argument
      #             ^  error: Unsupported literal in type syntax
      #             ^  error: Unexpected bare `Integer(1)` value found in type position
      k1: T.class_of(1.0),
      #   ^^^^^^^^^^^^^^^ error: `T.class_of` needs a class or module as its argument
      #              ^^^  error: Unsupported literal in type syntax
      #              ^^^  error: Unexpected bare `Float(1.000000)` value found in type position
      k2: T.class_of("a"),
      #   ^^^^^^^^^^^^^^^ error: `T.class_of` needs a class or module as its argument
      #              ^^^  error: Unsupported literal in type syntax
      #              ^^^  error: Unexpected bare `String("a")` value found in type position
      k3: T.class_of(:b),
      #   ^^^^^^^^^^^^^^ error: `T.class_of` needs a class or module as its argument
      #              ^^  error: Unsupported literal in type syntax
      #              ^^  error: Unexpected bare `Symbol(:b)` value found in type position
      l: {[] => String}, # error: Shape keys must be literals
      m: {foo: 0}, # error: Unsupported literal in type syntax
      n: T.all, # error: Not enough arguments provided for method
      o: T.any, # error: Not enough arguments provided for method
    )
    .returns(T2)
  end
  def bad(a, b, c, c1, c2, c3, d, e, b1, c11, c12, c13, c14, d1, e1, f, g, h, i, j, k, k1, k2, k3, l, m, n, o)
    T2.new
  end
end
