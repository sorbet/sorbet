# typed: true

class A extend T::Sig
  sig {params(x: Integer).returns(String)}
  def bar(x:)
    x.to_s
  end
end

A.new.bar
#        ^ error: Missing required keyword argument `x` for method `A#bar`
A.new.bar(x: 0)
A.new.bar(y: 0)
#         ^^^^ error: Unrecognized keyword argument `y` passed for method `A#bar`
#         ^^^^ error: Missing required keyword argument `x` for method `A#bar`
