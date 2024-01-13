# typed: true

class A
  include Singleton
end

# Singleton supports inheritance, turning the sub-class into a singleton as well.
class B < A; end

T.reveal_type(A.instance) # error: Revealed type: `A`
T.reveal_type(B.instance) # error: Revealed type: `B`

class C
  include Singleton
  extend T::Helpers
  final!
end

T.reveal_type(C.instance) # error: Revealed type: `C`

class D < C; end # error: `C` was declared as final
