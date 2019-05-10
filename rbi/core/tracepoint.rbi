# typed: core

class TracePoint < Object
  sig do
    params(
        events: Symbol,
        blk: T.proc.params(tp: TracePoint).void,
    )
    .void
  end
  def initialize(*events, &blk); end

  sig {returns(T.untyped)}
  def self.stat; end

  sig do
    params(
        events: Symbol,
        blk: T.proc.params(tp: TracePoint).void,
    )
    .returns(TracePoint)
  end
  def self.trace(*events, &blk); end

  sig {returns(T.untyped)}
  def binding; end

  sig {returns(T.untyped)}
  def callee_id; end

  sig {returns(Module)}
  def defined_class; end

  sig {returns(T::Boolean)}
  sig {params(blk: T.proc.void).void}
  def disable(&blk); end

  sig {returns(T::Boolean)}
  sig {params(blk: T.proc.void).void}
  def enable(&blk); end

  sig {returns(T::Boolean)}
  def enabled?; end

  sig {returns(String)}
  def inspect; end

  sig {returns(Integer)}
  def lineno; end

  sig {returns(T.untyped)}
  def method_id; end

  sig {returns(String)}
  def path; end

  sig {returns(T.untyped)}
  def raised_exception; end

  sig {returns(T.untyped)}
  def return_value; end

  sig {returns(T.untyped)}
  def self; end
end

