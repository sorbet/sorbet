# typed: strict

module Foobar extend T::Sig
  sig {params(s: T::Array[Integer]).returns(String)}
  def self.bar(s)
    "${s[0]}"
  end
end
