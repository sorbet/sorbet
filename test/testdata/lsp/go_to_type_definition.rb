# typed: true

class A; end
#     ^ type-def: A
#     ^ type-def: AB
#     ^ type-def: ABMN

class B
#     ^ type-def: B
#     ^ type-def: AB
#     ^ type-def: ABMN
  extend T::Sig

  sig {returns(A)}
  def self.returns_A; A.new; end
end

module M; end
#     ^ type-def: MN
#     ^ type-def: ABMN
module N; end
#     ^ type-def: MN
#     ^ type-def: ABMN

class TestClass
  a_inst = A.new
  # ^ type: A
  puts a_inst
  #    ^ type: A

  puts B.returns_A
  #      ^ type: A

  a_cls = A
  # ^ type: A
  puts a_cls
  puts A
  #    ^ type: A

  a_or_b = T.let(A.new, T.any(A, B))
  puts a_or_b
  #    ^ type: AB

  m_or_n = T.cast(nil, T.all(M, N))
  puts m_or_n
  #    ^ type: MN

  a_or_b_or_mn = T.let(A.new, T.any(A, B, T.all(M, N)))
  puts a_or_b_or_mn
  #    ^ type: ABMN
end
