# typed: true
class Main
    extend T::Sig

    sig {params(a: Integer).returns(Integer)}
    def foo(a)
        a
    end

    def foo(a, b) # error: redefined
    end

    def foo(a, b, c) # error: redefined
    end
end
