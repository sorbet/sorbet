# typed: __STDLIB_INTERNAL

class ArgumentError < StandardError
end

class ClosedQueueError < StopIteration
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

class ThreadError < StandardError
end

class TypeError < StandardError
end
