# typed: __STDLIB_INTERNAL

class ThreadGroup < Object
  Default = T.let(T.unsafe(nil), ThreadGroup)

  sig {params(thread: Thread).returns(ThreadGroup)}
  def add(thread); end

  sig {returns(ThreadGroup)}
  def enclose; end

  sig {returns(T::Boolean)}
  def enclosed?; end

  sig {returns(T::Array[Thread])}
  def list; end
end
