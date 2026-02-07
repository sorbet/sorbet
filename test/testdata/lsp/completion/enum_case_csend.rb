# typed: true
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

sig { params(my_enum: T.nilable(MyEnum)).void }
def example(my_enum)
  # We don't handle csend well right now, because we can't see the receiver loc
  #
  # We still generate the case arms, so the user could presumably address this
  # manually, but it's not clear why the user typed `&.` if they were expecting
  # to produce a `case` completion in the first place.

  my_enum&.cas
  #        ^^^ error: does not exist
  #           ^ apply-completion: [A] item: 0
end
