# typed: strict
# selective-apply-code-action: quickfix

class BooleanTypeSuggestion1
    extend T::Sig

    sig { void }
    def initialize
      @a = false # error: The instance variable `@a` must be declared using `T.let` when specifying `# typed: strict`
    end
end

class BooleanTypeSuggestion2
    extend T::Sig

    sig { void }
    def initialize
      @a = true # error: The instance variable `@a` must be declared using `T.let` when specifying `# typed: strict`
    end
end
