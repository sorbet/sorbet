# typed: true
class A
  extend T::Sig

  sig {void}
  def initialize()
    yield
    1
  end
end

b = A.new {}
T.assert_type!(b, A)
