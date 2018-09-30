# typed: strict

class A
    extend T::Helpers

    abstract!
    sig {abstract.void}
    def abstract
    end
end

class B < A
end
