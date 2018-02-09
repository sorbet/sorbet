# @typed

class Main
    sig(val: T::Array[String]).returns(String)
    def array(val)
        val[0]
    end

    sig(val: T::Hash[Symbol, String]).returns(String)
    def hash(val)
        val[:zero]
    end

    sig(val: T::Enumerable[Symbol]).returns(T.nilable(Symbol))
    def enumerable(val)
        val.first
    end
end
