# typed: true
class Exception < Object
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ==(arg0); end

  sig {returns(T::Array[String])}
  def backtrace(); end

  sig {returns(T::Array[Thread::Backtrace::Location])}
  def backtrace_locations(); end

  sig {returns(NilClass)}
  def cause(); end

  sig do
    params(
        arg0: String,
    )
    .returns(Exception)
  end
  def exception(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil)); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(String)}
  def message(); end

  sig do
    params(
        arg0: T.any(String, T::Array[String]),
    )
    .returns(T::Array[String])
  end
  def set_backtrace(arg0); end

  sig {returns(String)}
  def to_s(); end
end
