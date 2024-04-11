# typed: true

extend T::Sig

module M1
end

module M2
end

class C
end

class C1
  include M1
end

class C2
  include M2
end

sig { params(x: T.all(M1, M2)).void }
def takes_intersection(x)
end

takes_intersection(C.new)
takes_intersection(C1.new)
takes_intersection(C2.new)
