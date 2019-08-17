# typed: __STDLIB_INTERNAL

# Threads are the Ruby implementation for a concurrent programming model.
#
# Programs that require multiple threads of execution are a perfect
# candidate for Ruby's [Thread](Thread) class.
#
# For example, we can create a new thread separate from the main thread's
# execution using [::new](Thread#method-c-new).
#
# ```ruby
# thr = Thread.new { puts "Whats the big deal" }
# ```
#
# Then we are able to pause the execution of the main thread and allow our
# new thread to finish, using
# [join](Thread#method-i-join):
#
# ```ruby
# thr.join #=> "Whats the big deal"
# ```
#
# If we don't call `thr.join` before the main thread terminates, then all
# other threads including `thr` will be killed.
#
# Alternatively, you can use an array for handling multiple threads at
# once, like in the following example:
#
# ```ruby
# threads = []
# threads << Thread.new { puts "Whats the big deal" }
# threads << Thread.new { 3.times { puts "Threads are fun!" } }
# ```
#
# After creating a few threads we wait for them all to finish
# consecutively.
#
# ```ruby
# threads.each { |thr| thr.join }
# ```
#
#
# In order to create new threads, Ruby provides
# [::new](Thread#method-c-new),
# [::start](Thread#method-c-start), and
# [::fork](Thread#method-c-fork). A block must be
# provided with each of these methods, otherwise a
# [ThreadError](https://ruby-doc.org/core-2.6.3/ThreadError.html) will be
# raised.
#
# When subclassing the [Thread](Thread) class, the
# `initialize` method of your subclass will be ignored by
# [::start](Thread#method-c-start) and
# [::fork](Thread#method-c-fork). Otherwise, be sure
# to call super in your `initialize` method.
#
#
# For terminating threads, Ruby provides a variety of ways to do this.
#
# The class method [::kill](Thread#method-c-kill), is
# meant to exit a given thread:
#
#     thr = Thread.new { ... }
#     Thread.kill(thr) # sends exit() to thr
#
# Alternatively, you can use the instance method
# [exit](Thread#method-i-exit), or any of its aliases
# [kill](Thread#method-i-kill) or
# [terminate](Thread#method-i-terminate).
#
# ```ruby
# thr.exit
# ```
#
#
# Ruby provides a few instance methods for querying the state of a given
# thread. To get a string with the current thread's state use
# [status](Thread#method-i-status)
#
# ```ruby
# thr = Thread.new { sleep }
# thr.status # => "sleep"
# thr.exit
# thr.status # => false
# ```
#
# You can also use [alive?](Thread#method-i-alive-3F)
# to tell if the thread is running or sleeping, and
# [stop?](Thread#method-i-stop-3F) if the thread is
# dead or sleeping.
#
#
# Since threads are created with blocks, the same rules apply to other
# Ruby blocks for variable scope. Any local variables created within this
# block are accessible to only this thread.
#
#
# Each fiber has its own bucket for
# [\#\[\]](Thread#method-i-5B-5D) storage. When you
# set a new fiber-local it is only accessible within this
# [Fiber](https://ruby-doc.org/core-2.6.3/Fiber.html). To illustrate:
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
# This example uses [\[\]](Thread#method-i-5B-5D) for
# getting and [\[\]=](Thread#method-i-5B-5D-3D) for
# setting fiber-locals, you can also use
# [keys](Thread#method-i-keys) to list the
# fiber-locals for a given thread and
# [key?](Thread#method-i-key-3F) to check if a
# fiber-local exists.
#
# When it comes to thread-locals, they are accessible within the entire
# scope of the thread. Given the following example:
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
# You can see that the thread-local `:foo` carried over into the fiber and
# was changed to `2` by the end of the thread.
#
# This example makes use of
# [thread\_variable\_set](Thread#method-i-thread_variable_set)
# to create new thread-locals, and
# [thread\_variable\_get](Thread#method-i-thread_variable_get)
# to reference them.
#
# There is also
# [thread\_variables](Thread#method-i-thread_variables)
# to list all thread-locals, and
# [thread\_variable?](Thread#method-i-thread_variable-3F)
# to check if a given thread-local exists.
#
#
# Any thread can raise an exception using the
# [raise](Thread#method-i-raise) instance method,
# which operates similarly to
# [Kernel\#raise](https://ruby-doc.org/core-2.6.3/Kernel.html#method-i-raise)
# .
#
# However, it's important to note that an exception that occurs in any
# thread except the main thread depends on
# [abort\_on\_exception](Thread#method-i-abort_on_exception)
# . This option is `false` by default, meaning that any unhandled
# exception will cause the thread to terminate silently when waited on by
# either [join](Thread#method-i-join) or
# [value](Thread#method-i-value). You can change this
# default by either
# [abort\_on\_exception=](Thread#method-i-abort_on_exception-3D)
# `true` or setting $DEBUG to `true` .
#
# With the addition of the class method
# [::handle\_interrupt](Thread#method-c-handle_interrupt)
# , you can now handle exceptions asynchronously with threads.
#
#
# Ruby provides a few ways to support scheduling threads in your program.
#
# The first way is by using the class method
# [::stop](Thread#method-c-stop), to put the current
# running thread to sleep and schedule the execution of another thread.
#
# Once a thread is asleep, you can use the instance method
# [wakeup](Thread#method-i-wakeup) to mark your thread
# as eligible for scheduling.
#
# You can also try [::pass](Thread#method-c-pass),
# which attempts to pass execution to another thread but is dependent on
# the OS whether a running thread will switch or not. The same goes for
# [priority](Thread#method-i-priority), which lets
# you hint to the thread scheduler which threads you want to take
# precedence when passing execution. This method is also dependent on the
# OS and may be ignored on some platforms.
class Thread < Object
  sig {returns(Thread)}
  def self.current; end

  # Returns the main thread.
  sig {returns(Thread)}
  def self.main; end

  sig {params(key: T.any(String, Symbol)).returns(T.untyped)}
  def [](key); end

  # Attribute Assignment—Sets or creates the value of a fiber-local
  # variable, using either a symbol or a string.
  #
  # See also [\#\[\]](Thread.downloaded.ruby_doc#method-i-5B-5D).
  #
  # For thread-local variables, please see
  # [thread\_variable\_set](Thread.downloaded.ruby_doc#method-i-thread_variable_set)
  # and
  # [thread\_variable\_get](Thread.downloaded.ruby_doc#method-i-thread_variable_get)
  # .
  sig {params(key: T.any(String, Symbol), value: T.untyped).returns(T.untyped)}
  def []=(key, value); end

  sig {returns(T::Boolean)}
  def alive?; end

  # Terminates `thr` and schedules another thread to be run.
  #
  # If this thread is already marked to be killed,
  # [exit](Thread.downloaded.ruby_doc#method-i-exit) returns the
  # [Thread](Thread.downloaded.ruby_doc).
  #
  # If this is the main thread, or the last thread, exits the process.
  sig {returns(T.nilable(Thread))}
  def kill; end

  # Returns the status of the thread-local “abort on exception” condition
  # for this `thr` .
  #
  # The default is `false` .
  #
  # See also
  # [abort\_on\_exception=](Thread.downloaded.ruby_doc#method-i-abort_on_exception-3D)
  # .
  #
  # There is also a class level method to set this for all threads, see
  # [::abort\_on\_exception](Thread.downloaded.ruby_doc#method-c-abort_on_exception)
  # .
  sig {returns(T::Boolean)}
  def abort_on_exception; end

  # When set to `true`, if this `thr` is aborted by an exception, the
  # raised exception will be re-raised in the main thread.
  #
  # See also
  # [abort\_on\_exception](Thread.downloaded.ruby_doc#method-i-abort_on_exception)
  # .
  #
  # There is also a class level method to set this for all threads, see
  # [::abort\_on\_exception=](Thread.downloaded.ruby_doc#method-c-abort_on_exception-3D)
  # .
  sig {params(abort_on_exception: T::Boolean).returns(T.untyped)}
  def abort_on_exception=(abort_on_exception); end

  # Adds *proc* as a handler for tracing.
  #
  # See
  # [\#set\_trace\_func](Thread.downloaded.ruby_doc#method-i-set_trace_func)
  # and
  # [Kernel\#set\_trace\_func](https://ruby-doc.org/core-2.6.3/Kernel.html#method-i-set_trace_func)
  # .
  sig {params(proc: T.untyped).returns(T.untyped)}
  def add_trace_func(proc); end

  # Returns the current backtrace of the target thread.
  sig {params(args: T.untyped).returns(T::Array[T.untyped])}
  def backtrace(*args); end

  # Returns the execution stack for the target thread—an array containing
  # backtrace location objects.
  #
  # See
  # [Thread::Backtrace::Location](https://ruby-doc.org/core-2.6.3/Thread/Backtrace/Location.html)
  # for more information.
  #
  # This method behaves similarly to
  # [Kernel\#caller\_locations](https://ruby-doc.org/core-2.6.3/Kernel.html#method-i-caller_locations)
  # except it applies to a specific thread.
  sig {params(args: T.untyped).returns(T.nilable(T::Array[T.untyped]))}
  def backtrace_locations(*args); end

  # Terminates `thr` and schedules another thread to be run.
  #
  # If this thread is already marked to be killed,
  # [exit](Thread.downloaded.ruby_doc#method-i-exit) returns the
  # [Thread](Thread.downloaded.ruby_doc).
  #
  # If this is the main thread, or the last thread, exits the process.
  sig {returns(T.nilable(Thread))}
  def exit; end

  # Returns a fiber-local for the given key. If the key can’t be found,
  # there are several options: With no other arguments, it will raise a
  # `KeyError` exception; if *default* is given, then that will be returned;
  # if the optional code block is specified, then that will be run and its
  # result returned. See [\#\[\]](Thread.downloaded.ruby_doc#method-i-5B-5D)
  # and
  # [Hash\#fetch](https://ruby-doc.org/core-2.6.3/Hash.html#method-i-fetch)
  # .
  sig {params(sym: T.untyped).returns(T.untyped)}
  def fetch(*sym); end

  sig {returns(T.nilable(ThreadGroup))}
  def group; end

  sig {params(args: T.untyped).returns(Thread)}
  def initialize(*args); end

  # The calling thread will suspend execution and run this `thr` .
  #
  # Does not return until `thr` exits or until the given `limit` seconds
  # have passed.
  #
  # If the time limit expires, `nil` will be returned, otherwise `thr` is
  # returned.
  #
  # Any threads not joined will be killed when the main program exits.
  #
  # If `thr` had previously raised an exception and the
  # [::abort\_on\_exception](Thread.downloaded.ruby_doc#method-c-abort_on_exception)
  # or $DEBUG flags are not set, (so the exception has not yet been
  # processed), it will be processed at this time.
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
  #     tick...
  #     Waiting
  #     tick...
  #     Waiting
  #     tick...
  #     tick...
  sig {params(limit: T.untyped).returns(Thread)}
  def join(*limit); end

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

  sig {returns(T::Array[Symbol])}
  def keys; end

  # show the name of the thread.
  sig {returns(String)}
  def name; end

  # set given name to the ruby thread. On some platform, it may set the name
  # to pthread and/or kernel.
  sig {params(name: T.untyped).returns(T.untyped)}
  def name=(name); end

  # Returns whether or not the asynchronous queue is empty for the target
  # thread.
  #
  # If `error` is given, then check only for `error` type deferred events.
  #
  # See
  # [::pending\_interrupt?](Thread.downloaded.ruby_doc#method-c-pending_interrupt-3F)
  # for more information.
  sig {params(args: T.untyped).returns(T::Boolean)}
  def pending_interrupt?(*args); end

  # Returns the priority of *thr* . Default is inherited from the current
  # thread which creating the new thread, or zero for the initial main
  # thread; higher-priority thread will run more frequently than
  # lower-priority threads (but lower-priority threads can also run).
  #
  # This is just hint for Ruby thread scheduler. It may be ignored on some
  # platform.
  #
  # ```ruby
  # Thread.current.priority   #=> 0
  # ```
  sig {returns(Integer)}
  def priority; end

  # Sets the priority of *thr* to *integer* . Higher-priority threads will
  # run more frequently than lower-priority threads (but lower-priority
  # threads can also run).
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

  # Returns the status of the thread-local “report on exception” condition
  # for this `thr` .
  #
  # The default value when creating a [Thread](Thread.downloaded.ruby_doc)
  # is the value of the global flag
  # [::report\_on\_exception](Thread.downloaded.ruby_doc#method-c-report_on_exception)
  # .
  #
  # See also
  # [report\_on\_exception=](Thread.downloaded.ruby_doc#method-i-report_on_exception-3D)
  # .
  #
  # There is also a class level method to set this for all new threads, see
  # [::report\_on\_exception=](Thread.downloaded.ruby_doc#method-c-report_on_exception-3D)
  # .
  sig {returns(T::Boolean)}
  def report_on_exception; end

  # When set to `true`, a message is printed on $stderr if an exception
  # kills this `thr` . See
  # [::report\_on\_exception](Thread.downloaded.ruby_doc#method-c-report_on_exception)
  # for details.
  #
  # See also
  # [report\_on\_exception](Thread.downloaded.ruby_doc#method-i-report_on_exception)
  # .
  #
  # There is also a class level method to set this for all new threads, see
  # [::report\_on\_exception=](Thread.downloaded.ruby_doc#method-c-report_on_exception-3D)
  # .
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
  # [wakeup](Thread.downloaded.ruby_doc#method-i-wakeup).
  sig {returns(Thread)}
  def run; end

  # Returns the safe level.
  #
  # This method is obsolete because $SAFE is a process global state. Simply
  # check $SAFE.
  sig {returns(Integer)}
  def safe_level; end

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
  # See also [alive?](Thread.downloaded.ruby_doc#method-i-alive-3F) and
  # [status](Thread.downloaded.ruby_doc#method-i-status).
  sig {returns(T::Boolean)}
  def stop?; end

  # Terminates `thr` and schedules another thread to be run.
  #
  # If this thread is already marked to be killed,
  # [exit](Thread.downloaded.ruby_doc#method-i-exit) returns the
  # [Thread](Thread.downloaded.ruby_doc).
  #
  # If this is the main thread, or the last thread, exits the process.
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
  # [\#\[\]](Thread.downloaded.ruby_doc#method-i-5B-5D) and
  # [\#thread\_variable\_get](Thread.downloaded.ruby_doc#method-i-thread_variable_get)
  # for more details.
  sig {params(key: T.any(String, Symbol)).returns(T::Boolean)}
  def thread_variable?(key); end

  # Returns the value of a thread local variable that has been set. Note
  # that these are different than fiber local values. For fiber local
  # values, please see [\#\[\]](Thread.downloaded.ruby_doc#method-i-5B-5D)
  # and [\#\[\]=](Thread.downloaded.ruby_doc#method-i-5B-5D-3D).
  #
  # [Thread](Thread.downloaded.ruby_doc) local values are carried along with
  # threads, and do not respect fibers. For example:
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
  # The value “bar” is returned for the thread local, where nil is returned
  # for the fiber local. The fiber is executed in the same thread, so the
  # thread local values are available.
  sig {params(key: T.untyped).returns(T.untyped)}
  def thread_variable_get(key); end

  # Sets a thread local with `key` to `value` . Note that these are local to
  # threads, and not to fibers. Please see
  # [\#thread\_variable\_get](Thread.downloaded.ruby_doc#method-i-thread_variable_get)
  # and [\#\[\]](Thread.downloaded.ruby_doc#method-i-5B-5D) for more
  # information.
  sig {params(key: T.untyped, value: T.untyped).returns(T.untyped)}
  def thread_variable_set(key, value); end

  sig {returns(T::Array[Symbol])}
  def thread_variables; end

  # Waits for `thr` to complete, using
  # [join](Thread.downloaded.ruby_doc#method-i-join), and returns its value
  # or raises the exception which terminated the thread.
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

  # Marks a given thread as eligible for scheduling, however it may still
  # remain blocked on I/O.
  #
  # **Note:** This does not invoke the scheduler, see
  # [run](Thread.downloaded.ruby_doc#method-i-run) for more information.
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

  # Returns the status of the global “abort on exception” condition.
  #
  # The default is `false` .
  #
  # When set to `true`, if any thread is aborted by an exception, the
  # raised exception will be re-raised in the main thread.
  #
  # Can also be specified by the global $DEBUG flag or command line option
  # `-d` .
  #
  # See also
  # [::abort\_on\_exception=](Thread.downloaded.ruby_doc#method-c-abort_on_exception-3D)
  # .
  #
  # There is also an instance level method to set this for a specific
  # thread, see
  # [abort\_on\_exception](Thread.downloaded.ruby_doc#method-i-abort_on_exception)
  # .
  sig {returns(T.untyped)}
  def self.abort_on_exception; end

  # When set to `true`, if any thread is aborted by an exception, the
  # raised exception will be re-raised in the main thread. Returns the new
  # state.
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
  #     In new thread
  #     prog.rb:4: Exception from thread (RuntimeError)
  #      from prog.rb:2:in `initialize'
  #      from prog.rb:2:in `new'
  #      from prog.rb:2
  #
  # See also
  # [::abort\_on\_exception](Thread.downloaded.ruby_doc#method-c-abort_on_exception)
  # .
  #
  # There is also an instance level method to set this for a specific
  # thread, see
  # [abort\_on\_exception=](Thread.downloaded.ruby_doc#method-i-abort_on_exception-3D)
  # .
  sig {params(abort_on_exception: T.untyped).returns(T.untyped)}
  def self.abort_on_exception=(abort_on_exception); end

  # Wraps the block in a single, VM-global
  # [Mutex\#synchronize](https://ruby-doc.org/core-2.6.3/Mutex.html#method-i-synchronize)
  # , returning the value of the block. A thread executing inside the
  # exclusive section will only block other threads which also use the
  # [::exclusive](Thread.downloaded.ruby_doc#method-c-exclusive) mechanism.
  sig {params(block: T.untyped).returns(T.untyped)}
  def self.exclusive(&block); end

  # Terminates the currently running thread and schedules another thread to
  # be run.
  #
  # If this thread is already marked to be killed,
  # [::exit](Thread.downloaded.ruby_doc#method-c-exit) returns the
  # [Thread](Thread.downloaded.ruby_doc).
  #
  # If this is the main thread, or the last thread, exit the process.
  sig {returns(T.untyped)}
  def self.exit; end

  # Basically the same as [::new](Thread.downloaded.ruby_doc#method-c-new).
  # However, if class [Thread](Thread.downloaded.ruby_doc) is subclassed,
  # then calling `start` in that subclass will not invoke the subclass’s
  # `initialize` method.
  sig {params(args: T.untyped).returns(T.untyped)}
  def self.fork(*args); end

  # Changes asynchronous interrupt timing.
  #
  # *interrupt* means asynchronous event and corresponding procedure by
  # [\#raise](Thread.downloaded.ruby_doc#method-i-raise),
  # [\#kill](Thread.downloaded.ruby_doc#method-i-kill), signal trap (not
  # supported yet) and main thread termination (if main thread terminates,
  # then all other thread will be killed).
  #
  # The given `hash` has pairs like `ExceptionClass => :TimingSymbol` .
  # Where the ExceptionClass is the interrupt handled by the given block.
  # The TimingSymbol can be one of the following symbols:
  #
  #   - `:immediate`
  #     Invoke interrupts immediately.
  #
  #   - `:on_blocking`
  #     Invoke interrupts while *BlockingOperation* .
  #
  #   - `:never`
  #     Never invoke all interrupts.
  #
  # *BlockingOperation* means that the operation will block the calling
  # thread, such as read and write. On CRuby implementation,
  # *BlockingOperation* is any operation executed without GVL.
  #
  # Masked asynchronous interrupts are delayed until they are enabled. This
  # method is similar to sigprocmask(3).
  #
  #
  # Asynchronous interrupts are difficult to use.
  #
  # If you need to communicate between threads, please consider to use
  # another way such as [Queue](https://ruby-doc.org/core-2.6.3/Queue.html)
  # .
  #
  # Or use them with deep understanding about this method.
  #
  #
  # In this example, we can guard from
  # [\#raise](Thread.downloaded.ruby_doc#method-i-raise) exceptions.
  #
  # Using the `:never` TimingSymbol the
  # [RuntimeError](https://ruby-doc.org/core-2.6.3/RuntimeError.html)
  # exception will always be ignored in the first block of the main thread.
  # In the second
  # [::handle\_interrupt](Thread.downloaded.ruby_doc#method-c-handle_interrupt)
  # block we can purposefully handle
  # [RuntimeError](https://ruby-doc.org/core-2.6.3/RuntimeError.html)
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
  # [RuntimeError](https://ruby-doc.org/core-2.6.3/RuntimeError.html)
  # exception, it’s safe to write our resource allocation code. Then, the
  # ensure block is where we can safely deallocate your resources.
  #
  #
  # In the next example, we will guard from the Timeout::Error exception.
  # This will help prevent from leaking resources when Timeout::Error
  # exceptions occur during normal ensure clause. For this example we use
  # the help of the standard library Timeout, from lib/timeout.rb
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
  # In the first part of the `timeout` block, we can rely on Timeout::Error
  # being ignored. Then in the `Timeout::Error => :on_blocking` block, any
  # operation that will block the calling thread is susceptible to a
  # Timeout::Error exception being raised.
  #
  #
  # It’s possible to stack multiple levels of
  # [::handle\_interrupt](Thread.downloaded.ruby_doc#method-c-handle_interrupt)
  # blocks in order to control more than one ExceptionClass and TimingSymbol
  # at a time.
  #
  # ```ruby
  # Thread.handle_interrupt(FooError => :never) {
  #   Thread.handle_interrupt(BarError => :never) {
  #      # FooError and BarError are prohibited.
  #   }
  # }
  # ```
  #
  #
  # All exceptions inherited from the ExceptionClass parameter will be
  # considered.
  #
  # ```ruby
  # Thread.handle_interrupt(Exception => :never) {
  #   # all exceptions inherited from Exception are prohibited.
  # }
  # ```
  sig {params(hash: T.untyped).returns(T.untyped)}
  def self.handle_interrupt(hash); end

  sig {params(thread: Thread).returns(T.untyped)}
  def self.kill(thread); end

  sig {returns(T.untyped)}
  def self.list; end

  # Give the thread scheduler a hint to pass execution to another thread. A
  # running thread may or may not switch, it depends on OS and processor.
  sig {returns(T.untyped)}
  def self.pass; end

  # Returns whether or not the asynchronous queue is empty.
  #
  # Since
  # [::handle\_interrupt](Thread.downloaded.ruby_doc#method-c-handle_interrupt)
  # can be used to defer asynchronous events, this method can be used to
  # determine if there are any deferred events.
  #
  # If you find this method returns true, then you may finish `:never`
  # blocks.
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
  #
  #     th = Thread.new{
  #       Thread.handle_interrupt(RuntimeError => :on_blocking){
  #         while true
  #           ...
  #           # reach safe point to invoke interrupt
  #           if Thread.pending_interrupt?
  #             Thread.handle_interrupt(Object => :immediate){}
  #           end
  #           ...
  #         end
  #       }
  #     }
  #     ...
  #     th.raise # stop thread
  #
  # This example can also be written as the following, which you should use
  # to avoid asynchronous interrupts.
  #
  #     flag = true
  #     th = Thread.new{
  #       Thread.handle_interrupt(RuntimeError => :on_blocking){
  #         while true
  #           ...
  #           # reach safe point to invoke interrupt
  #           break if flag == false
  #           ...
  #         end
  #       }
  #     }
  #     ...
  #     flag = false # stop thread
  sig {params(args: T.untyped).returns(T::Boolean)}
  def self.pending_interrupt?(*args); end

  sig {returns(T.untyped)}
  def self.report_on_exception; end

  sig {params(report_on_exception: T.untyped).returns(T.untyped)}
  def self.report_on_exception=(report_on_exception); end

  # Basically the same as [::new](Thread.downloaded.ruby_doc#method-c-new).
  # However, if class [Thread](Thread.downloaded.ruby_doc) is subclassed,
  # then calling `start` in that subclass will not invoke the subclass’s
  # `initialize` method.
  sig {params(args: T.untyped).returns(T.untyped)}
  def self.start(*args); end

  # Stops execution of the current thread, putting it into a “sleep” state,
  # and schedules execution of another thread.
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
end

class Thread::Backtrace < Object
end

class Thread::Backtrace::Location
  sig {returns(T.nilable(String))}
  def absolute_path(); end

  sig {returns(T.nilable(String))}
  def base_label(); end

  sig {returns(T.nilable(String))}
  def label(); end

  sig {returns(Integer)}
  def lineno(); end

  sig {returns(T.nilable(String))}
  def path(); end
end

# [ConditionVariable](ConditionVariable) objects
# augment class [Mutex](https://ruby-doc.org/core-2.6.3/Mutex.html).
# Using condition variables, it is possible to suspend while in the middle
# of a critical section until a resource becomes available.
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

  # Releases the lock held in `mutex` and waits; reacquires the lock on
  # wakeup.
  #
  # If `timeout` is given, this method returns after `timeout` seconds
  # passed, even if no other thread doesn't signal.
  sig {params(_: T.untyped).returns(T.untyped)}
  def wait(*_); end
end

# [Mutex](Mutex) implements a simple semaphore that
# can be used to coordinate access to shared data from multiple concurrent
# threads.
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
  # Attempts to grab the lock and waits if it isn’t available. Raises
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
  # completes. See the example under `Mutex` .
  sig {returns(T.untyped)}
  def synchronize; end

  # Attempts to obtain the lock and returns immediately. Returns `true` if
  # the lock was granted.
  sig {returns(T::Boolean)}
  def try_lock; end

  # Releases the lock. Raises `ThreadError` if `mutex` wasn’t locked by the
  # current thread.
  sig {returns(T.untyped)}
  def unlock; end
end

# The [Queue](Queue) class implements multi-producer,
# multi-consumer queues. It is especially useful in threaded programming
# when information must be exchanged safely between multiple threads. The
# [Queue](Queue) class implements all the required
# locking semantics.
#
# The class implements FIFO type of queue. In a FIFO queue, the first
# tasks added are the first retrieved.
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
  # Alias for: [push](Queue.downloaded.ruby_doc#method-i-push)
  sig {params(obj: T.untyped).returns(T.untyped)}
  def <<(obj); end

  # Removes all objects from the queue.
  sig {returns(T.untyped)}
  def clear; end

  # Closes the queue. A closed queue cannot be re-opened.
  #
  # After the call to close completes, the following are true:
  #
  #   - `closed?` will return true
  #
  #   - `close` will be ignored.
  #
  #   - calling enq/push/\<\< will raise a `ClosedQueueError` .
  #
  #   - when `empty?` is false, calling deq/pop/shift will return an object
  #     from the queue as usual.
  #
  #   - when `empty?` is true, deq(false) will not suspend the thread and
  #     will return nil. deq(true) will raise a `ThreadError` .
  #
  # [ClosedQueueError](https://ruby-doc.org/core-2.6.3/ClosedQueueError.html)
  # is inherited from
  # [StopIteration](https://ruby-doc.org/core-2.6.3/StopIteration.html), so
  # that you can break loop block.
  #
  #     Example:
  #
  #         q = Queue.new
  #         Thread.new{
  #           while e = q.deq # wait for nil to break loop
  #             # ...
  #           end
  #         }
  #         q.close
  sig {returns(T.untyped)}
  def close; end

  # Returns `true` if the queue is closed.
  sig {returns(T::Boolean)}
  def closed?; end

  # Alias for: [pop](Queue.downloaded.ruby_doc#method-i-pop)
  sig {params(args: T.untyped).returns(T.untyped)}
  def deq(*args); end

  # Returns `true` if the queue is empty.
  sig {returns(T::Boolean)}
  def empty?; end

  # Alias for: [push](Queue.downloaded.ruby_doc#method-i-push)
  sig {params(obj: T.untyped).returns(T.untyped)}
  def enq(obj); end

  # Returns the length of the queue.
  #
  #
  #
  # Also aliased as: [size](Queue.downloaded.ruby_doc#method-i-size)
  sig {returns(Integer)}
  def length; end

  sig {returns(T.untyped)}
  def marshal_dump; end

  # Returns the number of threads waiting on the queue.
  sig {returns(T.untyped)}
  def num_waiting; end

  # Retrieves data from the queue.
  #
  # If the queue is empty, the calling thread is suspended until data is
  # pushed onto the queue. If `non_block` is true, the thread isn't
  # suspended, and `ThreadError` is raised.
  #
  #
  #
  # Also aliased as: [deq](Queue.downloaded.ruby_doc#method-i-deq),
  # [shift](Queue.downloaded.ruby_doc#method-i-shift)
  sig {params(args: T.untyped).returns(T.untyped)}
  def pop(*args); end

  # Pushes the given `object` to the queue.
  #
  #
  #
  # Also aliased as: [enq](Queue.downloaded.ruby_doc#method-i-enq),
  # [\<\<](Queue.downloaded.ruby_doc#method-i-3C-3C)
  sig {params(obj: T.untyped).returns(T.untyped)}
  def push(obj); end

  # Alias for: [pop](Queue.downloaded.ruby_doc#method-i-pop)
  sig {params(args: T.untyped).returns(T.untyped)}
  def shift(*args); end

  # Alias for: [length](Queue.downloaded.ruby_doc#method-i-length)
  sig {returns(Integer)}
  def size; end
end

# This class represents queues of specified size capacity. The push
# operation may be blocked if the capacity is full.
#
# See [Queue](https://ruby-doc.org/core-2.6.3/Queue.html) for an example
# of how a [SizedQueue](SizedQueue) works.
class Thread::SizedQueue < Thread::Queue
  # Alias for: [push](SizedQueue.downloaded.ruby_doc#method-i-push)
  sig {params(args: T.untyped).returns(T.untyped)}
  def <<(*args); end

  # Alias for: [push](SizedQueue.downloaded.ruby_doc#method-i-push)
  sig {params(args: T.untyped).returns(T.untyped)}
  def enq(*args); end

  sig {params(max: T.untyped).returns(SizedQueue)}
  def initialize(max); end

  # Returns the maximum size of the queue.
  sig {returns(Integer)}
  def max; end

  # Sets the maximum size of the queue to the given `number` .
  sig {params(max: Integer).returns(T.untyped)}
  def max=(max); end

  # Pushes `object` to the queue.
  #
  # If there is no space left in the queue, waits until space becomes
  # available, unless `non_block` is true. If `non_block` is true, the
  # thread isn't suspended, and `ThreadError` is raised.
  #
  #
  #
  # Also aliased as: [enq](SizedQueue.downloaded.ruby_doc#method-i-enq),
  # [\<\<](SizedQueue.downloaded.ruby_doc#method-i-3C-3C)
  sig {params(args: T.untyped).returns(T.untyped)}
  def push(*args); end
end

ConditionVariable = Thread::ConditionVariable
Mutex = Thread::Mutex
Queue = Thread::Queue
SizedQueue = Thread::SizedQueue
