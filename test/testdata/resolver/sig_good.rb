# typed: strict
class T1; end
class T2; end
module M1; end
A1 = T1

class A
  sig(
    a: [T1, T2],
    b: T1,
    c: T.nilable(T1),
    d: T.any(T1, T2),
    e: T.untyped,
    f: T::Array[T1],
    g: T::Hash[T1, T2],
    h: T.enum([false, 1, 3.14, "foo", :bar]),
    i: A1,
    j: T1.singleton_class,
    k: A1.singleton_class,
    l: T.class_of(T1),
    m: T.class_of(A1),
    n: T.class_of(M1),
    o: {
      foo: String,
      bar: T.nilable(Integer),
    },
    p: {},
    q: {"hi" => String, :bye => String},
  )
  .returns(T2)
  def good(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q)
  end
end
