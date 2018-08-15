# typed: strict
class C
  L = T.let(lambda { |x| x*x }, T.proc(x: Integer).returns(Integer))
end
