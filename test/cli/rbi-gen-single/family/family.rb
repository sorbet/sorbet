# typed: true

module Family

  class Simpsons
    extend T::Sig

    # These two aliases will have the same RHS in the rbi file that's generated.
    MaybeBart = T.type_alias{T.nilable(Bart::Character)}
    MaybeBartFull = T.type_alias{T.nilable(Family::Bart::Character)}

    sig {returns(MaybeBart)}
    def no_bart
      nil
    end

    # These two constants will have the same RHS in the rbi file that's
    # generated.
    RelativeBart = Bart::Character
    FullyQualifiedBart = Family::Bart::Character

    sig {returns(Bart::Character)}
    def bart
      Bart::Character.new
    end
  end

end
