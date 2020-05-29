# typed: __STDLIB_INTERNAL

# This class watches for termination of multiple threads. Basic functionality
# (wait until specified threads have terminated) can be accessed through the
# class method
# [`ThreadsWait::all_waits`](https://docs.ruby-lang.org/en/2.6.0/ThreadsWait.html#method-c-all_waits).
# Finer control can be gained using instance methods.
#
# Example:
#
# ```
# ThreadsWait.all_waits(thr1, thr2, ...) do |t|
#   STDERR.puts "Thread #{t} has terminated."
# end
#
# th = ThreadsWait.new(thread1,...)
# th.next_wait # next one to be done
# ```
ThWait = ThreadsWait

# This class watches for termination of multiple threads. Basic functionality
# (wait until specified threads have terminated) can be accessed through the
# class method
# [`ThreadsWait::all_waits`](https://docs.ruby-lang.org/en/2.6.0/ThreadsWait.html#method-c-all_waits).
# Finer control can be gained using instance methods.
#
# Example:
#
# ```
# ThreadsWait.all_waits(thr1, thr2, ...) do |t|
#   STDERR.puts "Thread #{t} has terminated."
# end
#
# th = ThreadsWait.new(thread1,...)
# th.next_wait # next one to be done
# ```
class ThreadsWait
  extend(::Exception2MessageMapper)

  # Creates a
  # [`ThreadsWait`](https://docs.ruby-lang.org/en/2.6.0/ThreadsWait.html)
  # object, specifying the threads to wait on. Non-blocking.
  def self.new(*threads); end

  def Fail(err = _, *rest); end

  def Raise(err = _, *rest); end

  # Waits until all of the specified threads are terminated. If a block is
  # supplied for the method, it is executed for each thread termination.
  #
  # Raises exceptions in the same manner as `next_wait`.
  def all_waits; end

  # Returns `true` if there are no threads in the pool still running.
  def empty?; end

  # Returns `true` if any thread has terminated and is ready to be collected.
  def finished?; end

  # Waits for specified threads to terminate, and returns when one of the
  # threads terminated.
  def join(*threads); end

  # Specifies the threads that this object will wait for, but does not actually
  # wait.
  def join_nowait(*threads); end

  # Waits until any of the specified threads has terminated, and returns the one
  # that does.
  #
  # If there is no thread to wait, raises `ErrNoWaitingThread`. If `nonblock` is
  # true, and there is no terminated thread, raises `ErrNoFinishedThread`.
  def next_wait(nonblock = _); end

  # Returns the array of threads that have not terminated yet.
  def threads; end

  # Waits until all specified threads have terminated. If a block is provided,
  # it is executed for each thread as they terminate.
  def self.all_waits(*threads); end
end
