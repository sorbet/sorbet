# typed: true

class A
  extend T::Sig

  sig do
    type_parameters(:U)
      .params(
        x: T.type_parameter(:U),
        f: T.proc.params(y: T.type_parameter(:U)).void,
      )
      .returns(T.proc.params(z: T.type_parameter(:U)).returns(T.type_parameter(:U)))
  end
  def self.example(x, f)
    res1 = f.call(x)
    T.reveal_type(res1) # error: `Sorbet::Private::Static::Void`

    g = T.let(->(z) { x }, T.proc.params(z: T.type_parameter(:U)).returns(T.type_parameter(:U)))

    res2 = g.call(x)
    T.reveal_type(res2) # error: `T.type_parameter(:U) (of A.example)`
    res3 = f.call(res2)
    T.reveal_type(res3) # error: `Sorbet::Private::Static::Void`
    g
  end

  sig do
    type_parameters(:U)
      .params(
        blk: T.proc.bind(T.type_parameter(:U)).void # error: Malformed `bind`: Can only bind to simple class names
      )
      .returns(T.proc.bind(T.type_parameter(:U)).void) # error: Malformed `bind`: Can only bind to simple class names
  end
  def self.example_bad(&blk)
    f = T.let(blk, T.proc.bind(T.type_parameter(:U)).void)
    #              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `bind`: Can only bind to simple class names
    f
  end
end

int = T.let(0, Integer)
takes_int_or_str = T.let(->(x) {}, T.proc.params(y: T.any(Integer, String)).void)
res_f = A.example(int, takes_int_or_str)
# TypeScript on an equivalent example infers `(z: number) => number`, instead
T.reveal_type(res_f) # error: `T.proc.params(arg0: T.any(Integer, String)).returns(T.any(Integer, String))`
