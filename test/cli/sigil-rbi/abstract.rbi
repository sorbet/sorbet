# typed: strict

class A
    abstract!
    sig.abstract.void
    def abstract
    end
end

class B < A
end
