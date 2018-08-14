# typed: true
module Abbrev
  Sorbet.sig(
      words: T::Array[String],
  )
  .returns(T::Hash[String, String])
  def self.abbrev(words); end
end
