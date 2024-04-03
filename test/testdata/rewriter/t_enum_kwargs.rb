# typed: true

class Test < T::Enum
  extend T::Sig

  sig {params(a: Integer, b: String, c: Integer, d: Integer).void}
  def initialize(a, b="hi", c:, d: 20)
  end

  enums do
    A = new(10, c: 20)
    B = new(10, "foo", c: 20)
    C = new(10, c: 20, d: 30)
    D = new(10, "bar", c: 20, d: 30)

    E = new(10, "bar", c: 20, d: 30, f: "error")
    #                                ^^^^^^^^^^ error: Unrecognized keyword argument
    F = new(1, "bar", 3, 4, c: 20)
    #                 ^^^^ error: Too many positional arguments
  end

end
