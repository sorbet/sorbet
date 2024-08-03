# typed: true

extend T::Sig

class MParent
  extend T::Generic
  Elem = type_member(:out)
end

class MChild < MParent
  extend T::Generic
  Elem = type_member(:out)
end

module Other1; end
module Other2; end

sig {
  params(
    x1: T.all(Other2, MParent[Other1], MChild[T.anything]),
    x2: T.all(MParent[Other1], Other2, MChild[T.anything]),
    x3: T.all(MParent[Other1], MChild[T.anything], Other2),
  ).void
}
def example(x1, x2, x3)
  T.reveal_type(x1) # error: `T.all(MParent[Other1], Other2, MChild[T.anything])`
  takes_mchild_other1(x1) # error: but found
  T.reveal_type(x2) # error: `T.all(MParent[Other1], Other2, MChild[T.anything])`
  takes_mchild_other1(x2) # error: but found
  T.reveal_type(x3) # error: `T.all(MChild[Other1], Other2)`
  takes_mchild_other1(x3)
end

sig { params(mchild_other1: MChild[Other1]).void }
def takes_mchild_other1(mchild_other1)
end



