# typed: true

class A extend T::Sig
  sig {params(x: Integer).returns(String)}
  def bar(x:)
    x.to_s
  end
end
  