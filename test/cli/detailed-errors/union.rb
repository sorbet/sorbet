# typed: true

module Union
  extend T::Sig

  module M1
  end

  module M2
  end

  module M3
  end

  class C1
    include M1
  end

  class C2
    include M2
  end

  class C3
    include M3
  end

  class C13
    include M1
    include M3
  end

  class C12
    include M1
    include M2
  end

  # T.any < _
  C1_or_C2 = T.let(C1.new, T.any(C1, C2))
  T.let(C1_or_C2, M1)
  T.let(C1_or_C2, M2)

  # T.any < T.all
  C12_or_C13 = T.let(C12.new, T.any(C12, C13))
  T.let(C12_or_C13, T.all(M1, M2));
  T.let(C12_or_C13, T.all(M1, M3));

  # T.any < T.any
  T.let(C1_or_C2, T.any(C1, C3))
  T.let(C1_or_C2, T.any(C2, C3))

  C1_or_C2_or_C3 = T.let(C1.new, T.any(T.nilable(T.any(C1, C2)), C3))
  T.let(C1_or_C2_or_C3, T.any(M1, M3))
end
