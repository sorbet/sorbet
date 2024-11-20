# typed: true

extend T::Sig

module M1
end

module M2
end

module M3
end

module M4
end

class C
end

class C1
  include M1
end

class C2
  include M2
end

class C124
  include M1
  include M2
  include M4
end

class C123
  include M1
  include M2
  include M3
end

class C134
  include M1
  include M3
  include M4
end

T.let(C.new, T.all(M1, M2))
T.let(C1.new, T.all(M1, M2))
T.let(C2.new, T.all(M1, M2))
T.let(C1.new, T.all(M1, M2, M3, M4))
T.let(C123.new, T.all(M1, M2, M3, M4))
T.let(C124.new, T.all(M1, M2, M3, M4))
T.let(C134.new, T.all(M1, M2, M3, M4))
