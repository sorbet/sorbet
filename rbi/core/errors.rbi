# typed: __STDLIB_INTERNAL

# Raised when the arguments are wrong and there isn't a more specific
# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) class.
#
# Ex: passing the wrong number of arguments
#
# ```ruby
# [1, 2, 3].first(4, 5)
# ```
#
# *raises the exception:*
#
# ```
# ArgumentError: wrong number of arguments (given 2, expected 1)
# ```
#
# Ex: passing an argument that is not acceptable:
#
# ```ruby
# [1, 2, 3].first(-4)
# ```
#
# *raises the exception:*
#
# ```
# ArgumentError: negative array size
# ```
class ArgumentError < StandardError
end

# The exception class which will be raised when pushing into a closed Queue. See
# [`Thread::Queue#close`](https://docs.ruby-lang.org/en/2.7.0/Thread/Queue.html#method-i-close)
# and
# [`Thread::SizedQueue#close`](https://docs.ruby-lang.org/en/2.7.0/Thread/SizedQueue.html#method-i-close).
class ClosedQueueError < StopIteration
end

# [`EncodingError`](https://docs.ruby-lang.org/en/2.7.0/EncodingError.html) is
# the base class for encoding errors.
class EncodingError < StandardError
end

# Raised by some [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) operations
# when reaching the end of file. Many
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) methods exist in two
# forms,
#
# one that returns `nil` when the end of file is reached, the other raises
# `EOFError`.
#
# `EOFError` is a subclass of `IOError`.
#
# ```ruby
# file = File.open("/etc/hosts")
# file.read
# file.gets     #=> nil
# file.readline #=> EOFError: end of file reached
# ```
class EOFError < IOError
end

# Raised when attempting to convert special float values (in particular
# `Infinity` or `NaN`) to numerical classes which don't support them.
#
# ```ruby
# Float::INFINITY.to_r   #=> FloatDomainError: Infinity
# ```
class FloatDomainError < RangeError
end

# Raised when there is an attempt to modify a frozen object.
#
# ```ruby
# [1, 2, 3].freeze << 4
# ```
#
# *raises the exception:*
#
# ```
# FrozenError: can't modify frozen Array
# ```
class FrozenError < RuntimeError
  # Construct a new
  # [`FrozenError`](https://docs.ruby-lang.org/en/2.7.0/FrozenError.html)
  # exception. If given the *receiver* parameter may subsequently be examined
  # using the
  # [`FrozenError#receiver`](https://docs.ruby-lang.org/en/2.7.0/FrozenError.html#method-i-receiver)
  # method.
  #
  # ```ruby
  # a = [].freeze
  # raise FrozenError.new("can't modify frozen array", receiver: a)
  # ```
  sig { params(msg: T.untyped, receiver: T.untyped).void }
  def initialize(msg = nil, receiver: nil); end

  # Return the receiver associated with this
  # [`FrozenError`](https://docs.ruby-lang.org/en/2.7.0/FrozenError.html)
  # exception.
  sig { returns(T.untyped) }
  def receiver; end
end

# Raised when the given index is invalid.
#
# ```ruby
# a = [:foo, :bar]
# a.fetch(0)   #=> :foo
# a[4]         #=> nil
# a.fetch(4)   #=> IndexError: index 4 outside of array bounds: -2...2
# ```
class IndexError < StandardError
end

# Raised when the interrupt signal is received, typically because the user has
# pressed Control-C (on most posix platforms). As such, it is a subclass of
# `SignalException`.
#
# ```ruby
# begin
#   puts "Press ctrl-C when you get bored"
#   loop {}
# rescue Interrupt => e
#   puts "Note: You will typically use Signal.trap instead."
# end
# ```
#
# *produces:*
#
# ```
# Press ctrl-C when you get bored
# ```
#
# *then waits until it is interrupted with Control-C and then prints:*
#
# ```
# Note: You will typically use Signal.trap instead.
# ```
class Interrupt < SignalException
end

# Raised when an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) operation
# fails.
#
# ```ruby
# File.open("/etc/hosts") {|f| f << "example"}
#   #=> IOError: not opened for writing
#
# File.open("/etc/hosts") {|f| f.close; f.read }
#   #=> IOError: closed stream
# ```
#
# Note that some [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) failures
# raise `SystemCallError`s and these are not subclasses of IOError:
#
# ```ruby
# File.open("does/not/exist")
#   #=> Errno::ENOENT: No such file or directory - does/not/exist
# ```
class IOError < StandardError
end

# Raised when the specified key is not found. It is a subclass of
# [`IndexError`](https://docs.ruby-lang.org/en/2.7.0/IndexError.html).
#
# ```ruby
# h = {"foo" => :bar}
# h.fetch("foo") #=> :bar
# h.fetch("baz") #=> KeyError: key not found: "baz"
# ```
class KeyError < IndexError
  # Construct a new KeyError exception with the given message, receiver and key.
  sig { params(msg: T.untyped, receiver: T.untyped, key: T.untyped).void }
  def initialize(msg = nil, receiver: nil, key: nil); end

  # Return the key caused this
  # [`KeyError`](https://docs.ruby-lang.org/en/2.7.0/KeyError.html) exception.
  def key; end

  # Return the receiver associated with this
  # [`KeyError`](https://docs.ruby-lang.org/en/2.7.0/KeyError.html) exception.
  def receiver; end
end

# Raised when a file required (a Ruby script, extension library, ...) fails to
# load.
#
# ```ruby
# require 'this/file/does/not/exist'
# ```
#
# *raises the exception:*
#
# ```
# LoadError: no such file to load -- this/file/does/not/exist
# ```
class LoadError < ScriptError
  # the path failed to load
  def path; end
end

# Raised when Ruby can't yield as requested.
#
# A typical scenario is attempting to yield when no block is given:
#
# ```ruby
# def call_block
#   yield 42
# end
# call_block
# ```
#
# *raises the exception:*
#
# ```
# LocalJumpError: no block given (yield)
# ```
#
# A more subtle example:
#
# ```ruby
# def get_me_a_return
#   Proc.new { return 42 }
# end
# get_me_a_return.call
# ```
#
# *raises the exception:*
#
# ```
# LocalJumpError: unexpected return
# ```
class LocalJumpError < StandardError
  # Returns the exit value associated with this `LocalJumpError`.
  def exit_value; end

  # The reason this block was terminated: :break, :redo, :retry, :next, :return,
  # or :noreason.
  def reason; end
end

# Raised when a given name is invalid or undefined.
#
# ```ruby
# puts foo
# ```
#
# *raises the exception:*
#
# ```
# NameError: undefined local variable or method `foo' for main:Object
# ```
#
# Since constant names must start with a capital:
#
# ```ruby
# Integer.const_set :answer, 42
# ```
#
# *raises the exception:*
#
# ```
# NameError: wrong constant name answer
# ```
class NameError < StandardError
  # Construct a new
  # [`NameError`](https://docs.ruby-lang.org/en/2.7.0/NameError.html) exception.
  # If given the *name* parameter may subsequently be examined using the
  # [`NameError#name`](https://docs.ruby-lang.org/en/2.7.0/NameError.html#method-i-name)
  # method. *receiver* parameter allows to pass object in context of which the
  # error happened. Example:
  #
  # ```ruby
  # [1, 2, 3].method(:rject) # NameError with name "rject" and receiver: Array
  # [1, 2, 3].singleton_method(:rject) # NameError with name "rject" and receiver: [1, 2, 3]
  # ```
  def self.new(*_); end

  # Return a list of the local variable names defined where this
  # [`NameError`](https://docs.ruby-lang.org/en/2.7.0/NameError.html) exception
  # was raised.
  #
  # Internal use only.
  def local_variables; end

  # Return the name associated with this
  # [`NameError`](https://docs.ruby-lang.org/en/2.7.0/NameError.html) exception.
  def name; end

  # Return the receiver associated with this
  # [`NameError`](https://docs.ruby-lang.org/en/2.7.0/NameError.html) exception.
  def receiver; end
end

# Raised when memory allocation fails.
class NoMemoryError < Exception
end

# Raised when a method is called on a receiver which doesn't have it defined and
# also fails to respond with `method_missing`.
#
# ```ruby
# "hello".to_ary
# ```
#
# *raises the exception:*
#
# ```
# NoMethodError: undefined method `to_ary' for "hello":String
# ```
class NoMethodError < NameError
  # Construct a
  # [`NoMethodError`](https://docs.ruby-lang.org/en/2.7.0/NoMethodError.html)
  # exception for a method of the given name called with the given arguments.
  # The name may be accessed using the `#name` method on the resulting object,
  # and the arguments using the `#args` method.
  #
  # If *private* argument were passed, it designates method was attempted to
  # call in private context, and can be accessed with `#private_call?` method.
  #
  # *receiver* argument stores an object whose method was called.
  def self.new(*_); end

  # Return the arguments passed in as the third parameter to the constructor.
  def args; end

  # Return true if the caused method was called as private.
  def private_call?; end
end

# Raised when a feature is not implemented on the current platform. For example,
# methods depending on the `fsync` or `fork` system calls may raise this
# exception if the underlying operating system or Ruby runtime does not support
# them.
#
# Note that if `fork` raises a `NotImplementedError`, then `respond_to?(:fork)`
# returns `false`.
class NotImplementedError < ScriptError
end

class NoMatchingPatternError < StandardError
end

# Raised when a given numerical value is out of range.
#
# ```ruby
# [1, 2, 3].drop(1 << 100)
# ```
#
# *raises the exception:*
#
# ```
# RangeError: bignum too big to convert into `long'
# ```
class RangeError < StandardError
end

# Raised when given an invalid regexp expression.
#
# ```ruby
# Regexp.new("?")
# ```
#
# *raises the exception:*
#
# ```
# RegexpError: target of repeat operator is not specified: /?/
# ```
class RegexpError < StandardError
end

# A generic error class raised when an invalid operation is attempted.
# [`Kernel#raise`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-raise)
# will raise a
# [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html) if no
# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) class is
# specified.
#
# ```ruby
# raise "ouch"
# ```
#
# *raises the exception:*
#
# ```
# RuntimeError: ouch
# ```
class RuntimeError < StandardError
end

# [`ScriptError`](https://docs.ruby-lang.org/en/2.7.0/ScriptError.html) is the
# superclass for errors raised when a script can not be executed because of a
# `LoadError`, `NotImplementedError` or a `SyntaxError`. Note these type of
# `ScriptErrors` are not `StandardError` and will not be rescued unless it is
# specified explicitly (or its ancestor `Exception`).
class ScriptError < Exception
end

# No longer used by internal code.
class SecurityError < Exception
end

# Raised when a signal is received.
#
# ```ruby
# begin
#   Process.kill('HUP',Process.pid)
#   sleep # wait for receiver to handle signal sent by Process.kill
# rescue SignalException => e
#   puts "received Exception #{e}"
# end
# ```
#
# *produces:*
#
# ```ruby
# received Exception SIGHUP
# ```
class SignalException < Exception
  # Construct a new
  # [`SignalException`](https://docs.ruby-lang.org/en/2.7.0/SignalException.html)
  # object. `sig_name` should be a known signal name.
  def self.new(*_); end

  def signm; end

  # Returns a signal number.
  def signo; end
end

# The most standard error types are subclasses of
# [`StandardError`](https://docs.ruby-lang.org/en/2.7.0/StandardError.html). A
# rescue clause without an explicit
# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) class will
# rescue all StandardErrors (and only those).
#
# ```ruby
# def foo
#   raise "Oups"
# end
# foo rescue "Hello"   #=> "Hello"
# ```
#
# On the other hand:
#
# ```ruby
# require 'does/not/exist' rescue "Hi"
# ```
#
# *raises the exception:*
#
# ```
# LoadError: no such file to load -- does/not/exist
# ```
class StandardError < Exception
end

# Raised to stop the iteration, in particular by
# [`Enumerator#next`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html#method-i-next).
# It is rescued by
# [`Kernel#loop`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-loop).
#
# ```ruby
# loop do
#   puts "Hello"
#   raise StopIteration
#   puts "World"
# end
# puts "Done!"
# ```
#
# *produces:*
#
# ```ruby
# Hello
# Done!
# ```
class StopIteration < IndexError
  # Returns the return value of the iterator.
  #
  # ```ruby
  # o = Object.new
  # def o.each
  #   yield 1
  #   yield 2
  #   yield 3
  #   100
  # end
  #
  # e = o.to_enum
  #
  # puts e.next                   #=> 1
  # puts e.next                   #=> 2
  # puts e.next                   #=> 3
  #
  # begin
  #   e.next
  # rescue StopIteration => ex
  #   puts ex.result              #=> 100
  # end
  # ```
  def result; end
end

# Raised when encountering Ruby code with an invalid syntax.
#
# ```ruby
# eval("1+1=2")
# ```
#
# *raises the exception:*
#
# ```
# SyntaxError: (eval):1: syntax error, unexpected '=', expecting $end
# ```
class SyntaxError < ScriptError
end

# [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
# is the base class for all low-level platform-dependent errors.
#
# The errors available on the current platform are subclasses of
# [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
# and are defined in the
# [`Errno`](https://docs.ruby-lang.org/en/2.7.0/Errno.html) module.
#
# ```ruby
# File.open("does/not/exist")
# ```
#
# *raises the exception:*
#
# ```
# Errno::ENOENT: No such file or directory - does/not/exist
# ```
class SystemCallError < StandardError
  # If *errno* corresponds to a known system error code, constructs the
  # appropriate [`Errno`](https://docs.ruby-lang.org/en/2.7.0/Errno.html) class
  # for that error, otherwise constructs a generic
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # object. The error number is subsequently available via the
  # [`errno`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html#method-i-errno)
  # method.
  def self.new(*_); end

  # Return this SystemCallError's error number.
  def errno; end

  # Return `true` if the receiver is a generic `SystemCallError`, or if the
  # error numbers `self` and *other* are the same.
  def self.===(_); end
end

# Raised by `exit` to initiate the termination of the script.
class SystemExit < Exception
  # Create a new `SystemExit` exception with the given status and message.
  # Status is true, false, or an integer. If status is not given, true is used.
  def self.new(*_); end

  # Return the status value associated with this system exit.
  def status; end

  # Returns `true` if exiting successful, `false` if not.
  def success?; end
end

# Raised in case of a stack overflow.
#
# ```ruby
# def me_myself_and_i
#   me_myself_and_i
# end
# me_myself_and_i
# ```
#
# *raises the exception:*
#
# ```
# SystemStackError: stack level too deep
# ```
class SystemStackError < Exception
end

# Raised when an invalid operation is attempted on a thread.
#
# For example, when no other thread has been started:
#
# ```ruby
# Thread.stop
# ```
#
# This will raises the following exception:
#
# ```
# ThreadError: stopping only thread
# note: use sleep to stop forever
# ```
class ThreadError < StandardError
end

# Raised when encountering an object that is not of the expected type.
#
# ```ruby
# [1, 2, 3].first("two")
# ```
#
# *raises the exception:*
#
# ```
# TypeError: no implicit conversion of String into Integer
# ```
class TypeError < StandardError
end

# Raised when `throw` is called with a *tag* which does not have corresponding
# `catch` block.
#
# ```ruby
# throw "foo", "bar"
# ```
#
# *raises the exception:*
#
# ```
# UncaughtThrowError: uncaught throw "foo"
# ```
class UncaughtThrowError < ArgumentError
  # Document-class:
  # [`UncaughtThrowError`](https://docs.ruby-lang.org/en/2.7.0/UncaughtThrowError.html)
  #
  # Raised when `throw` is called with a *tag* which does not have corresponding
  # `catch` block.
  #
  # ```ruby
  # throw "foo", "bar"
  # ```
  #
  # *raises the exception:*
  #
  # ```
  # UncaughtThrowError: uncaught throw "foo"
  # ```
  def self.new(*_); end

  # Return the tag object which was called for.
  def tag; end

  # Returns formatted message with the inspected tag.
  def to_s; end

  # Return the return value which was called for.
  def value; end
end

# Raised when attempting to divide an integer by 0.
#
# ```ruby
# 42 / 0   #=> ZeroDivisionError: divided by 0
# ```
#
# Note that only division by an exact 0 will raise the exception:
#
# ```ruby
# 42 /  0.0   #=> Float::INFINITY
# 42 / -0.0   #=> -Float::INFINITY
# 0  /  0.0   #=> NaN
# ```
class ZeroDivisionError < StandardError
end
