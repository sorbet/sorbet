# typed: strict

module Timeout
  Sorbet.sig do
    params(
      sec: ::T.untyped,
      klass: ::T.untyped,
      message: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.timeout(sec, klass=T.unsafe(nil), message=T.unsafe(nil)); end
end

class Timeout::Error < RuntimeError
  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def exception(*_); end

  Sorbet.sig {returns(::T.untyped)}
  def thread(); end

  Sorbet.sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.catch(*args); end
end
