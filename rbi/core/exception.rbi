# typed: true
class Exception < Object
  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  Sorbet.sig.returns(T::Array[String])
  def backtrace(); end

  Sorbet.sig.returns(T::Array[Thread::Backtrace::Location])
  def backtrace_locations(); end

  Sorbet.sig.returns(NilClass)
  def cause(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(Exception)
  def exception(arg0=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: String,
  )
  .void
  def initialize(arg0=T.unsafe(nil)); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(String)
  def message(); end

  Sorbet.sig(
      arg0: T.any(String, T::Array[String]),
  )
  .returns(T::Array[String])
  def set_backtrace(arg0); end

  Sorbet.sig.returns(String)
  def to_s(); end
end
