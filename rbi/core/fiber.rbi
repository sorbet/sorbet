# typed: strict

class Fiber < Object
  sig {returns(Fiber)}
  def current; end

  sig {returns(T.any(TrueClass, FalseClass))}
  def alive?; end
end
