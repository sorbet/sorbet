# typed: strict
extend T::Sig

class Letter < T::Enum
  enums do
    V0 = new
    V1 = new
    V2 = new
    V3 = new
    V4 = new
    V5 = new
    V6 = new
    V7 = new
    V8 = new
    V9 = new
    V10 = new
    V11 = new
    V12 = new
    V13 = new
    V14 = new
    V15 = new
    V16 = new
    V17 = new
    V18 = new
    V19 = new
  end
end

# Locals holding `==` comparisons against an enum this large carry knowledge that narrows it to unions of
# almost every value, which `KnowledgeRef::under` compares against the environment at every merge.

sig {params(x: Letter).void}
def aliased_comparisons(x)
  a = x == Letter::V19 || x == Letter::V18
  b = x == Letter::V17 || x == Letter::V16
  if a
    T.reveal_type(x) # error: Revealed type: `T.any(Letter::V18, Letter::V19)`
  end
  if !(x == Letter::V15 || x == Letter::V14)
    if b
      T.reveal_type(x) # error: Revealed type: `T.any(Letter::V16, Letter::V17, Letter::V18, Letter::V19)`
    end
  end
  if x == Letter::V17 || x == Letter::V16
    if a
      T.reveal_type(x) # error: Revealed type: `T.any(Letter::V17, Letter::V16)`
    end
  end
end
