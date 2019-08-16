# typed: __STDLIB_INTERNAL

# [ThreadGroup](ThreadGroup) provides a means of
# keeping track of a number of threads as a group.
#
# A given [Thread](https://ruby-doc.org/core-2.6.3/Thread.html) object can
# only belong to one [ThreadGroup](ThreadGroup) at a
# time; adding a thread to a new group will remove it from any previous
# group.
#
# Newly created threads belong to the same group as the thread from which
# they were created.
class ThreadGroup < Object
  Default = T.let(T.unsafe(nil), ThreadGroup)

  sig {params(thread: Thread).returns(ThreadGroup)}
  def add(thread); end

  sig {returns(ThreadGroup)}
  def enclose; end

  # Returns `true` if the `thgrp` is enclosed. See also
  # [\#enclose](ThreadGroup.downloaded.ruby_doc#method-i-enclose).
  sig {returns(T::Boolean)}
  def enclosed?; end

  sig {returns(T::Array[Thread])}
  def list; end
end
