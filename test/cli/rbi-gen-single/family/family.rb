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

    # `::WhoKnows` is defined in external.rbi, and is not managed by any package.
    # In the resulting rbi for `family` it prints as just `WhoKnows`, but this
    # is fine as the enclosing class is not nested at all, and the constant will
    # resolve at the root.
    sig {returns(::WhoKnows)}
    def make_something
      WhoKnows.new
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

  class Flanders
    extend T::Sig
  end

  class Krabappel
    extend T::Sig
  end

end

