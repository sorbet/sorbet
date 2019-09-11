# typed: false
class T1; end
class T2; end
module M1; end
A1 = T1

class A
  sig do
    params(
      a: [T1, T2],
      b: T1,
      c: T.nilable(T1),
      d: T.any(T1, T2),
      e: T.untyped,
      f: T::Array[T1],
      g: T::Hash[T1, T2],
      h: T.enum([false, 1, 3.14, "foo", :bar]),
      i: A1,
      j: T.class_of(T1),
      k: T.class_of(A1),
      l: T.class_of(M1),
      m: {
        foo: String,
        bar: T.nilable(Integer),
      },
      n: {},
      o: {"hi" => String, :bye => String},
    )
    .returns(T2)
  end
  def good(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o)
  end
end
