# @typed
class Side
    def foo
        a = self
        a.bar(a, if true; a = true; else a = 2; end, a);
    end

    def bar(a, b, c)
        puts a, b, c
    end
end
