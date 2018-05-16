# typed: true
class ArgumentError < StandardError
end

class ClosedQueueError < StopIteration
end

class EOFError < IOError
end

class EncodingError < StandardError
end

class FiberError < StandardError
end

class FloatDomainError < RangeError
end

class IOError < StandardError
end

class IndexError < StandardError
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

class StandardError < Exception
end

class SyntaxError < ScriptError
end

class SystemCallError < StandardError
end

class SystemStackError < Exception
end

class ThreadError < StandardError
end

class TypeError < StandardError
end

class UncaughtThrowError < ArgumentError
end

class ZeroDivisionError < StandardError
end
