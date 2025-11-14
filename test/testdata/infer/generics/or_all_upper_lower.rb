# typed: true

module M; end
module N; end

class Upper
  extend T::Sig, T::Generic

  X = type_member { {upper: T.any(Integer, String)} }
  Y = type_member { {upper: T.all(M, N)} }
  Z = type_member { {upper: T.all(T.any(Integer, String), M, N)} }

  sig { params(x: T.any(Integer, String)).void}
  def takes_any(x)
  end

  sig { params(x: T.all(M, N)).void}
  def takes_all(x)
  end

  sig { params(x: X, y: Y, z: Z).void }
  def example(x, y, z)
    takes_any(x)
    takes_all(y)

    takes_any(z)
    takes_all(z)
  end
end

class Lower
  extend T::Sig, T::Generic

  X = type_member { {lower: T.all(M, N)} }
  Y = type_member { {lower: T.any(Integer, String)} }
  Z = type_member { {lower: T.any(T.all(M, N), Integer, String)} }

  sig { params(x: X).void }
  def takes_x(x)
  end

  sig { params(y: Y).void }
  def takes_y(y)
  end

  sig { params(z: Z).void }
  def takes_z(z)
  end

  sig {
    params(
      mn: T.all(M, N),
      i_or_s: T.any(Integer, String),
      mn_or_i_or_s: T.any(T.all(M, N), Integer, String),
    ).void
  }
  def example(mn, i_or_s, mn_or_i_or_s)
    takes_x(mn)
    takes_z(mn)

    takes_y(i_or_s)
    takes_z(i_or_s)

    takes_z(mn_or_i_or_s)
  end
end
