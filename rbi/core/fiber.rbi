# typed: __STDLIB_INTERNAL

# Fibers are primitives for implementing light weight cooperative concurrency in
# Ruby. Basically they are a means of creating code blocks that can be paused
# and resumed, much like threads. The main difference is that they are never
# preempted and that the scheduling must be done by the programmer and not the
# VM.
#
# As opposed to other stackless light weight concurrency models, each fiber
# comes with a stack. This enables the fiber to be paused from deeply nested
# function calls within the fiber block. See the ruby(1) manpage to configure
# the size of the fiber stack(s).
#
# When a fiber is created it will not run automatically. Rather it must be
# explicitly asked to run using the
# [`Fiber#resume`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-resume)
# method. The code running inside the fiber can give up control by calling
# [`Fiber.yield`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-yield)
# in which case it yields control back to caller (the caller of the
# [`Fiber#resume`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-resume)).
#
# Upon yielding or termination the
# [`Fiber`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html) returns the value of
# the last executed expression
#
# For instance:
#
# ```ruby
# fiber = Fiber.new do
#   Fiber.yield 1
#   2
# end
#
# puts fiber.resume
# puts fiber.resume
# puts fiber.resume
# ```
#
# *produces*
#
# ```
# 1
# 2
# FiberError: dead fiber called
# ```
#
# The
# [`Fiber#resume`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-resume)
# method accepts an arbitrary number of parameters, if it is the first call to
# [`resume`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-resume)
# then they will be passed as block arguments. Otherwise they will be the return
# value of the call to
# [`Fiber.yield`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-yield)
#
# Example:
#
# ```ruby
# fiber = Fiber.new do |first|
#   second = Fiber.yield first + 2
# end
#
# puts fiber.resume 10
# puts fiber.resume 1_000_000
# puts fiber.resume "The fiber will be dead before I can cause trouble"
# ```
#
# *produces*
#
# ```
# 12
# 1000000
# FiberError: dead fiber called
# ```
#
# ## Non-blocking Fibers
#
# The concept of *non-blocking fiber* was introduced in Ruby 3.0. A non-blocking
# fiber, when reaching a operation that would normally block the fiber (like
# `sleep`, or wait for another process or I/O) will yield control to other
# fibers and allow the *scheduler* to handle blocking and waking up (resuming)
# this fiber when it can proceed.
#
# For a [`Fiber`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html) to behave as
# non-blocking, it need to be created in
# [`Fiber.new`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-new)
# with `blocking: false` (which is the default), and
# [`Fiber.scheduler`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-scheduler)
# should be set with
# [`Fiber.set_scheduler`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-set_scheduler).
# If
# [`Fiber.scheduler`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-scheduler)
# is not set in the current thread, blocking and non-blocking fibers' behavior
# is identical.
#
# Ruby doesn't provide a scheduler class: it is expected to be implemented by
# the user and correspond to
# [`Fiber::SchedulerInterface`](https://docs.ruby-lang.org/en/2.7.0/Fiber/SchedulerInterface.html).
#
# There is also
# [`Fiber.schedule`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-schedule)
# method, which is expected to immediately perform the given block in a
# non-blocking manner. Its actual implementation is up to the scheduler.
class Fiber < Object
  sig {returns(Fiber)}
  def current; end

  # Returns true if the fiber can still be resumed (or transferred to). After
  # finishing execution of the fiber block this method will always return
  # `false`.
  sig {returns(T::Boolean)}
  def alive?; end

  # Alias for:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-to_s)
  def inspect; end

  # Raises an exception in the fiber at the point at which the last
  # `Fiber.yield` was called. If the fiber has not been started or has already
  # run to completion, raises `FiberError`. If the fiber is yielding, it is
  # resumed. If it is transferring, it is transferred into. But if it is
  # resuming, raises `FiberError`.
  #
  # With no arguments, raises a `RuntimeError`. With a single `String` argument,
  # raises a `RuntimeError` with the string as a message. Otherwise, the first
  # parameter should be the name of an `Exception` class (or an object that
  # returns an `Exception` object when sent an `exception` message). The
  # optional second parameter sets the message associated with the exception,
  # and the third parameter is an array of callback information. Exceptions are
  # caught by the `rescue` clause of `begin...end` blocks.
  def raise(message_or_exception, message = nil, callbacks = []); end

  # Resumes the fiber from the point at which the last
  # [`Fiber.yield`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-yield)
  # was called, or starts running it if it is the first call to
  # [`resume`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-resume).
  # Arguments passed to resume will be the value of the
  # [`Fiber.yield`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-yield)
  # expression or will be passed as block parameters to the fiber's block if
  # this is the first
  # [`resume`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-resume).
  #
  # Alternatively, when resume is called it evaluates to the arguments passed to
  # the next
  # [`Fiber.yield`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-yield)
  # statement inside the fiber's block or to the block value if it runs to
  # completion without any
  # [`Fiber.yield`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-yield)
  def resume(*_); end

  # Also aliased as:
  # [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-inspect)
  def to_s; end

  # Yields control back to the context that resumed the fiber, passing along any
  # arguments that were passed to it. The fiber will resume processing at this
  # point when
  # [`resume`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-resume)
  # is called next. Any arguments passed to the next
  # [`resume`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-i-resume)
  # will be the value that this
  # [`Fiber.yield`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html#method-c-yield)
  # expression evaluates to.
  def self.yield(*_); end

  def self.[]=(key, value); end

  def self.[](key); end
end
