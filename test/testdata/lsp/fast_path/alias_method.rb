# typed: true

class A
  def paid; end

  alias_method :paid?, :paid
end
