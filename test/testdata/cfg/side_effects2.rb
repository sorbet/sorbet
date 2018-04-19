# typed: strict
class Side
    def foo(cond)
        a = self
        a.bar(a, if cond; a = true; else a = 2; end, a);
    end

    def bar(a, b, c)
        puts a, b, c
    end
end
