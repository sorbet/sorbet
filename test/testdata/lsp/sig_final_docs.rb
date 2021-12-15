# typed: true
class A
  extend T::Sig

  # These docs should show!
  sig(:final) do
    params(x: Integer).returns(String)
  end
  def bar(x)
    # ^ hover: These docs should show!
    x.to_s
  end
end
