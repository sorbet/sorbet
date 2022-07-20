# typed: true

class A
  def target; end

  alias_method :inner, :target

  alias_method :outer, :inner
end
