# typed: true
module GC
  INTERNAL_CONSTANTS = T.let(T.unsafe(nil), Hash)
  OPTS = T.let(T.unsafe(nil), Array)

  sig {returns(Integer)}
  def self.count(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def self.disable(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def self.enable(); end

  sig do
    params(
        full_mark: T.any(TrueClass, FalseClass),
        immediate_sweep: T.any(TrueClass, FalseClass),
    )
    .returns(NilClass)
  end
  def self.start(full_mark: T.unsafe(nil), immediate_sweep: T.unsafe(nil)); end

  sig {params(arg0: T::Hash[Symbol, Integer]).returns(T::Hash[Symbol, Integer])}
  sig {params(arg0: Symbol).returns(Integer)}
  def self.stat(arg0={}); end

  sig {returns(T.any(Integer, TrueClass, FalseClass))}
  def self.stress(); end
end
