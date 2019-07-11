# typed: true
class A
  extend T::Sig

  # Change 'String' to 'Integer'
  sig {params(x: Integer).returns(String)}
  def bar(x)
    x.to_s
  end
end

j = T.let(A.new.bar(10), String)
