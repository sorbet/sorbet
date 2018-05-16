# typed: true
class Class < Module
  sig.returns(T.untyped)
  def allocate(); end

  sig(
      arg0: Class,
  )
  .returns(T.untyped)
  def inherited(arg0); end

  sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def instance_methods(arg0=_); end

  sig.returns(String)
  def name(); end

  sig.returns(T.nilable(Class))
  sig.returns(Class)
  def superclass(); end
end
