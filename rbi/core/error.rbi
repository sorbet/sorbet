# typed: __STDLIB_INTERNAL
class Exception < Object
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  sig {returns(T::Array[String])}
  def backtrace(); end

  sig {returns(T::Array[Thread::Backtrace::Location])}
  def backtrace_locations(); end

  sig {returns(T.nilable(Exception))}
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

class ArgumentError < StandardError
end

class EncodingError < StandardError
end

class IndexError < StandardError
end

class Interrupt < SignalException
end

class KeyError < IndexError
end

class LoadError < ScriptError
end

class NameError < StandardError
end

class NoMemoryError < Exception
end

class NoMethodError < NameError
end

class NotImplementedError < ScriptError
end

class RangeError < StandardError
end

class RuntimeError < StandardError
end

class ScriptError < Exception
end

class SecurityError < Exception
end

class SignalException < Exception
end

class StandardError < Exception
end

class SyntaxError < ScriptError
end

class SystemCallError < StandardError
end

class SystemExit < Exception
end

class TypeError < StandardError
end

module Warning
end
