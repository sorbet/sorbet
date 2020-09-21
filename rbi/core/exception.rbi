# typed: __STDLIB_INTERNAL

# Descendants of class
# [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) are used to
# communicate between
# [`Kernel#raise`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-raise)
# and `rescue` statements in `begin ... end` blocks.
# [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) objects
# carry information about the exception -- its type (the exception's class
# name), an optional descriptive string, and optional traceback information.
# [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) subclasses
# may add additional information like
# [`NameError#name`](https://docs.ruby-lang.org/en/2.6.0/NameError.html#method-i-name).
#
# Programs may make subclasses of
# [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html), typically
# of [`StandardError`](https://docs.ruby-lang.org/en/2.6.0/StandardError.html)
# or [`RuntimeError`](https://docs.ruby-lang.org/en/2.6.0/RuntimeError.html), to
# provide custom classes and add additional information. See the subclass list
# below for defaults for `raise` and `rescue`.
#
# When an exception has been raised but not yet handled (in `rescue`, `ensure`,
# `at_exit` and `END` blocks) the global variable `$!` will contain the current
# exception and `$@` contains the current exception's backtrace.
#
# It is recommended that a library should have one subclass of
# [`StandardError`](https://docs.ruby-lang.org/en/2.6.0/StandardError.html) or
# [`RuntimeError`](https://docs.ruby-lang.org/en/2.6.0/RuntimeError.html) and
# have specific exception types inherit from it. This allows the user to rescue
# a generic exception type to catch all exceptions the library may raise even if
# future versions of the library add new exception subclasses.
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
# The built-in subclasses of
# [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) are:
#
# *   [`NoMemoryError`](https://docs.ruby-lang.org/en/2.6.0/NoMemoryError.html)
# *   [`ScriptError`](https://docs.ruby-lang.org/en/2.6.0/ScriptError.html)
#     *   [`LoadError`](https://docs.ruby-lang.org/en/2.6.0/LoadError.html)
#     *   [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
#     *   [`SyntaxError`](https://docs.ruby-lang.org/en/2.6.0/SyntaxError.html)
#
# *   [`SecurityError`](https://docs.ruby-lang.org/en/2.6.0/SecurityError.html)
# *   [`SignalException`](https://docs.ruby-lang.org/en/2.6.0/SignalException.html)
#     *   [`Interrupt`](https://docs.ruby-lang.org/en/2.6.0/Interrupt.html)
#
# *   [`StandardError`](https://docs.ruby-lang.org/en/2.6.0/StandardError.html)
#     -- default for `rescue`
#     *   [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html)
#         *   [`UncaughtThrowError`](https://docs.ruby-lang.org/en/2.6.0/UncaughtThrowError.html)
#
#     *   [`EncodingError`](https://docs.ruby-lang.org/en/2.6.0/EncodingError.html)
#     *   [`FiberError`](https://docs.ruby-lang.org/en/2.6.0/FiberError.html)
#     *   [`IOError`](https://docs.ruby-lang.org/en/2.6.0/IOError.html)
#         *   [`EOFError`](https://docs.ruby-lang.org/en/2.6.0/EOFError.html)
#
#     *   [`IndexError`](https://docs.ruby-lang.org/en/2.6.0/IndexError.html)
#         *   [`KeyError`](https://docs.ruby-lang.org/en/2.6.0/KeyError.html)
#         *   [`StopIteration`](https://docs.ruby-lang.org/en/2.6.0/StopIteration.html)
#
#     *   [`LocalJumpError`](https://docs.ruby-lang.org/en/2.6.0/LocalJumpError.html)
#     *   [`NameError`](https://docs.ruby-lang.org/en/2.6.0/NameError.html)
#         *   [`NoMethodError`](https://docs.ruby-lang.org/en/2.6.0/NoMethodError.html)
#
#     *   [`RangeError`](https://docs.ruby-lang.org/en/2.6.0/RangeError.html)
#         *   [`FloatDomainError`](https://docs.ruby-lang.org/en/2.6.0/FloatDomainError.html)
#
#     *   [`RegexpError`](https://docs.ruby-lang.org/en/2.6.0/RegexpError.html)
#     *   [`RuntimeError`](https://docs.ruby-lang.org/en/2.6.0/RuntimeError.html)
#         -- default for `raise`
#         *   [`FrozenError`](https://docs.ruby-lang.org/en/2.6.0/FrozenError.html)
#
#     *   [`SystemCallError`](https://docs.ruby-lang.org/en/2.6.0/SystemCallError.html)
#         *   Errno::\*
#
#     *   [`ThreadError`](https://docs.ruby-lang.org/en/2.6.0/ThreadError.html)
#     *   [`TypeError`](https://docs.ruby-lang.org/en/2.6.0/TypeError.html)
#     *   [`ZeroDivisionError`](https://docs.ruby-lang.org/en/2.6.0/ZeroDivisionError.html)
#
# *   [`SystemExit`](https://docs.ruby-lang.org/en/2.6.0/SystemExit.html)
# *   [`SystemStackError`](https://docs.ruby-lang.org/en/2.6.0/SystemStackError.html)
# *   fatal -- impossible to rescue
class Exception < Object
  # Equality---If *obj* is not an `Exception`, returns `false`. Otherwise,
  # returns `true` if *exc* and *obj* share same class, messages, and backtrace.
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Returns any backtrace associated with the exception. The backtrace is an
  # array of strings, each containing either "filename:lineNo: in 'method'" or
  # "filename:lineNo."
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
  # ```
  # prog.rb:2:in `a'
  # prog.rb:6:in `b'
  # prog.rb:10
  # ```
  sig {returns(T.nilable(T::Array[String]))}
  def backtrace(); end

  # Returns any backtrace associated with the exception. This method is similar
  # to
  # [`Exception#backtrace`](https://docs.ruby-lang.org/en/2.6.0/Exception.html#method-i-backtrace),
  # but the backtrace is an array of
  # [`Thread::Backtrace::Location`](https://docs.ruby-lang.org/en/2.6.0/Thread/Backtrace/Location.html).
  #
  # Now, this method is not affected by
  # [`Exception#set_backtrace()`](https://docs.ruby-lang.org/en/2.6.0/Exception.html#method-i-set_backtrace).
  sig {returns(T.nilable(T::Array[Thread::Backtrace::Location]))}
  def backtrace_locations(); end

  # Returns the previous exception ($!) at the time this exception was raised.
  # This is useful for wrapping exceptions and retaining the original exception
  # information.
  sig {returns(T.nilable(Exception))}
  def cause(); end

  # With no argument, or if the argument is the same as the receiver, return the
  # receiver. Otherwise, create a new exception object of the same class as the
  # receiver, but with a message equal to `string.to_str`.
  sig do
    params(
        arg0: String,
    )
    .returns(Exception)
  end
  def exception(arg0=T.unsafe(nil)); end

  # Returns formatted string of *exception*. The returned string is formatted
  # using the same format that Ruby uses when printing an uncaught exceptions to
  # stderr.
  #
  # If *highlight* is `true` the default error handler will send the messages to
  # a tty.
  #
  # *order* must be either of `:top` or `:bottom`, and places the error message
  # and the innermost backtrace come at the top or the bottom.
  #
  # The default values of these options depend on `$stderr` and its `tty?` at
  # the timing of a call.
  def full_message(*_); end

  sig do
    params(
        arg0: T.any(String, Symbol, NilClass, Exception),
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil)); end

  # Return this exception's class name and message.
  sig {returns(String)}
  def inspect(); end

  # Returns the result of invoking `exception.to_s`. Normally this returns the
  # exception's message or name.
  sig {returns(String)}
  def message(); end

  # Sets the backtrace information associated with `exc`. The `backtrace` must
  # be an array of [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # objects or a single
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) in the format
  # described in
  # [`Exception#backtrace`](https://docs.ruby-lang.org/en/2.6.0/Exception.html#method-i-backtrace).
  sig do
    params(
        arg0: T.nilable(T.any(String, T::Array[String])),
    )
    .returns(T::Array[String])
  end
  def set_backtrace(arg0); end

  # Returns exception's message (or the name of the exception if no message is
  # set).
  sig {returns(String)}
  def to_s(); end

  # With no argument, or if the argument is the same as the receiver, return the
  # receiver. Otherwise, create a new exception object of the same class as the
  # receiver, but with a message equal to `string.to_str`.
  def self.exception(*_); end

  # Returns `true` if exception messages will be sent to a tty.
  def self.to_tty?; end
end
