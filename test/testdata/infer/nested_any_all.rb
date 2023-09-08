# typed: true

module M1; end
module M2; end

module A; end
module B; end

module Main
  extend T::Sig

  sig {params(x: T.all(M1, T.any(A, B), M2)).returns(T.any(A, B))}
  def self.test1(x)
    T.reveal_type(x) # error: `T.all(M1, T.any(A, B), M2)`
    x
  end

  # remove M2, shouldn't matter
  sig {params(x: T.all(M1, T.any(A, B))).returns(T.any(A, B))}
  def self.test2(x)
    T.reveal_type(x) # error: `T.all(M1, T.any(A, B))`
    x
  end

  # reorder T.any and M2, shouldn't matter
  sig {params(x: T.all(M1, M2, T.any(A, B))).returns(T.any(A, B))}
  def self.test3(x)
    T.reveal_type(x) # error: `T.all(M1, M2, T.any(A, B))`
    x
  end

  # associate the other way, shouldn't matter
  sig {params(x: T.all(M1, T.all(M2, T.any(A, B)))).returns(T.any(A, B))}
  def self.test4(x)
    T.reveal_type(x) # error: `T.all(M2, T.any(A, B), M1)`
    x
  end
end
