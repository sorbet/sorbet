# typed: __STDLIB_INTERNAL

# [`ThreadGroup`](https://docs.ruby-lang.org/en/2.6.0/ThreadGroup.html) provides
# a means of keeping track of a number of threads as a group.
#
# A given [`Thread`](https://docs.ruby-lang.org/en/2.6.0/Thread.html) object can
# only belong to one
# [`ThreadGroup`](https://docs.ruby-lang.org/en/2.6.0/ThreadGroup.html) at a
# time; adding a thread to a new group will remove it from any previous group.
#
# Newly created threads belong to the same group as the thread from which they
# were created.
class ThreadGroup < Object
  # The default
  # [`ThreadGroup`](https://docs.ruby-lang.org/en/2.6.0/ThreadGroup.html)
  # created when Ruby starts; all Threads belong to it by default.
  Default = T.let(T.unsafe(nil), ThreadGroup)

  # Adds the given `thread` to this group, removing it from any other group to
  # which it may have previously been a member.
  #
  # ```ruby
  # puts "Initial group is #{ThreadGroup::Default.list}"
  # tg = ThreadGroup.new
  # t1 = Thread.new { sleep }
  # t2 = Thread.new { sleep }
  # puts "t1 is #{t1}"
  # puts "t2 is #{t2}"
  # tg.add(t1)
  # puts "Initial group now #{ThreadGroup::Default.list}"
  # puts "tg group now #{tg.list}"
  # ```
  #
  # This will produce:
  #
  # ```ruby
  # Initial group is #<Thread:0x401bdf4c>
  # t1 is #<Thread:0x401b3c90>
  # t2 is #<Thread:0x401b3c18>
  # Initial group now #<Thread:0x401b3c18>#<Thread:0x401bdf4c>
  # tg group now #<Thread:0x401b3c90>
  # ```
  sig {params(thread: Thread).returns(ThreadGroup)}
  def add(thread); end

  # Prevents threads from being added to or removed from the receiving
  # [`ThreadGroup`](https://docs.ruby-lang.org/en/2.6.0/ThreadGroup.html).
  #
  # New threads can still be started in an enclosed
  # [`ThreadGroup`](https://docs.ruby-lang.org/en/2.6.0/ThreadGroup.html).
  #
  # ```ruby
  # ThreadGroup::Default.enclose        #=> #<ThreadGroup:0x4029d914>
  # thr = Thread.new { Thread.stop }    #=> #<Thread:0x402a7210 sleep>
  # tg = ThreadGroup.new                #=> #<ThreadGroup:0x402752d4>
  # tg.add thr
  # #=> ThreadError: can't move from the enclosed thread group
  # ```
  sig {returns(ThreadGroup)}
  def enclose; end

  # Returns `true` if the `thgrp` is enclosed. See also
  # [`ThreadGroup#enclose`](https://docs.ruby-lang.org/en/2.6.0/ThreadGroup.html#method-i-enclose).
  sig {returns(T::Boolean)}
  def enclosed?; end

  # Returns an array of all existing
  # [`Thread`](https://docs.ruby-lang.org/en/2.6.0/Thread.html) objects that
  # belong to this group.
  #
  # ```ruby
  # ThreadGroup::Default.list   #=> [#<Thread:0x401bdf4c run>]
  # ```
  sig {returns(T::Array[Thread])}
  def list; end
end
