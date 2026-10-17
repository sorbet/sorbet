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

class C0; end
class C1; end
class C2; end
class C3; end
class C4; end
class C5; end
class C6; end
class C7; end
class C8; end
class C9; end
class C10; end
class C11; end
class C12; end
class C13; end
class C14; end
class C15; end
class C16; end
class C17; end
class C18; end
class C19; end

# Unions of at least 16 classes take faster paths in `isSubType` and `glb`.

sig {params(x: Letter).void}
def narrowing(x)
  a = x == Letter::V19 || x == Letter::V18 || x == Letter::V17
  b = x == Letter::V18 || x == Letter::V17 || x == Letter::V16
  if a && b
    T.reveal_type(x) # error: Revealed type: `T.any(Letter::V17, Letter::V18)`
  end
  return if x == Letter::V0 || x == Letter::V19
  if a
    T.reveal_type(x) # error: Revealed type: `T.any(Letter::V18, Letter::V17)`
  end
  if b
    T.reveal_type(x) # error: Revealed type: `T.any(Letter::V18, Letter::V17, Letter::V16)`
  end
end

sig {params(x: T.any(C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19)).void}
def union_subtyping(x)
  return if x.is_a?(C19) || x.is_a?(C18) || x.is_a?(C17)
  T.let(x, T.any(C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16))
  return if x.is_a?(C16)
  T.let(x, T.any(C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16)) # error: Argument does not have asserted type `T.any(C1, C2,
end
