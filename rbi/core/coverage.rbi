# typed: true
module Coverage
  sig.returns(T::Hash[String, T::Array[T.nilable(Integer)]])
  def self.result(); end

  sig.returns(NilClass)
  def self.start(); end
end
