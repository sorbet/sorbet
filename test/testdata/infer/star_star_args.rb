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
    end
end
