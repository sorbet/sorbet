# typed: true
class Class < Module
  Sorbet.sig.returns(T.untyped)
  def allocate(); end

  Sorbet.sig(args: T.untyped).returns(T.untyped)
  def new(*args); end

  Sorbet.sig(
      arg0: Class,
  )
  .returns(T.untyped)
  def inherited(arg0); end

  Sorbet.sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def instance_methods(arg0=_); end

  Sorbet.sig.returns(String)
  def name(); end

  Sorbet.sig.returns(T.nilable(Class))
  Sorbet.sig.returns(Class)
  def superclass(); end

  Sorbet.sig.void
  Sorbet.sig(
      superclass: Class,
  )
  .void
  Sorbet.sig(
      blk: T.proc(arg0: Class).returns(BasicObject),
  )
  .void
  Sorbet.sig(
      superclass: Class,
      blk: T.proc(arg0: Class).returns(BasicObject),
  )
  .void
  def initialize(superclass=_, &blk); end
end
