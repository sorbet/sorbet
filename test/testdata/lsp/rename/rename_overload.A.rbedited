# typed: true

class Main
    extend T::Sig

    sig {params(val: T::Set[String]).returns(T.nilable(String))}
    def set(val)
        val.first
          # ^ apply-rename: [A] newName: second placeholderText: first invalid: true
    end
end
