# typed: core

class Fiber < Object
  sig {returns(Fiber)}
  def current; end

  sig {returns(T::Boolean)}
  def alive?; end
end

class FiberError < StandardError
end
