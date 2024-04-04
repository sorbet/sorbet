# typed: __STDLIB_INTERNAL

# Threads are the Ruby implementation for a concurrent programming model.
#
# Programs that require multiple threads of execution are a perfect candidate
# for Ruby's [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) class.
#
# For example, we can create a new thread separate from the main thread's
# execution using
# [`::new`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-new).
#
# ```ruby
# thr = Thread.new { puts "What's the big deal" }
# ```
#
# Then we are able to pause the execution of the main thread and allow our new
# thread to finish, using
# [`join`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-join):
#
# ```ruby
# thr.join #=> "What's the big deal"
# ```
#
# If we don't call `thr.join` before the main thread terminates, then all other
# threads including `thr` will be killed.
#
# Alternatively, you can use an array for handling multiple threads at once,
# like in the following example:
#
# ```ruby
# threads = []
# threads << Thread.new { puts "What's the big deal" }
# threads << Thread.new { 3.times { puts "Threads are fun!" } }
# ```
#
# After creating a few threads we wait for them all to finish consecutively.
#
# ```ruby
# threads.each { |thr| thr.join }
# ```
#
# To retrieve the last value of a thread, use
# [`value`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-value)
#
# ```ruby
# thr = Thread.new { sleep 1; "Useful value" }
# thr.value #=> "Useful value"
# ```
#
# ### [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) initialization
#
# In order to create new threads, Ruby provides
# [`::new`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-new),
# [`::start`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-start),
# and [`::fork`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-fork).
# A block must be provided with each of these methods, otherwise a
# [`ThreadError`](https://docs.ruby-lang.org/en/2.7.0/ThreadError.html) will be
# raised.
#
# When subclassing the
# [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) class, the
# `initialize` method of your subclass will be ignored by
# [`::start`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-start)
# and [`::fork`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-fork).
# Otherwise, be sure to call super in your `initialize` method.
#
# ### [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) termination
#
# For terminating threads, Ruby provides a variety of ways to do this.
#
# The class method
# [`::kill`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-kill), is
# meant to exit a given thread:
#
# ```ruby
# thr = Thread.new { sleep }
# Thread.kill(thr) # sends exit() to thr
# ```
#
# Alternatively, you can use the instance method
# [`exit`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-exit), or
# any of its aliases
# [`kill`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-kill) or
# [`terminate`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-terminate).
#
# ```ruby
# thr.exit
# ```
#
# ### [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) status
#
# Ruby provides a few instance methods for querying the state of a given thread.
# To get a string with the current thread's state use
# [`status`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-status)
#
# ```ruby
# thr = Thread.new { sleep }
# thr.status # => "sleep"
# thr.exit
# thr.status # => false
# ```
#
# You can also use
# [`alive?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-alive-3F)
# to tell if the thread is running or sleeping, and
# [`stop?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-stop-3F) if
# the thread is dead or sleeping.
#
# ### [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) variables and scope
#
# Since threads are created with blocks, the same rules apply to other Ruby
# blocks for variable scope. Any local variables created within this block are
# accessible to only this thread.
#
# #### Fiber-local vs. Thread-local
#
# Each fiber has its own bucket for
# [`Thread#[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D)
# storage. When you set a new fiber-local it is only accessible within this
# [`Fiber`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html). To illustrate:
#
# ```ruby
# Thread.new {
#   Thread.current[:foo] = "bar"
#   Fiber.new {
#     p Thread.current[:foo] # => nil
#   }.resume
# }.join
# ```
#
# This example uses
# [`[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D) for
# getting and
# [`[]=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D-3D) for
# setting fiber-locals, you can also use
# [`keys`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-keys) to
# list the fiber-locals for a given thread and
# [`key?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-key-3F) to
# check if a fiber-local exists.
#
# When it comes to thread-locals, they are accessible within the entire scope of
# the thread. Given the following example:
#
# ```ruby
# Thread.new{
#   Thread.current.thread_variable_set(:foo, 1)
#   p Thread.current.thread_variable_get(:foo) # => 1
#   Fiber.new{
#     Thread.current.thread_variable_set(:foo, 2)
#     p Thread.current.thread_variable_get(:foo) # => 2
#   }.resume
#   p Thread.current.thread_variable_get(:foo)   # => 2
# }.join
# ```
#
# You can see that the thread-local `:foo` carried over into the fiber and was
# changed to `2` by the end of the thread.
#
# This example makes use of
# [`thread_variable_set`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_set)
# to create new thread-locals, and
# [`thread_variable_get`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_get)
# to reference them.
#
# There is also
# [`thread_variables`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variables)
# to list all thread-locals, and
# [`thread_variable?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable-3F)
# to check if a given thread-local exists.
#
# ### [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) handling
#
# When an unhandled exception is raised inside a thread, it will terminate. By
# default, this exception will not propagate to other threads. The exception is
# stored and when another thread calls
# [`value`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-value) or
# [`join`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-join), the
# exception will be re-raised in that thread.
#
# ```ruby
# t = Thread.new{ raise 'something went wrong' }
# t.value #=> RuntimeError: something went wrong
# ```
#
# An exception can be raised from outside the thread using the
# [`Thread#raise`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-raise)
# instance method, which takes the same parameters as
# [`Kernel#raise`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-raise).
#
# Setting
# [`Thread.abort_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-abort_on_exception)
# = true,
# [`Thread#abort_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-abort_on_exception)
# = true, or $DEBUG = true will cause a subsequent unhandled exception raised in
# a thread to be automatically re-raised in the main thread.
#
# With the addition of the class method
# [`::handle_interrupt`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-handle_interrupt),
# you can now handle exceptions asynchronously with threads.
#
# ### Scheduling
#
# Ruby provides a few ways to support scheduling threads in your program.
#
# The first way is by using the class method
# [`::stop`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-stop), to
# put the current running thread to sleep and schedule the execution of another
# thread.
#
# Once a thread is asleep, you can use the instance method
# [`wakeup`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-wakeup) to
# mark your thread as eligible for scheduling.
#
# You can also try
# [`::pass`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-pass),
# which attempts to pass execution to another thread but is dependent on the OS
# whether a running thread will switch or not. The same goes for
# [`priority`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-priority),
# which lets you hint to the thread scheduler which threads you want to take
# precedence when passing execution. This method is also dependent on the OS and
# may be ignored on some platforms.
class Thread < Object
  # Returns the currently executing thread.
  #
  # ```ruby
  # Thread.current   #=> #<Thread:0x401bdf4c run>
  # ```
  sig {returns(Thread)}
  def self.current; end

  # Returns the main thread.
  sig {returns(Thread)}
  def self.main; end

  # Attribute Reference---Returns the value of a fiber-local variable (current
  # thread's root fiber if not explicitly inside a
  # [`Fiber`](https://docs.ruby-lang.org/en/2.7.0/Fiber.html)), using either a
  # symbol or a string name. If the specified variable does not exist, returns
  # `nil`.
  #
  # ```ruby
  # [
  #   Thread.new { Thread.current["name"] = "A" },
  #   Thread.new { Thread.current[:name]  = "B" },
  #   Thread.new { Thread.current["name"] = "C" }
  # ].each do |th|
  #   th.join
  #   puts "#{th.inspect}: #{th[:name]}"
  # end
  # ```
  #
  # This will produce:
  #
  # ```ruby
  # #<Thread:0x00000002a54220 dead>: A
  # #<Thread:0x00000002a541a8 dead>: B
  # #<Thread:0x00000002a54130 dead>: C
  # ```
  #
  # [`Thread#[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D)
  # and
  # [`Thread#[]=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D-3D)
  # are not thread-local but fiber-local. This confusion did not exist in Ruby
  # 1.8 because fibers are only available since Ruby 1.9. Ruby 1.9 chooses that
  # the methods behaves fiber-local to save following idiom for dynamic scope.
  #
  # ```ruby
  # def meth(newvalue)
  #   begin
  #     oldvalue = Thread.current[:name]
  #     Thread.current[:name] = newvalue
  #     yield
  #   ensure
  #     Thread.current[:name] = oldvalue
  #   end
  # end
  # ```
  #
  # The idiom may not work as dynamic scope if the methods are thread-local and
  # a given block switches fiber.
  #
  # ```ruby
  # f = Fiber.new {
  #   meth(1) {
  #     Fiber.yield
  #   }
  # }
  # meth(2) {
  #   f.resume
  # }
  # f.resume
  # p Thread.current[:name]
  # #=> nil if fiber-local
  # #=> 2 if thread-local (The value 2 is leaked to outside of meth method.)
  # ```
  #
  # For thread-local variables, please see
  # [`thread_variable_get`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_get)
  # and
  # [`thread_variable_set`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_set).
  sig {params(key: T.any(String, Symbol)).returns(T.untyped)}
  def [](key); end

  # Attribute Assignment---Sets or creates the value of a fiber-local variable,
  # using either a symbol or a string.
  #
  # See also
  # [`Thread#[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D).
  #
  # For thread-local variables, please see
  # [`thread_variable_set`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_set)
  # and
  # [`thread_variable_get`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_get).
  sig {params(key: T.any(String, Symbol), value: T.untyped).returns(T.untyped)}
  def []=(key, value); end

  # Returns `true` if `thr` is running or sleeping.
  #
  # ```ruby
  # thr = Thread.new { }
  # thr.join                #=> #<Thread:0x401b3fb0 dead>
  # Thread.current.alive?   #=> true
  # thr.alive?              #=> false
  # ```
  #
  # See also
  # [`stop?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-stop-3F)
  # and
  # [`status`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-status).
  sig {returns(T::Boolean)}
  def alive?; end

  # Terminates `thr` and schedules another thread to be run, returning the
  # terminated [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html). If
  # this is the main thread, or the last thread, exits the process.
  sig {returns(T.nilable(Thread))}
  def kill; end

  # Returns the status of the thread-local "abort on exception" condition for
  # this `thr`.
  #
  # The default is `false`.
  #
  # See also
  # [`abort_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-abort_on_exception-3D).
  #
  # There is also a class level method to set this for all threads, see
  # [`::abort_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-abort_on_exception).
  sig {returns(T::Boolean)}
  def abort_on_exception; end

  # When set to `true`, if this `thr` is aborted by an exception, the raised
  # exception will be re-raised in the main thread.
  #
  # See also
  # [`abort_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-abort_on_exception).
  #
  # There is also a class level method to set this for all threads, see
  # [`::abort_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-abort_on_exception-3D).
  sig {params(abort_on_exception: T::Boolean).returns(T.untyped)}
  def abort_on_exception=(abort_on_exception); end

  # Adds *proc* as a handler for tracing.
  #
  # See
  # [`Thread#set_trace_func`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-set_trace_func)
  # and
  # [`Kernel#set_trace_func`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-set_trace_func).
  sig {params(proc: T.untyped).returns(T.untyped)}
  def add_trace_func(proc); end

  # Returns the current backtrace of the target thread.
  sig {params(args: T.untyped).returns(T::Array[T.untyped])}
  def backtrace(*args); end

  # Returns the execution stack for the target thread---an array containing
  # backtrace location objects.
  #
  # See
  # [`Thread::Backtrace::Location`](https://docs.ruby-lang.org/en/2.7.0/Thread/Backtrace/Location.html)
  # for more information.
  #
  # This method behaves similarly to
  # [`Kernel#caller_locations`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-caller_locations)
  # except it applies to a specific thread.
  sig {params(args: T.untyped).returns(T.nilable(T::Array[T.untyped]))}
  def backtrace_locations(*args); end

  # Terminates `thr` and schedules another thread to be run, returning the
  # terminated [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html). If
  # this is the main thread, or the last thread, exits the process.
  sig {returns(T.nilable(Thread))}
  def exit; end

  # Returns a fiber-local for the given key. If the key can't be found, there
  # are several options: With no other arguments, it will raise a
  # [`KeyError`](https://docs.ruby-lang.org/en/2.7.0/KeyError.html) exception;
  # if *default* is given, then that will be returned; if the optional code
  # block is specified, then that will be run and its result returned. See
  # [`Thread#[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D)
  # and
  # [`Hash#fetch`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-fetch).
  sig {params(sym: T.untyped).returns(T.untyped)}
  sig {params(sym: T.untyped, blk: T.proc.returns(T.untyped)).returns(T.untyped)}
  def fetch(*sym, &blk); end

  # Returns the
  # [`ThreadGroup`](https://docs.ruby-lang.org/en/2.7.0/ThreadGroup.html) which
  # contains the given thread, or returns `nil` if `thr` is not a member of any
  # group.
  #
  # ```ruby
  # Thread.main.group   #=> #<ThreadGroup:0x4029d914>
  # ```
  sig {returns(T.nilable(ThreadGroup))}
  def group; end

  sig {params(args: T.untyped, blk: T.untyped).void}
  def initialize(*args, &blk); end

  # The calling thread will suspend execution and run this `thr`.
  #
  # Does not return until `thr` exits or until the given `limit` seconds have
  # passed.
  #
  # If the time limit expires, `nil` will be returned, otherwise `thr` is
  # returned.
  #
  # Any threads not joined will be killed when the main program exits.
  #
  # If `thr` had previously raised an exception and the
  # [`::abort_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-abort_on_exception)
  # or $DEBUG flags are not set, (so the exception has not yet been processed),
  # it will be processed at this time.
  #
  # ```ruby
  # a = Thread.new { print "a"; sleep(10); print "b"; print "c" }
  # x = Thread.new { print "x"; Thread.pass; print "y"; print "z" }
  # x.join # Let thread x finish, thread a will be killed on exit.
  # #=> "axyz"
  # ```
  #
  # The following example illustrates the `limit` parameter.
  #
  # ```ruby
  # y = Thread.new { 4.times { sleep 0.1; puts 'tick... ' }}
  # puts "Waiting" until y.join(0.15)
  # ```
  #
  # This will produce:
  #
  # ```
  # tick...
  # Waiting
  # tick...
  # Waiting
  # tick...
  # tick...
  # ```
  sig {returns(Thread)}
  sig {params(limit: Numeric).returns(T.nilable(Thread))}
  def join(limit=nil); end

  # Returns `true` if the given string (or symbol) exists as a fiber-local
  # variable.
  #
  # ```ruby
  # me = Thread.current
  # me[:oliver] = "a"
  # me.key?(:oliver)    #=> true
  # me.key?(:stanley)   #=> false
  # ```
  sig {params(sym: Symbol).returns(T::Boolean)}
  def key?(sym); end

  # Returns an array of the names of the fiber-local variables (as Symbols).
  #
  # ```ruby
  # thr = Thread.new do
  #   Thread.current[:cat] = 'meow'
  #   Thread.current["dog"] = 'woof'
  # end
  # thr.join   #=> #<Thread:0x401b3f10 dead>
  # thr.keys   #=> [:dog, :cat]
  # ```
  sig {returns(T::Array[Symbol])}
  def keys; end

  # show the name of the thread.
  sig {returns(String)}
  def name; end

  # set given name to the ruby thread. On some platform, it may set the name to
  # pthread and/or kernel.
  sig {params(name: T.untyped).returns(T.untyped)}
  def name=(name); end

  # Return the native thread ID which is used by the Ruby thread.
  #
  # The ID depends on the OS. (not POSIX thread ID returned by pthread_self(3))
  # * On Linux it is TID returned by gettid(2).
  # * On macOS it is the system-wide unique integral ID of thread returned
  #   by pthread_threadid_np(3).
  # * On FreeBSD it is the unique integral ID of the thread returned by
  #   pthread_getthreadid_np(3).
  # * On Windows it is the thread identifier returned by GetThreadId().
  # * On other platforms, it raises NotImplementedError.
  #
  # NOTE:
  # If the thread is not associated yet or already deassociated with a native
  # thread, it returns _nil_.
  # If the Ruby implementation uses M:N thread model, the ID may change
  # depending on the timing.
  sig { returns(Integer) }
  def native_thread_id; end;

  # Returns whether or not the asynchronous queue is empty for the target
  # thread.
  #
  # If `error` is given, then check only for `error` type deferred events.
  #
  # See
  # [`::pending_interrupt?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-pending_interrupt-3F)
  # for more information.
  sig {params(args: T.untyped).returns(T::Boolean)}
  def pending_interrupt?(*args); end

  # Returns the priority of *thr*. Default is inherited from the current thread
  # which creating the new thread, or zero for the initial main thread;
  # higher-priority thread will run more frequently than lower-priority threads
  # (but lower-priority threads can also run).
  #
  # This is just hint for Ruby thread scheduler. It may be ignored on some
  # platform.
  #
  # ```ruby
  # Thread.current.priority   #=> 0
  # ```
  sig {returns(Integer)}
  def priority; end

  # Sets the priority of *thr* to *integer*. Higher-priority threads will run
  # more frequently than lower-priority threads (but lower-priority threads can
  # also run).
  #
  # This is just hint for Ruby thread scheduler. It may be ignored on some
  # platform.
  #
  # ```ruby
  # count1 = count2 = 0
  # a = Thread.new do
  #       loop { count1 += 1 }
  #     end
  # a.priority = -1
  #
  # b = Thread.new do
  #       loop { count2 += 1 }
  #     end
  # b.priority = -2
  # sleep 1   #=> 1
  # count1    #=> 622504
  # count2    #=> 5832
  # ```
  sig {params(priority: Integer).returns(T.untyped)}
  def priority=(priority); end

  # Raises an exception from the given thread. The caller does not have to be
  # `thr`. See
  # [`Kernel#raise`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-raise)
  # for more information.
  #
  # ```ruby
  # Thread.abort_on_exception = true
  # a = Thread.new { sleep(200) }
  # a.raise("Gotcha")
  # ```
  #
  # This will produce:
  #
  # ```
  # prog.rb:3: Gotcha (RuntimeError)
  #  from prog.rb:2:in `initialize'
  #  from prog.rb:2:in `new'
  #  from prog.rb:2
  # ```
  def raise(*_); end

  # Returns the status of the thread-local "report on exception" condition for
  # this `thr`.
  #
  # The default value when creating a
  # [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) is the value of
  # the global flag
  # [`Thread.report_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-report_on_exception).
  #
  # See also
  # [`report_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-report_on_exception-3D).
  #
  # There is also a class level method to set this for all new threads, see
  # [`::report_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-report_on_exception-3D).
  sig {returns(T::Boolean)}
  def report_on_exception; end

  # When set to `true`, a message is printed on $stderr if an exception kills
  # this `thr`. See
  # [`::report_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-report_on_exception)
  # for details.
  #
  # See also
  # [`report_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-report_on_exception).
  #
  # There is also a class level method to set this for all new threads, see
  # [`::report_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-report_on_exception-3D).
  sig {params(report_on_exception: T::Boolean).returns(T.untyped)}
  def report_on_exception=(report_on_exception); end

  # Wakes up `thr`, making it eligible for scheduling.
  #
  # ```ruby
  # a = Thread.new { puts "a"; Thread.stop; puts "c" }
  # sleep 0.1 while a.status!='sleep'
  # puts "Got here"
  # a.run
  # a.join
  # ```
  #
  # This will produce:
  #
  # ```ruby
  # a
  # Got here
  # c
  # ```
  #
  # See also the instance method
  # [`wakeup`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-wakeup).
  sig {returns(Thread)}
  def run; end

  # Returns the safe level.
  #
  # This method is obsolete because $SAFE is a process global state. Simply
  # check $SAFE.
  sig {returns(Integer)}
  def safe_level; end

  # Establishes *proc* on *thr* as the handler for tracing, or disables tracing
  # if the parameter is `nil`.
  #
  # See
  # [`Kernel#set_trace_func`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-set_trace_func).
  def set_trace_func(_); end

  # Returns the status of `thr`.
  #
  # `"sleep"`
  # :   Returned if this thread is sleeping or waiting on I/O
  # `"run"`
  # :   When this thread is executing
  # `"aborting"`
  # :   If this thread is aborting
  # `false`
  # :   When this thread is terminated normally
  # `nil`
  # :   If terminated with an exception.
  #
  #
  # ```ruby
  # a = Thread.new { raise("die now") }
  # b = Thread.new { Thread.stop }
  # c = Thread.new { Thread.exit }
  # d = Thread.new { sleep }
  # d.kill                  #=> #<Thread:0x401b3678 aborting>
  # a.status                #=> nil
  # b.status                #=> "sleep"
  # c.status                #=> false
  # d.status                #=> "aborting"
  # Thread.current.status   #=> "run"
  # ```
  #
  # See also the instance methods
  # [`alive?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-alive-3F)
  # and
  # [`stop?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-stop-3F)
  sig {returns(T.nilable(T.any(String, T::Boolean)))}
  def status; end

  # Returns `true` if `thr` is dead or sleeping.
  #
  # ```ruby
  # a = Thread.new { Thread.stop }
  # b = Thread.current
  # a.stop?   #=> true
  # b.stop?   #=> false
  # ```
  #
  # See also
  # [`alive?`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-alive-3F)
  # and
  # [`status`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-status).
  sig {returns(T::Boolean)}
  def stop?; end

  # Terminates `thr` and schedules another thread to be run, returning the
  # terminated [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html). If
  # this is the main thread, or the last thread, exits the process.
  sig {returns(T.nilable(Thread))}
  def terminate; end

  # Returns `true` if the given string (or symbol) exists as a thread-local
  # variable.
  #
  # ```ruby
  # me = Thread.current
  # me.thread_variable_set(:oliver, "a")
  # me.thread_variable?(:oliver)    #=> true
  # me.thread_variable?(:stanley)   #=> false
  # ```
  #
  # Note that these are not fiber local variables. Please see
  # [`Thread#[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D)
  # and
  # [`Thread#thread_variable_get`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_get)
  # for more details.
  sig {params(key: T.any(String, Symbol)).returns(T::Boolean)}
  def thread_variable?(key); end

  # Returns the value of a thread local variable that has been set. Note that
  # these are different than fiber local values. For fiber local values, please
  # see
  # [`Thread#[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D)
  # and
  # [`Thread#[]=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D-3D).
  #
  # [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) local values are
  # carried along with threads, and do not respect fibers. For example:
  #
  # ```ruby
  # Thread.new {
  #   Thread.current.thread_variable_set("foo", "bar") # set a thread local
  #   Thread.current["foo"] = "bar"                    # set a fiber local
  #
  #   Fiber.new {
  #     Fiber.yield [
  #       Thread.current.thread_variable_get("foo"), # get the thread local
  #       Thread.current["foo"],                     # get the fiber local
  #     ]
  #   }.resume
  # }.join.value # => ['bar', nil]
  # ```
  #
  # The value "bar" is returned for the thread local, where nil is returned for
  # the fiber local. The fiber is executed in the same thread, so the thread
  # local values are available.
  sig {params(key: T.untyped).returns(T.untyped)}
  def thread_variable_get(key); end

  # Sets a thread local with `key` to `value`. Note that these are local to
  # threads, and not to fibers. Please see
  # [`Thread#thread_variable_get`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_get)
  # and
  # [`Thread#[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D)
  # for more information.
  sig {params(key: T.untyped, value: T.untyped).returns(T.untyped)}
  def thread_variable_set(key, value); end

  # Returns an array of the names of the thread-local variables (as Symbols).
  #
  # ```ruby
  # thr = Thread.new do
  #   Thread.current.thread_variable_set(:cat, 'meow')
  #   Thread.current.thread_variable_set("dog", 'woof')
  # end
  # thr.join               #=> #<Thread:0x401b3f10 dead>
  # thr.thread_variables   #=> [:dog, :cat]
  # ```
  #
  # Note that these are not fiber local variables. Please see
  # [`Thread#[]`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-5B-5D)
  # and
  # [`Thread#thread_variable_get`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-thread_variable_get)
  # for more details.
  sig {returns(T::Array[Symbol])}
  def thread_variables; end

  # Waits for `thr` to complete, using
  # [`join`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-join), and
  # returns its value or raises the exception which terminated the thread.
  #
  # ```ruby
  # a = Thread.new { 2 + 2 }
  # a.value   #=> 4
  #
  # b = Thread.new { raise 'something went wrong' }
  # b.value   #=> RuntimeError: something went wrong
  # ```
  sig {returns(Object)}
  def value; end

  # Marks a given thread as eligible for scheduling, however it may still remain
  # blocked on I/O.
  #
  # **Note:** This does not invoke the scheduler, see
  # [`run`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-run) for
  # more information.
  #
  # ```ruby
  # c = Thread.new { Thread.stop; puts "hey!" }
  # sleep 0.1 while c.status!='sleep'
  # c.wakeup
  # c.join
  # #=> "hey!"
  # ```
  sig {returns(Thread)}
  def wakeup; end

  # Returns the status of the global "abort on exception" condition.
  #
  # The default is `false`.
  #
  # When set to `true`, if any thread is aborted by an exception, the raised
  # exception will be re-raised in the main thread.
  #
  # Can also be specified by the global $DEBUG flag or command line option `-d`.
  #
  # See also
  # [`::abort_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-abort_on_exception-3D).
  #
  # There is also an instance level method to set this for a specific thread,
  # see
  # [`abort_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-abort_on_exception).
  sig {returns(T.untyped)}
  def self.abort_on_exception; end

  # When set to `true`, if any thread is aborted by an exception, the raised
  # exception will be re-raised in the main thread. Returns the new state.
  #
  # ```ruby
  # Thread.abort_on_exception = true
  # t1 = Thread.new do
  #   puts  "In new thread"
  #   raise "Exception from thread"
  # end
  # sleep(1)
  # puts "not reached"
  # ```
  #
  # This will produce:
  #
  # ```
  # In new thread
  # prog.rb:4: Exception from thread (RuntimeError)
  #  from prog.rb:2:in `initialize'
  #  from prog.rb:2:in `new'
  #  from prog.rb:2
  # ```
  #
  # See also
  # [`::abort_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-abort_on_exception).
  #
  # There is also an instance level method to set this for a specific thread,
  # see
  # [`abort_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-abort_on_exception-3D).
  sig {params(abort_on_exception: T.untyped).returns(T.untyped)}
  def self.abort_on_exception=(abort_on_exception); end

  # Terminates the currently running thread and schedules another thread to be
  # run.
  #
  # If this thread is already marked to be killed,
  # [`::exit`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-exit)
  # returns the [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html).
  #
  # If this is the main thread, or the last  thread, exit the process.
  sig {returns(T.untyped)}
  def self.exit; end

  # Basically the same as
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-new).
  # However, if class
  # [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) is subclassed,
  # then calling `start` in that subclass will not invoke the subclass's
  # `initialize` method.
  sig {params(args: T.untyped).returns(T.untyped)}
  def self.fork(*args); end

  # Changes asynchronous interrupt timing.
  #
  # *interrupt* means asynchronous event and corresponding procedure by
  # [`Thread#raise`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-raise),
  # [`Thread#kill`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-kill),
  # signal trap (not supported yet) and main thread termination (if main thread
  # terminates, then all other thread will be killed).
  #
  # The given `hash` has pairs like `ExceptionClass => :TimingSymbol`. Where the
  # ExceptionClass is the interrupt handled by the given block. The TimingSymbol
  # can be one of the following symbols:
  #
  # `:immediate`
  # :   Invoke interrupts immediately.
  # `:on_blocking`
  # :   Invoke interrupts while *BlockingOperation*.
  # `:never`
  # :   Never invoke all interrupts.
  #
  #
  # *BlockingOperation* means that the operation will block the calling thread,
  # such as read and write. On CRuby implementation, *BlockingOperation* is any
  # operation executed without GVL.
  #
  # Masked asynchronous interrupts are delayed until they are enabled. This
  # method is similar to sigprocmask(3).
  #
  # ### NOTE
  #
  # Asynchronous interrupts are difficult to use.
  #
  # If you need to communicate between threads, please consider to use another
  # way such as [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html).
  #
  # Or use them with deep understanding about this method.
  #
  # ### Usage
  #
  # In this example, we can guard from
  # [`Thread#raise`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-raise)
  # exceptions.
  #
  # Using the `:never` TimingSymbol the
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html)
  # exception will always be ignored in the first block of the main thread. In
  # the second
  # [`::handle_interrupt`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-handle_interrupt)
  # block we can purposefully handle
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html)
  # exceptions.
  #
  # ```ruby
  # th = Thread.new do
  #   Thread.handle_interrupt(RuntimeError => :never) {
  #     begin
  #       # You can write resource allocation code safely.
  #       Thread.handle_interrupt(RuntimeError => :immediate) {
  #         # ...
  #       }
  #     ensure
  #       # You can write resource deallocation code safely.
  #     end
  #   }
  # end
  # Thread.pass
  # # ...
  # th.raise "stop"
  # ```
  #
  # While we are ignoring the
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html)
  # exception, it's safe to write our resource allocation code. Then, the ensure
  # block is where we can safely deallocate your resources.
  #
  # #### Guarding from [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html)
  #
  # In the next example, we will guard from the
  # [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html)
  # exception. This will help prevent from leaking resources when
  # [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html)
  # exceptions occur during normal ensure clause. For this example we use the
  # help of the standard library
  # [`Timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html), from
  # lib/timeout.rb
  #
  # ```ruby
  # require 'timeout'
  # Thread.handle_interrupt(Timeout::Error => :never) {
  #   timeout(10){
  #     # Timeout::Error doesn't occur here
  #     Thread.handle_interrupt(Timeout::Error => :on_blocking) {
  #       # possible to be killed by Timeout::Error
  #       # while blocking operation
  #     }
  #     # Timeout::Error doesn't occur here
  #   }
  # }
  # ```
  #
  # In the first part of the `timeout` block, we can rely on
  # [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html)
  # being ignored. Then in the `Timeout::Error => :on_blocking` block, any
  # operation that will block the calling thread is susceptible to a
  # [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html)
  # exception being raised.
  #
  # #### Stack control settings
  #
  # It's possible to stack multiple levels of
  # [`::handle_interrupt`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-handle_interrupt)
  # blocks in order to control more than one ExceptionClass and TimingSymbol at
  # a time.
  #
  # ```ruby
  # Thread.handle_interrupt(FooError => :never) {
  #   Thread.handle_interrupt(BarError => :never) {
  #      # FooError and BarError are prohibited.
  #   }
  # }
  # ```
  #
  # #### Inheritance with ExceptionClass
  #
  # All exceptions inherited from the ExceptionClass parameter will be
  # considered.
  #
  # ```ruby
  # Thread.handle_interrupt(Exception => :never) {
  #   # all exceptions inherited from Exception are prohibited.
  # }
  # ```
  sig {params(hash: T.untyped, block: T.proc.returns(T.untyped)).returns(T.untyped)}
  def self.handle_interrupt(hash, &block); end

  # Causes the given `thread` to exit, see also
  # [`Thread::exit`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-exit).
  #
  # ```ruby
  # count = 0
  # a = Thread.new { loop { count += 1 } }
  # sleep(0.1)       #=> 0
  # Thread.kill(a)   #=> #<Thread:0x401b3d30 dead>
  # count            #=> 93947
  # a.alive?         #=> false
  # ```
  sig {params(thread: Thread).returns(T.untyped)}
  def self.kill(thread); end

  # Returns an array of
  # [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) objects for all
  # threads that are either runnable or stopped.
  #
  # ```ruby
  # Thread.new { sleep(200) }
  # Thread.new { 1000000.times {|i| i*i } }
  # Thread.new { Thread.stop }
  # Thread.list.each {|t| p t}
  # ```
  #
  # This will produce:
  #
  # ```ruby
  # #<Thread:0x401b3e84 sleep>
  # #<Thread:0x401b3f38 run>
  # #<Thread:0x401b3fb0 sleep>
  # #<Thread:0x401bdf4c run>
  # ```
  sig {returns(T.untyped)}
  def self.list; end

  # Give the thread scheduler a hint to pass execution to another thread. A
  # running thread may or may not switch, it depends on OS and processor.
  sig {returns(T.untyped)}
  def self.pass; end

  # Returns whether or not the asynchronous queue is empty.
  #
  # Since
  # [`Thread::handle_interrupt`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-handle_interrupt)
  # can be used to defer asynchronous events, this method can be used to
  # determine if there are any deferred events.
  #
  # If you find this method returns true, then you may finish `:never` blocks.
  #
  # For example, the following method processes deferred asynchronous events
  # immediately.
  #
  # ```ruby
  # def Thread.kick_interrupt_immediately
  #   Thread.handle_interrupt(Object => :immediate) {
  #     Thread.pass
  #   }
  # end
  # ```
  #
  # If `error` is given, then check only for `error` type deferred events.
  #
  # ### Usage
  #
  # ```
  # th = Thread.new{
  #   Thread.handle_interrupt(RuntimeError => :on_blocking){
  #     while true
  #       ...
  #       # reach safe point to invoke interrupt
  #       if Thread.pending_interrupt?
  #         Thread.handle_interrupt(Object => :immediate){}
  #       end
  #       ...
  #     end
  #   }
  # }
  # ...
  # th.raise # stop thread
  # ```
  #
  # This example can also be written as the following, which you should use to
  # avoid asynchronous interrupts.
  #
  # ```
  # flag = true
  # th = Thread.new{
  #   Thread.handle_interrupt(RuntimeError => :on_blocking){
  #     while true
  #       ...
  #       # reach safe point to invoke interrupt
  #       break if flag == false
  #       ...
  #     end
  #   }
  # }
  # ...
  # flag = false # stop thread
  # ```
  sig {params(args: T.untyped).returns(T::Boolean)}
  def self.pending_interrupt?(*args); end

  # Returns the status of the global "report on exception" condition.
  #
  # The default is `true` since Ruby 2.5.
  #
  # All threads created when this flag is true will report a message on $stderr
  # if an exception kills the thread.
  #
  # ```ruby
  # Thread.new { 1.times { raise } }
  # ```
  #
  # will produce this output on $stderr:
  #
  # ```
  # #<Thread:...> terminated with exception (report_on_exception is true):
  # Traceback (most recent call last):
  #         2: from -e:1:in `block in <main>'
  #         1: from -e:1:in `times'
  # ```
  #
  # This is done to catch errors in threads early. In some cases, you might not
  # want this output. There are multiple ways to avoid the extra output:
  #
  # *   If the exception is not intended, the best is to fix the cause of the
  #     exception so it does not happen anymore.
  # *   If the exception is intended, it might be better to rescue it closer to
  #     where it is raised rather then let it kill the
  #     [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html).
  # *   If it is guaranteed the
  #     [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) will be
  #     joined with
  #     [`Thread#join`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-join)
  #     or
  #     [`Thread#value`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-value),
  #     then it is safe to disable this report with
  #     `Thread.current.report_on_exception = false` when starting the
  #     [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html). However,
  #     this might handle the exception much later, or not at all if the
  #     [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) is never
  #     joined due to the parent thread being blocked, etc.
  #
  #
  # See also
  # [`::report_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-report_on_exception-3D).
  #
  # There is also an instance level method to set this for a specific thread,
  # see
  # [`report_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-report_on_exception-3D).
  sig {returns(T.untyped)}
  def self.report_on_exception; end

  # Returns the new state. When set to `true`, all threads created afterwards
  # will inherit the condition and report a message on $stderr if an exception
  # kills a thread:
  #
  # ```ruby
  # Thread.report_on_exception = true
  # t1 = Thread.new do
  #   puts  "In new thread"
  #   raise "Exception from thread"
  # end
  # sleep(1)
  # puts "In the main thread"
  # ```
  #
  # This will produce:
  #
  # ```
  # In new thread
  # #<Thread:...prog.rb:2> terminated with exception (report_on_exception is true):
  # Traceback (most recent call last):
  # prog.rb:4:in `block in <main>': Exception from thread (RuntimeError)
  # In the main thread
  # ```
  #
  # See also
  # [`::report_on_exception`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-report_on_exception).
  #
  # There is also an instance level method to set this for a specific thread,
  # see
  # [`report_on_exception=`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-report_on_exception-3D).
  sig {params(report_on_exception: T.untyped).returns(T.untyped)}
  def self.report_on_exception=(report_on_exception); end

  # Basically the same as
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-c-new).
  # However, if class
  # [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) is subclassed,
  # then calling `start` in that subclass will not invoke the subclass's
  # `initialize` method.
  sig {params(args: T.untyped, blk: T.untyped).returns(T.untyped)}
  def self.start(*args, &blk); end

  # Stops execution of the current thread, putting it into a "sleep" state, and
  # schedules execution of another thread.
  #
  # ```ruby
  # a = Thread.new { print "a"; Thread.stop; print "c" }
  # sleep 0.1 while a.status!='sleep'
  # print "b"
  # a.run
  # a.join
  # #=> "abc"
  # ```
  sig {returns(T.untyped)}
  def self.stop; end

  # Yields each frame of the current execution stack as a
  # backtrace location object.
  sig {params(blk: T.proc.params(location: Thread::Backtrace::Location).void).returns(T.untyped)}
  def self.each_caller_location(&blk); end
end

class Thread::Backtrace < Object
end

# An object representation of a stack frame, initialized by
# [`Kernel#caller_locations`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-caller_locations).
#
# For example:
#
# ```ruby
# # caller_locations.rb
# def a(skip)
#   caller_locations(skip)
# end
# def b(skip)
#   a(skip)
# end
# def c(skip)
#   b(skip)
# end
#
# c(0..2).map do |call|
#   puts call.to_s
# end
# ```
#
# Running `ruby caller_locations.rb` will produce:
#
# ```
# caller_locations.rb:2:in `a'
# caller_locations.rb:5:in `b'
# caller_locations.rb:8:in `c'
# ```
#
# Here's another example with a slightly different result:
#
# ```ruby
# # foo.rb
# class Foo
#   attr_accessor :locations
#   def initialize(skip)
#     @locations = caller_locations(skip)
#   end
# end
#
# Foo.new(0..2).locations.map do |call|
#   puts call.to_s
# end
# ```
#
# Now run `ruby foo.rb` and you should see:
#
# ```
# init.rb:4:in `initialize'
# init.rb:8:in `new'
# init.rb:8:in `<main>'
# ```
class Thread::Backtrace::Location
  # Returns the full file path of this frame.
  #
  # Same as
  # [`path`](https://docs.ruby-lang.org/en/2.7.0/Thread/Backtrace/Location.html#method-i-path),
  # but includes the absolute path.
  sig {returns(T.nilable(String))}
  def absolute_path(); end

  # Returns the base label of this frame.
  #
  # Usually same as
  # [`label`](https://docs.ruby-lang.org/en/2.7.0/Thread/Backtrace/Location.html#method-i-label),
  # without decoration.
  sig {returns(T.nilable(String))}
  def base_label(); end

  # Returns the label of this frame.
  #
  # Usually consists of method, class, module, etc names with decoration.
  #
  # Consider the following example:
  #
  # ```ruby
  # def foo
  #   puts caller_locations(0).first.label
  #
  #   1.times do
  #     puts caller_locations(0).first.label
  #
  #     1.times do
  #       puts caller_locations(0).first.label
  #     end
  #
  #   end
  # end
  # ```
  #
  # The result of calling `foo` is this:
  #
  # ```
  # label: foo
  # label: block in foo
  # label: block (2 levels) in foo
  # ```
  sig {returns(T.nilable(String))}
  def label(); end

  # Returns the line number of this frame.
  #
  # For example, using `caller_locations.rb` from
  # [`Thread::Backtrace::Location`](https://docs.ruby-lang.org/en/2.7.0/Thread/Backtrace/Location.html)
  #
  # ```ruby
  # loc = c(0..1).first
  # loc.lineno #=> 2
  # ```
  sig {returns(Integer)}
  def lineno(); end

  # Returns the file name of this frame.
  #
  # For example, using `caller_locations.rb` from
  # [`Thread::Backtrace::Location`](https://docs.ruby-lang.org/en/2.7.0/Thread/Backtrace/Location.html)
  #
  # ```ruby
  # loc = c(0..1).first
  # loc.path #=> caller_locations.rb
  # ```
  sig {returns(T.nilable(String))}
  def path(); end
end

# [`ConditionVariable`](https://docs.ruby-lang.org/en/2.7.0/ConditionVariable.html)
# objects augment class
# [`Mutex`](https://docs.ruby-lang.org/en/2.7.0/Mutex.html). Using condition
# variables, it is possible to suspend while in the middle of a critical section
# until a resource becomes available.
#
# Example:
#
# ```ruby
# mutex = Mutex.new
# resource = ConditionVariable.new
#
# a = Thread.new {
#    mutex.synchronize {
#      # Thread 'a' now needs the resource
#      resource.wait(mutex)
#      # 'a' can now have the resource
#    }
# }
#
# b = Thread.new {
#    mutex.synchronize {
#      # Thread 'b' has finished using the resource
#      resource.signal
#    }
# }
# ```
class Thread::ConditionVariable < Object
  # Wakes up all threads waiting for this lock.
  sig {returns(T.untyped)}
  def broadcast; end

  sig {returns(T.untyped)}
  def marshal_dump; end

  # Wakes up the first thread in line waiting for this lock.
  sig {returns(T.untyped)}
  def signal; end

  # Releases the lock held in `mutex` and waits; reacquires the lock on wakeup.
  #
  # If `timeout` is given, this method returns after `timeout` seconds passed,
  # even if no other thread doesn't signal.
  sig {params(_: T.untyped).returns(T.untyped)}
  def wait(*_); end
end

# [`Mutex`](https://docs.ruby-lang.org/en/2.7.0/Mutex.html) implements a simple
# semaphore that can be used to coordinate access to shared data from multiple
# concurrent threads.
#
# Example:
#
# ```ruby
# semaphore = Mutex.new
#
# a = Thread.new {
#   semaphore.synchronize {
#     # access shared resource
#   }
# }
#
# b = Thread.new {
#   semaphore.synchronize {
#     # access shared resource
#   }
# }
# ```
class Thread::Mutex < Object
  # Attempts to grab the lock and waits if it isn't available. Raises
  # `ThreadError` if `mutex` was locked by the current thread.
  sig {returns(T.untyped)}
  def lock; end

  # Returns `true` if this lock is currently held by some thread.
  sig {returns(T::Boolean)}
  def locked?; end

  # Returns `true` if this lock is currently held by current thread.
  sig {returns(T::Boolean)}
  def owned?; end

  # Obtains a lock, runs the block, and releases the lock when the block
  # completes. See the example under `Mutex`.
  sig {type_parameters(:T).params(blk: T.proc.returns(T.type_parameter(:T))).returns(T.type_parameter(:T))}
  def synchronize(&blk); end

  # Attempts to obtain the lock and returns immediately. Returns `true` if the
  # lock was granted.
  sig {returns(T::Boolean)}
  def try_lock; end

  # Releases the lock. Raises `ThreadError` if `mutex` wasn't locked by the
  # current thread.
  sig {returns(T.untyped)}
  def unlock; end
end

# The [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html) class implements
# multi-producer, multi-consumer queues. It is especially useful in threaded
# programming when information must be exchanged safely between multiple
# threads. The [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html) class
# implements all the required locking semantics.
#
# The class implements FIFO type of queue. In a FIFO queue, the first tasks
# added are the first retrieved.
#
# Example:
#
# ```ruby
# queue = Queue.new
#
# producer = Thread.new do
#   5.times do |i|
#      sleep rand(i) # simulate expense
#      queue << i
#      puts "#{i} produced"
#   end
# end
#
# consumer = Thread.new do
#   5.times do |i|
#      value = queue.pop
#      sleep rand(i/2) # simulate expense
#      puts "consumed #{value}"
#   end
# end
#
# consumer.join
# ```
class Thread::Queue < Object
  # Creates a new queue instance, optionally using the contents of an enumerable for its initial state.
  # https://ruby-doc.org/core-3.1.0/Thread/Queue.html#method-c-new
  sig {params(enumerable: T::Enumerable[T.untyped]).void}
  def initialize(enumerable=T.unsafe(nil)); end

  # Alias for:
  # [`push`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-push)
  sig {params(obj: T.untyped).returns(T.untyped)}
  def <<(obj); end

  # Removes all objects from the queue.
  sig {returns(T.untyped)}
  def clear; end

  # Closes the queue. A closed queue cannot be re-opened.
  #
  # After the call to close completes, the following are true:
  #
  # *   `closed?` will return true
  #
  # *   `close` will be ignored.
  #
  # *   calling enq/push/<< will raise a `ClosedQueueError`.
  #
  # *   when `empty?` is false, calling deq/pop/shift will return an object from
  #     the queue as usual.
  # *   when `empty?` is true, deq(false) will not suspend the thread and will
  #     return nil. deq(true) will raise a `ThreadError`.
  #
  #
  # [`ClosedQueueError`](https://docs.ruby-lang.org/en/2.7.0/ClosedQueueError.html)
  # is inherited from
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.7.0/StopIteration.html),
  # so that you can break loop block.
  #
  # ```
  # Example:
  #
  #     q = Queue.new
  #     Thread.new{
  #       while e = q.deq # wait for nil to break loop
  #         # ...
  #       end
  #     }
  #     q.close
  # ```
  sig {returns(T.untyped)}
  def close; end

  # Returns `true` if the queue is closed.
  sig {returns(T::Boolean)}
  def closed?; end

  # Alias for:
  # [`pop`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-pop)
  sig {params(non_block: T::Boolean, timeout: T.nilable(Integer)).returns(T.untyped)}
  def deq(non_block=false, timeout: nil); end

  # Returns `true` if the queue is empty.
  sig {returns(T::Boolean)}
  def empty?; end

  # Alias for:
  # [`push`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-push)
  sig {params(obj: T.untyped).returns(T.untyped)}
  def enq(obj); end

  # Returns the length of the queue.
  #
  # Also aliased as:
  # [`size`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-size)
  sig {returns(Integer)}
  def length; end

  sig {returns(T.untyped)}
  def marshal_dump; end

  # Returns the number of threads waiting on the queue.
  sig {returns(Integer)}
  def num_waiting; end

  # Retrieves data from the queue.
  #
  # If the queue is empty, the calling thread is suspended until data is pushed
  # onto the queue. If `non_block` is true, the thread isn't suspended, and
  # `ThreadError` is raised.
  #
  # Also aliased as:
  # [`deq`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-deq),
  # [`shift`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-shift)
  sig {params(non_block: T::Boolean, timeout: T.nilable(Integer)).returns(T.untyped)}
  def pop(non_block=false, timeout: nil); end

  # Pushes the given `object` to the queue.
  #
  # Also aliased as:
  # [`enq`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-enq),
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-3C-3C)
  sig {params(obj: T.untyped).returns(T.untyped)}
  def push(obj); end

  # Alias for:
  # [`pop`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-pop)
  sig {params(non_block: T::Boolean, timeout: T.nilable(Integer)).returns(T.untyped)}
  def shift(non_block=false, timeout: nil); end

  # Alias for:
  # [`length`](https://docs.ruby-lang.org/en/2.7.0/Queue.html#method-i-length)
  sig {returns(Integer)}
  def size; end
end

# This class represents queues of specified size capacity. The push operation
# may be blocked if the capacity is full.
#
# See [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html) for an example
# of how a [`SizedQueue`](https://docs.ruby-lang.org/en/2.7.0/SizedQueue.html)
# works.
class Thread::SizedQueue < Thread::Queue
  # Alias for:
  # [`push`](https://docs.ruby-lang.org/en/2.7.0/SizedQueue.html#method-i-push)
  sig {params(args: T.untyped).returns(T.untyped)}
  def <<(*args); end

  # Alias for:
  # [`push`](https://docs.ruby-lang.org/en/2.7.0/SizedQueue.html#method-i-push)
  sig {params(args: T.untyped).returns(T.untyped)}
  def enq(*args); end

  sig {params(max: T.untyped).void}
  def initialize(max); end

  # Returns the maximum size of the queue.
  sig {returns(Integer)}
  def max; end

  # Sets the maximum size of the queue to the given `number`.
  sig {params(max: Integer).returns(T.untyped)}
  def max=(max); end

  # Pushes `object` to the queue.
  #
  # If there is no space left in the queue, waits until space becomes available,
  # unless `non_block` is true. If `non_block` is true, the thread isn't
  # suspended, and `ThreadError` is raised.
  #
  # Also aliased as:
  # [`enq`](https://docs.ruby-lang.org/en/2.7.0/SizedQueue.html#method-i-enq),
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/SizedQueue.html#method-i-3C-3C)
  sig {params(args: T.untyped).returns(T.untyped)}
  def push(*args); end
end

# [`ConditionVariable`](https://docs.ruby-lang.org/en/2.7.0/ConditionVariable.html)
# objects augment class
# [`Mutex`](https://docs.ruby-lang.org/en/2.7.0/Mutex.html). Using condition
# variables, it is possible to suspend while in the middle of a critical section
# until a resource becomes available.
#
# Example:
#
# ```ruby
# mutex = Mutex.new
# resource = ConditionVariable.new
#
# a = Thread.new {
#    mutex.synchronize {
#      # Thread 'a' now needs the resource
#      resource.wait(mutex)
#      # 'a' can now have the resource
#    }
# }
#
# b = Thread.new {
#    mutex.synchronize {
#      # Thread 'b' has finished using the resource
#      resource.signal
#    }
# }
# ```
ConditionVariable = Thread::ConditionVariable
# [`Mutex`](https://docs.ruby-lang.org/en/2.7.0/Mutex.html) implements a simple
# semaphore that can be used to coordinate access to shared data from multiple
# concurrent threads.
#
# Example:
#
# ```ruby
# semaphore = Mutex.new
#
# a = Thread.new {
#   semaphore.synchronize {
#     # access shared resource
#   }
# }
#
# b = Thread.new {
#   semaphore.synchronize {
#     # access shared resource
#   }
# }
# ```
Mutex = Thread::Mutex
# The [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html) class implements
# multi-producer, multi-consumer queues. It is especially useful in threaded
# programming when information must be exchanged safely between multiple
# threads. The [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html) class
# implements all the required locking semantics.
#
# The class implements FIFO type of queue. In a FIFO queue, the first tasks
# added are the first retrieved.
#
# Example:
#
# ```ruby
# queue = Queue.new
#
# producer = Thread.new do
#   5.times do |i|
#      sleep rand(i) # simulate expense
#      queue << i
#      puts "#{i} produced"
#   end
# end
#
# consumer = Thread.new do
#   5.times do |i|
#      value = queue.pop
#      sleep rand(i/2) # simulate expense
#      puts "consumed #{value}"
#   end
# end
#
# consumer.join
# ```
Queue = Thread::Queue
# This class represents queues of specified size capacity. The push operation
# may be blocked if the capacity is full.
#
# See [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html) for an example
# of how a [`SizedQueue`](https://docs.ruby-lang.org/en/2.7.0/SizedQueue.html)
# works.
SizedQueue = Thread::SizedQueue
