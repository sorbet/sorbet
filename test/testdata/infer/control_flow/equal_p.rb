# typed: true
class Module
  include T::Sig
end

class Main
  sig {params(x: T.any(Integer, String)).void}
  def self.test1(x)
    if x.equal?(0)
      T.assert_type!(x, Integer)
    else
      T.assert_type!(x, T.any(Integer, String))
    end
  end
end
