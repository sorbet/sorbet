# @typed
class Main
    sig(a: Integer).returns(Integer)
    def foo(a)
        a
    end

    def foo(a, b) # error: `foo`: Method redefined
    end

    def foo(a, b, c) # error: `foo`: Method redefined
    end
end
