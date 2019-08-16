# typed: __STDLIB_INTERNAL

# Descendants of class [Exception](Exception) are used
# to communicate between
# [Kernel\#raise](https://ruby-doc.org/core-2.6.3/Kernel.html#method-i-raise)
# and `rescue` statements in `begin ... end` blocks.
# [Exception](Exception) objects carry information
# about the exception – its type (the exception’s class name), an optional
# descriptive string, and optional traceback information.
# [Exception](Exception) subclasses may add additional
# information like
# [NameError\#name](https://ruby-doc.org/core-2.6.3/NameError.html#method-i-name)
# .
# 
# Programs may make subclasses of
# [Exception](Exception) , typically of
# [StandardError](https://ruby-doc.org/core-2.6.3/StandardError.html) or
# [RuntimeError](https://ruby-doc.org/core-2.6.3/RuntimeError.html) , to
# provide custom classes and add additional information. See the subclass
# list below for defaults for `raise` and `rescue` .
# 
# When an exception has been raised but not yet handled (in `rescue`,
# `ensure`, `at_exit` and `END` blocks) the global variable `$!` will
# contain the current exception and `$@` contains the current exception’s
# backtrace.
# 
# It is recommended that a library should have one subclass of
# [StandardError](https://ruby-doc.org/core-2.6.3/StandardError.html) or
# [RuntimeError](https://ruby-doc.org/core-2.6.3/RuntimeError.html) and
# have specific exception types inherit from it. This allows the user to
# rescue a generic exception type to catch all exceptions the library may
# raise even if future versions of the library add new exception
# subclasses.
# 
# For example:
# 
# ```ruby
# class MyLibrary
#   class Error < RuntimeError
#   end
# 
#   class WidgetError < Error
#   end
# 
#   class FrobError < Error
#   end
# 
# end
# ```
# 
# To handle both WidgetError and FrobError the library user can rescue
# MyLibrary::Error.
# 
# The built-in subclasses of [Exception](Exception)
# are:
# 
#   - [NoMemoryError](https://ruby-doc.org/core-2.6.3/NoMemoryError.html)
# 
#   - [ScriptError](https://ruby-doc.org/core-2.6.3/ScriptError.html)
#     
#       - [LoadError](https://ruby-doc.org/core-2.6.3/LoadError.html)
#     
#       - [NotImplementedError](https://ruby-doc.org/core-2.6.3/NotImplementedError.html)
#     
#       - [SyntaxError](https://ruby-doc.org/core-2.6.3/SyntaxError.html)
# 
#   - [SecurityError](https://ruby-doc.org/core-2.6.3/SecurityError.html)
# 
#   - [SignalException](https://ruby-doc.org/core-2.6.3/SignalException.html)
#     
#       - [Interrupt](https://ruby-doc.org/core-2.6.3/Interrupt.html)
# 
#   - [StandardError](https://ruby-doc.org/core-2.6.3/StandardError.html)
#     -- default for `rescue`
#     
#       - [ArgumentError](https://ruby-doc.org/core-2.6.3/ArgumentError.html)
#         
#           - [UncaughtThrowError](https://ruby-doc.org/core-2.6.3/UncaughtThrowError.html)
#     
#       - [EncodingError](https://ruby-doc.org/core-2.6.3/EncodingError.html)
#     
#       - [FiberError](https://ruby-doc.org/core-2.6.3/FiberError.html)
#     
#       - [IOError](https://ruby-doc.org/core-2.6.3/IOError.html)
#         
#           - [EOFError](https://ruby-doc.org/core-2.6.3/EOFError.html)
#     
#       - [IndexError](https://ruby-doc.org/core-2.6.3/IndexError.html)
#         
#           - [KeyError](https://ruby-doc.org/core-2.6.3/KeyError.html)
#         
#           - [StopIteration](https://ruby-doc.org/core-2.6.3/StopIteration.html)
#     
#       - [LocalJumpError](https://ruby-doc.org/core-2.6.3/LocalJumpError.html)
#     
#       - [NameError](https://ruby-doc.org/core-2.6.3/NameError.html)
#         
#           - [NoMethodError](https://ruby-doc.org/core-2.6.3/NoMethodError.html)
#     
#       - [RangeError](https://ruby-doc.org/core-2.6.3/RangeError.html)
#         
#           - [FloatDomainError](https://ruby-doc.org/core-2.6.3/FloatDomainError.html)
#     
#       - [RegexpError](https://ruby-doc.org/core-2.6.3/RegexpError.html)
#     
#       - [RuntimeError](https://ruby-doc.org/core-2.6.3/RuntimeError.html)
#         -- default for `raise`
#         
#           - [FrozenError](https://ruby-doc.org/core-2.6.3/FrozenError.html)
#     
#       - [SystemCallError](https://ruby-doc.org/core-2.6.3/SystemCallError.html)
#         
#           - Errno::\*
#     
#       - [ThreadError](https://ruby-doc.org/core-2.6.3/ThreadError.html)
#     
#       - [TypeError](https://ruby-doc.org/core-2.6.3/TypeError.html)
#     
#       - [ZeroDivisionError](https://ruby-doc.org/core-2.6.3/ZeroDivisionError.html)
# 
#   - [SystemExit](https://ruby-doc.org/core-2.6.3/SystemExit.html)
# 
#   - [SystemStackError](https://ruby-doc.org/core-2.6.3/SystemStackError.html)
# 
#   - fatal – impossible to rescue
class Exception < Object
  # Equality—If *obj* is not an `Exception`, returns `false` . Otherwise,
  # returns `true` if *exc* and *obj* share same class, messages, and
  # backtrace.
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Returns any backtrace associated with the exception. The backtrace is an
  # array of strings, each containing either “filename:lineNo: in \`method”‘
  # or “filename:lineNo.”
  # 
  # ```ruby
  # def a
  #   raise "boom"
  # end
  # 
  # def b
  #   a()
  # end
  # 
  # begin
  #   b()
  # rescue => detail
  #   print detail.backtrace.join("\n")
  # end
  # ```
  # 
  # *produces:*
  # 
  #     prog.rb:2:in `a'
  #     prog.rb:6:in `b'
  #     prog.rb:10
  sig {returns(T::Array[String])}
  def backtrace(); end

  # Returns any backtrace associated with the exception. This method is
  # similar to
  # [\#backtrace](Exception.downloaded.ruby_doc#method-i-backtrace) , but
  # the backtrace is an array of
  # [Thread::Backtrace::Location](https://ruby-doc.org/core-2.6.3/Thread/Backtrace/Location.html)
  # .
  # 
  # Now, this method is not affected by
  # [\#set\_backtrace](Exception.downloaded.ruby_doc#method-i-set_backtrace)
  # .
  sig {returns(T::Array[Thread::Backtrace::Location])}
  def backtrace_locations(); end

  # Returns the previous exception ($\!) at the time this exception was
  # raised. This is useful for wrapping exceptions and retaining the
  # original exception information.
  sig {returns(T.nilable(Exception))}
  def cause(); end

  # With no argument, or if the argument is the same as the receiver, return
  # the receiver. Otherwise, create a new exception object of the same class
  # as the receiver, but with a message equal to `string.to_str` .
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

  # Return this exception’s class name and message.
  sig {returns(String)}
  def inspect(); end

  # Returns the result of invoking `exception.to_s` . Normally this returns
  # the exception’s message or name.
  sig {returns(String)}
  def message(); end

  # Sets the backtrace information associated with `exc` . The `backtrace`
  # must be an array of
  # [String](https://ruby-doc.org/core-2.6.3/String.html) objects or a
  # single [String](https://ruby-doc.org/core-2.6.3/String.html) in the
  # format described in
  # [\#backtrace](Exception.downloaded.ruby_doc#method-i-backtrace) .
  sig do
    params(
        arg0: T.any(String, T::Array[String]),
    )
    .returns(T::Array[String])
  end
  def set_backtrace(arg0); end

  # Returns exception’s message (or the name of the exception if no message
  # is set).
  sig {returns(String)}
  def to_s(); end
end
