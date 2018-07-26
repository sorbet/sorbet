# typed: true
class Exception < Object
  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig.returns(T::Array[String])
  def backtrace(); end

  sig.returns(T::Array[Thread::Backtrace::Location])
  def backtrace_locations(); end

  sig.returns(NilClass)
  def cause(); end

  sig(
      arg0: String,
  )
  .returns(Exception)
  def exception(arg0=_); end

  sig(
      arg0: String,
  )
  .void
  def initialize(arg0=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(String)
  def message(); end

  sig(
      arg0: T.any(String, T::Array[String]),
  )
  .returns(T::Array[String])
  def set_backtrace(arg0); end

  sig.returns(String)
  def to_s(); end
end
