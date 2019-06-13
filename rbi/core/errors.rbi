# typed: __STDLIB_INTERNAL

class ArgumentError < StandardError
end

class ClosedQueueError < StopIteration
end

class EncodingError < StandardError
end

class EOFError < IOError
end

class FloatDomainError < RangeError
end

class IndexError < StandardError
end

class Interrupt < SignalException
end

class IOError < StandardError
end

class KeyError < IndexError
end

class LoadError < ScriptError
end

class LocalJumpError < StandardError
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

class RegexpError < StandardError
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

class StopIteration < IndexError
end

class SyntaxError < ScriptError
end

class SystemCallError < StandardError
end

class SystemExit < Exception
end

class SystemStackError < Exception
end

class ThreadError < StandardError
end

class TypeError < StandardError
end

class ZeroDivisionError < StandardError
end
