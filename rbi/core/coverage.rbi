# typed: true
module Coverage
  Sorbet.sig.returns(T::Hash[String, T::Array[T.nilable(Integer)]])
  def self.result(); end

  Sorbet.sig.returns(NilClass)
  def self.start(); end
end
