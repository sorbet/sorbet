# typed: strict

class Fiber < Object
  sig {returns(Fiber)}
  def current; end

  sig {returns(T::Boolean)}
  def alive?; end
end
