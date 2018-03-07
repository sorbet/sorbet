# @typed

class Main
    def wild(*args)
    end

    def double_wild(**args)
    end

    def one_required(foo, **args)
    end

    sig(foo: Integer, args: String).returns(NilClass)
    def one_kwarg(foo:, **args)
    end

    sig(args: Integer).returns(NilClass)
    def with_type(**args)
    end

    sig(x: String, y: Symbol).returns(NilClass)
    def opt_and_repeated_kw(x="hi", **y)
    end

  def main
        wild(a: 1)
        double_wild
        double_wild(a: 1)
        double_wild(a: 1, b: 2)
        one_required(a: 1)
        one_required(1, a: 1)
        one_required(1, 2, a: 1) # error: Too many arguments provided for method
        one_kwarg(foo: 1)
        one_kwarg(foo: 1, a: "a")
        one_kwarg(foo: "bad", a: "bad") # error: Argument foo does not match expected type
        one_kwarg(foo: 1, a: 1) # error: Argument args does not match expected type
        one_kwarg(foo: "bad", a: 1) # error: MULTI
        with_type
        with_type(a: 1)
        with_type(a: "bad") # error: Argument args does not match expected type
        with_type(a: "bad", b: "bad") # error: MULTI

        # This should assign `z`, instead of assigning `x={z: :foo}`,
        # which would happen with `y={}`
        opt_and_repeated_kw(z: :foo)
        opt_and_repeated_kw("hi")
        opt_and_repeated_kw("hi", z: "foo") # error: Argument y does not match expected type Symbol
    end
end
