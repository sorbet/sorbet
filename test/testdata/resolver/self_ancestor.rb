# @typed

class A
    def self.hi
    end

    class B < self
        hi
        def self.foo
            hi
        end
    end
end

A::B.foo
