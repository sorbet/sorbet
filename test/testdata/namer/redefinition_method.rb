# typed: true
class Main
    extend T::Sig

    sig {params(a: Integer).returns(Integer)}
    def foo(a)
        a
    end

    def foo(a, b) # error: Method `Main#foo` redefined without matching argument count. Expected: `1`, got: `2`
    end

    def foo(a, b, c) # error: Method `Main#foo` redefined without matching argument count. Expected: `2`, got: `3`
    end
end
