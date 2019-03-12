# typed: strict

module Timeout
  sig do
    type_parameters(:U).params(
      sec: T.nilable(Numeric),
      klass: T.any(NilClass, T.class_of(Exception)),
      message: T.untyped,
      blk: T.proc.params(sec: T.nilable(Numeric)).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  def self.timeout(sec, klass = nil, message = nil, &blk); end
end

class Timeout::Error < RuntimeError
  sig { params(msg: T.untyped, blk: T.proc.params(exc: Timeout::Error).void).void }
  def self.catch(msg = nil, &blk); end

  sig { returns(T.nilable(Thread)) }
  def thread; end

  sig { params(msg: T.untyped).void }
  def initialize(msg = nil)
    @thread = T.let(T.unsafe(nil), T.nilable(Thread))
  end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def exception(*_); end

  Sorbet.sig {returns(::T.untyped)}
  def thread(); end
end
