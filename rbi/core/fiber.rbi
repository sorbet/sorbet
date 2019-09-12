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
# explicitly asked to run using the `Fiber#resume` method. The code running
# inside the fiber can give up control by calling `Fiber.yield` in which case it
# yields control back to caller (the caller of the `Fiber#resume`).
#
# Upon yielding or termination the
# [`Fiber`](https://docs.ruby-lang.org/en/2.6.0/Fiber.html) returns the value of
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
# The `Fiber#resume` method accepts an arbitrary number of parameters, if it is
# the first call to `resume` then they will be passed as block arguments.
# Otherwise they will be the return value of the call to `Fiber.yield`
#
# Example:
#
# ```ruby
# fiber = Fiber.new do |first|
#   second = Fiber.yield first + 2
# end
#
# puts fiber.resume 10
# puts fiber.resume 14
# puts fiber.resume 18
# ```
#
# *produces*
#
# ```
# 12
# 14
# FiberError: dead fiber called
# ```
class Fiber < Object
  sig {returns(Fiber)}
  def current; end

  # Returns true if the fiber can still be resumed (or transferred to). After
  # finishing execution of the fiber block this method will always return false.
  # You need to `require 'fiber'` before using this method.
  sig {returns(T::Boolean)}
  def alive?; end
end
