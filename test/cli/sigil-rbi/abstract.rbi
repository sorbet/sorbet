# typed: strict

class A
    extend T::Sig

    abstract!
    sig {abstract.void}
    def abstract
    end
end

class B < A
end
