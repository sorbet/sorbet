# typed: true

module M1; end
module M2; end

class A; end
class B; end

module Main
  extend T::Sig

  sig {params(x: T.all(M1, T.any(M2, T.all(M1, T.any(A, B))))).void}
  def self.test(x)
    T.reveal_type(x) # error: `T.all(M1, T.any(M2, T.all(M1, T.any(A, B))))`
  end
end

# Once upon a time, this raised an exception like this:
#
# Exception::raise(): core/types/subtyping.cc:647 enforced condition Types::isSubType(gs, ret, t2) has failed:
# M1 & (M2 | (M1 & (A | B)))
#  is not a subtype of
# M2 | (M1 & (A | B))
# was glbbing with
# M1
#
# Backtrace:
#   #3 0x1b1c2a9 sorbet::core::Types::all()
