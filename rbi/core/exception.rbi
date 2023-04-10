# typed: __STDLIB_INTERNAL

# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html)
# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) and its
# subclasses are used to communicate between
# [`Kernel#raise`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-raise)
# and `rescue` statements in `begin ... end` blocks.
#
# An [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) object
# carries information about an exception:
# *   Its type (the exception's class).
# *   An optional descriptive message.
# *   Optional backtrace information.
#
#
# Some built-in subclasses of
# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) have
# additional methods: e.g.,
# [`NameError#name`](https://docs.ruby-lang.org/en/2.7.0/NameError.html#method-i-name).
#
# ## Defaults
#
# Two Ruby statements have default exception classes:
# *   `raise`: defaults to
#     [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html).
# *   `rescue`: defaults to
#     [`StandardError`](https://docs.ruby-lang.org/en/2.7.0/StandardError.html).
#
#
# ## Global Variables
#
# When an exception has been raised but not yet handled (in `rescue`, `ensure`,
# `at_exit` and `END` blocks), two global variables are set:
# *   `$!` contains the current exception.
# *   `$@` contains its backtrace.
#
#
# ## Custom Exceptions
#
# To provide additional or alternate information, a program may create custom
# exception classes that derive from the built-in exception classes.
#
# A good practice is for a library to create a single "generic" exception class
# (typically a subclass of
# [`StandardError`](https://docs.ruby-lang.org/en/2.7.0/StandardError.html) or
# [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html)) and
# have its other exception classes derive from that class. This allows the user
# to rescue the generic exception, thus catching all exceptions the library may
# raise even if future versions of the library add new exception subclasses.
#
# For example:
#
# ```ruby
# class MyLibrary
#   class Error < ::StandardError
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
# To handle both MyLibrary::WidgetError and MyLibrary::FrobError the library
# user can rescue MyLibrary::Error.
#
# ## Built-In [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) Classes
#
# The built-in subclasses of
# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) are:
#
# *   [`NoMemoryError`](https://docs.ruby-lang.org/en/2.7.0/NoMemoryError.html)
# *   [`ScriptError`](https://docs.ruby-lang.org/en/2.7.0/ScriptError.html)
#     *   [`LoadError`](https://docs.ruby-lang.org/en/2.7.0/LoadError.html)
#     *   [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html)
#     *   [`SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/SyntaxError.html)
#
# *   [`SecurityError`](https://docs.ruby-lang.org/en/2.7.0/SecurityError.html)
# *   [`SignalException`](https://docs.ruby-lang.org/en/2.7.0/SignalException.html)
#     *   [`Interrupt`](https://docs.ruby-lang.org/en/2.7.0/Interrupt.html)
#
# *   [`StandardError`](https://docs.ruby-lang.org/en/2.7.0/StandardError.html)
#     *   [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html)
#         *   [`UncaughtThrowError`](https://docs.ruby-lang.org/en/2.7.0/UncaughtThrowError.html)
#
#     *   [`EncodingError`](https://docs.ruby-lang.org/en/2.7.0/EncodingError.html)
#     *   [`FiberError`](https://docs.ruby-lang.org/en/2.7.0/FiberError.html)
#     *   [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html)
#         *   [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html)
#
#     *   [`IndexError`](https://docs.ruby-lang.org/en/2.7.0/IndexError.html)
#         *   [`KeyError`](https://docs.ruby-lang.org/en/2.7.0/KeyError.html)
#         *   [`StopIteration`](https://docs.ruby-lang.org/en/2.7.0/StopIteration.html)
#             *   [`ClosedQueueError`](https://docs.ruby-lang.org/en/2.7.0/ClosedQueueError.html)
#
#
#     *   [`LocalJumpError`](https://docs.ruby-lang.org/en/2.7.0/LocalJumpError.html)
#     *   [`NameError`](https://docs.ruby-lang.org/en/2.7.0/NameError.html)
#         *   [`NoMethodError`](https://docs.ruby-lang.org/en/2.7.0/NoMethodError.html)
#
#     *   [`RangeError`](https://docs.ruby-lang.org/en/2.7.0/RangeError.html)
#         *   [`FloatDomainError`](https://docs.ruby-lang.org/en/2.7.0/FloatDomainError.html)
#
#     *   [`RegexpError`](https://docs.ruby-lang.org/en/2.7.0/RegexpError.html)
#     *   [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html)
#         *   [`FrozenError`](https://docs.ruby-lang.org/en/2.7.0/FrozenError.html)
#
#     *   [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
#         *   Errno::\*
#
#     *   [`ThreadError`](https://docs.ruby-lang.org/en/2.7.0/ThreadError.html)
#     *   [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html)
#     *   [`ZeroDivisionError`](https://docs.ruby-lang.org/en/2.7.0/ZeroDivisionError.html)
#
# *   [`SystemExit`](https://docs.ruby-lang.org/en/2.7.0/SystemExit.html)
# *   [`SystemStackError`](https://docs.ruby-lang.org/en/2.7.0/SystemStackError.html)
# *   fatal
class Exception < Object
  # Equality---If *obj* is not an
  # [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html), returns
  # `false`. Otherwise, returns `true` if *exc* and *obj* share same class,
  # messages, and backtrace.
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
  #
  # In the case no backtrace has been set, `nil` is returned
  #
  # ```ruby
  # ex = StandardError.new
  # ex.backtrace
  # #=> nil
  # ```
  sig {returns(T.nilable(T::Array[String]))}
  def backtrace(); end

  # Returns any backtrace associated with the exception. This method is similar
  # to
  # [`Exception#backtrace`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-backtrace),
  # but the backtrace is an array of
  # [`Thread::Backtrace::Location`](https://docs.ruby-lang.org/en/2.7.0/Thread/Backtrace/Location.html).
  #
  # This method is not affected by
  # [`Exception#set_backtrace()`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-set_backtrace).
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
        arg0: BasicObject,
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
  # be an array of [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # objects or a single
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) in the format
  # described in
  # [`Exception#backtrace`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-backtrace).
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
