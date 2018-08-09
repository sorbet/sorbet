# typed: strict

class Main
    extend T::Helpers

    sig(val: T::Array[String]).returns(T.nilable(String))
    def array(val)
        val[0]
    end

    sig(val: T::Hash[Symbol, String]).returns(String)
    def hash(val)
        val.fetch(:zero)
    end

    sig(val: T::Enumerable[Symbol]).returns(T.nilable(Symbol))
    def enumerable(val)
        val.first
    end

    sig(val: T::Range[Integer]).returns(Integer)
    def range(val)
        val.first
    end

    sig(val: T::Set[String]).returns(T.nilable(String))
    def set(val)
        val.first
    end
end
