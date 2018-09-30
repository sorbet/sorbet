# typed: true
module Abbrev
  sig do
    params(
        words: T::Array[String],
    )
    .returns(T::Hash[String, String])
  end
  def self.abbrev(words); end
end
