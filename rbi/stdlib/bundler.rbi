# typed: true

module Bundler
  Sorbet.sig {returns(::T.untyped)}
  def self.load(); end
end
