# typed: strict

module Foobar extend T::Sig
  sig {params(s: Integer).returns(String)}
  def self.bar(s)
    "${s}"
  end
end
