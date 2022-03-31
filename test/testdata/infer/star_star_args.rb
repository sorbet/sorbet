# typed: true

class Main
    extend T::Sig

    def wild(*args)
    end

    def double_wild(**args)
    end

    def one_required(foo, **args)
    end

    sig {params(foo: Integer, args: String).returns(NilClass)}
    def one_kwarg(foo:, **args)
    end

    sig {params(args: Integer).returns(NilClass)}
    def with_type(**args)
    end

    sig {params(x: String, y: Symbol).returns(NilClass)}
    def opt_and_repeated_kw(x="hi", **y)
    end

  def main
        wild(a: 1)
        double_wild
        double_wild(a: 1)
        double_wild(a: 1, b: 2)
        one_required(a: 1)
        one_required(1, a: 1)
        one_required(1, 2, a: 1)
        #               ^ error: Expected: `1`, got: `2`
        one_kwarg(foo: 1)
        one_kwarg(foo: 1, a: "a")

        one_kwarg(foo: "bad", a: "bad")
        #              ^^^^^ error: Expected `Integer` but found `String("bad")` for argument `foo`
        one_kwarg(foo: 1, a: 1)
        #         ^^^^^^^^^^^^ error: Expected `String` but found `Integer(1)` for argument `args`
        one_kwarg(foo: "bad", a: 1)
        #              ^^^^^ error: Expected `Integer` but found `String("bad")` for argument `foo`
        #         ^^^^^^^^^^^^^^^^ error: Expected `String` but found `Integer(1)` for argument `args`
        with_type
        with_type(a: 1)
        with_type(a: "bad")
        #         ^^^^^^^^ error: Expected `Integer` but found `String("bad")` for argument `args`
        with_type(a: "bad", b: "bad2")
        #         ^^^^^^^^^^^^^^^^^^^ error: Expected `Integer` but found `String("bad")` for argument `args`
        #         ^^^^^^^^^^^^^^^^^^^ error: Expected `Integer` but found `String("bad2")` for argument `args`

        # This should assign `z`, instead of assigning `x={z: :foo}`,
        # which would happen with `y={}`
        opt_and_repeated_kw(z: :foo)
        opt_and_repeated_kw("hi")
        opt_and_repeated_kw("hi", z: "foo")
        #                         ^^^^^^^^ error: Expected `Symbol` but found `String("foo")` for argument `y`
  end
end
