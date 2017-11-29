def bar(a)
    a
end

class Splatable
    def to_a
        [1]
    end
end

class Rescueable
    def to_a
        [String, RuntimeError]
    end
end

class Parent
    def foo(a,b)
        [b, a]
    end
end
class Child < Parent
    def foo
        a = [1,2]
        super(*a)
    end
end

def foo
    a = [1]
    [*a, 2]
    [1, *a]
    [1, *a, 2]
    bar(*a)
    bar(*Splatable.new)
    Child.new.foo
    b = *Splatable.new

    case(1)
    when *Integer
    end

    begin
        raise "a"
    rescue *Rescueable.new
    end

    begin
        raise "a"
    rescue Array, *Rescueable.new, Float
    end
end

foo
