# typed: __STDLIB_INTERNAL

# Use the [`Monitor`](https://docs.ruby-lang.org/en/2.6.0/Monitor.html) class
# when you want to have a lock object for blocks with mutual exclusion.
#
# ```ruby
# require 'monitor'
#
# lock = Monitor.new
# lock.synchronize do
#   # exclusive access
# end
# ```
class Monitor < Object
  include MonitorMixin
end

# In concurrent programming, a monitor is an object or module intended to be
# used safely by more than one thread. The defining characteristic of a monitor
# is that its methods are executed with mutual exclusion. That is, at each point
# in time, at most one thread may be executing any of its methods. This mutual
# exclusion greatly simplifies reasoning about the implementation of monitors
# compared to reasoning about parallel code that updates a data structure.
#
# You can read more about the general principles on the Wikipedia page for
# [Monitors](http://en.wikipedia.org/wiki/Monitor_%28synchronization%29)
#
# ## Examples
#
# ### Simple object.extend
#
# ```ruby
# require 'monitor.rb'
#
# buf = []
# buf.extend(MonitorMixin)
# empty_cond = buf.new_cond
#
# # consumer
# Thread.start do
#   loop do
#     buf.synchronize do
#       empty_cond.wait_while { buf.empty? }
#       print buf.shift
#     end
#   end
# end
#
# # producer
# while line = ARGF.gets
#   buf.synchronize do
#     buf.push(line)
#     empty_cond.signal
#   end
# end
# ```
#
# The consumer thread waits for the producer thread to push a line to buf while
# `buf.empty?`. The producer thread (main thread) reads a line from
# [`ARGF`](https://docs.ruby-lang.org/en/2.6.0/ARGF.html) and pushes it into buf
# then calls `empty_cond.signal` to notify the consumer thread of new data.
#
# ### Simple [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) include
#
# ```ruby
# require 'monitor'
#
# class SynchronizedArray < Array
#
#   include MonitorMixin
#
#   def initialize(*args)
#     super(*args)
#   end
#
#   alias :old_shift :shift
#   alias :old_unshift :unshift
#
#   def shift(n=1)
#     self.synchronize do
#       self.old_shift(n)
#     end
#   end
#
#   def unshift(item)
#     self.synchronize do
#       self.old_unshift(item)
#     end
#   end
#
#   # other methods ...
# end
# ```
#
# `SynchronizedArray` implements an
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) with synchronized
# access to items. This
# [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) is implemented as
# subclass of [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) which
# includes the
# [`MonitorMixin`](https://docs.ruby-lang.org/en/2.6.0/MonitorMixin.html)
# module.
module MonitorMixin
end

# FIXME: This isn't documented in Nutshell.
#
# Since
# [`MonitorMixin.new_cond`](https://docs.ruby-lang.org/en/2.6.0/MonitorMixin.html#method-i-new_cond)
# returns a
# [`ConditionVariable`](https://docs.ruby-lang.org/en/2.6.0/MonitorMixin/ConditionVariable.html),
# and the example above calls while\_wait and signal, this class should be
# documented.
class MonitorMixin::ConditionVariable < Object
end

class MonitorMixin::ConditionVariable::Timeout < Exception
end
