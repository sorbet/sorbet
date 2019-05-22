# typed: true

class A extend T::Sig
  sig {generated.params(x: Integer).returns(String)}
  def bar(x)
    x.to_s
  end
end
