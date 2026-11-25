# typed: strict

module Root::A
  class Marked
    package_private!

    X = 1
    package_private!(:X)

    Alias = String
    package_private!(:Alias)

    TypeAlias = T.type_alias { Integer }
    package_private!(:TypeAlias)

    # The marker comes before the constant.
    package_private!(:Later)
    Later = 1
  end

  class MyEnum < T::Enum
    enums do
      Value = new
      Other = new
    end
    package_private!(:Value)
  end

  module Plain
    # Not marked.
    Y = 1
  end
end
