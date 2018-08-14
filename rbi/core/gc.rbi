# typed: true
module GC
  INTERNAL_CONSTANTS = T.let(T.unsafe(nil), Hash)
  OPTS = T.let(T.unsafe(nil), Array)

  Sorbet.sig.returns(Integer)
  def self.count(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def self.disable(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def self.enable(); end

  Sorbet.sig(
      full_mark: T.any(TrueClass, FalseClass),
      immediate_sweep: T.any(TrueClass, FalseClass),
  )
  .returns(NilClass)
  def self.start(full_mark: _, immediate_sweep: _); end

  Sorbet.sig(
      arg0: T.any(Hash, Symbol),
  )
  .returns(T::Hash[Symbol, Integer])
  def self.stat(arg0=_); end

  Sorbet.sig.returns(T.any(Integer, TrueClass, FalseClass))
  def self.stress(); end
end
