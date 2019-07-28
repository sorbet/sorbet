# typed: __STDLIB_INTERNAL

class Thread < Object
  sig {returns(Thread)}
  def self.current; end

  sig {returns(Thread)}
  def self.main; end

  sig {params(key: T.any(String, Symbol)).returns(T.untyped)}
  def [](key); end

  sig {params(key: T.any(String, Symbol), value: T.untyped).returns(T.untyped)}
  def []=(key, value); end

  sig {returns(T::Boolean)}
  def alive?; end

  sig {returns(T.nilable(Thread))}
  def kill; end
  
  sig {returns(T::Boolean)}
  def abort_on_exception; end

  sig {params(abort_on_exception: T::Boolean).returns(T.untyped)}
  def abort_on_exception=(abort_on_exception); end

  sig {params(proc: T.untyped).returns(T.untyped)}
  def add_trace_func(proc); end

  sig {params(args: T.untyped).returns(T::Array[T.untyped])}
  def backtrace(*args); end

  sig {params(args: T.untyped).returns(T.nilable(T::Array[T.untyped]))}
  def backtrace_locations(*args); end

  sig {returns(T.nilable(Thread))}
  def exit; end

  sig {params(sym: T.untyped).returns(T.untyped)}
  def fetch(*sym); end

  sig {returns(T.nilable(ThreadGroup))}
  def group; end

  sig {params(args: T.untyped).returns(Thread)}
  def initialize(*args); end

  sig {params(limit: T.untyped).returns(Thread)}
  def join(*limit); end

  sig {params(sym: Symbol).returns(T::Boolean)}
  def key?(sym); end

  sig {returns(T::Array[Symbol])}
  def keys; end

  sig {returns(String)}
  def name; end

  sig {params(name: T.untyped).returns(T.untyped)}
  def name=(name); end

  sig {params(args: T.untyped).returns(T::Boolean)}
  def pending_interrupt?(*args); end

  sig {returns(Integer)}
  def priority; end

  sig {params(priority: Integer).returns(T.untyped)}
  def priority=(priority); end

  sig {returns(T::Boolean)}
  def report_on_exception; end

  sig {params(report_on_exception: T::Boolean).returns(T.untyped)}
  def report_on_exception=(report_on_exception); end

  sig {returns(Thread)}
  def run; end

  sig {returns(Integer)}
  def safe_level; end

  sig {returns(T.nilable(T.any(String, T::Boolean)))}
  def status; end

  sig {returns(T::Boolean)}
  def stop?; end

  sig {returns(T.nilable(Thread))}
  def terminate; end

  sig {params(key: T.any(String, Symbol)).returns(T::Boolean)}
  def thread_variable?(key); end

  sig {params(key: T.untyped).returns(T.untyped)}
  def thread_variable_get(key); end

  sig {params(key: T.untyped, value: T.untyped).returns(T.untyped)}
  def thread_variable_set(key, value); end

  sig {returns(T::Array[Symbol])}
  def thread_variables; end

  sig {returns(Object)}
  def value; end

  sig {returns(Thread)}
  def wakeup; end

  sig {returns(T.untyped)}
  def self.abort_on_exception; end

  sig {params(abort_on_exception: T.untyped).returns(T.untyped)}
  def self.abort_on_exception=(abort_on_exception); end

  sig {params(block: T.untyped).returns(T.untyped)}
  def self.exclusive(&block); end

  sig {returns(T.untyped)}
  def self.exit; end

  sig {params(args: T.untyped).returns(T.untyped)}
  def self.fork(*args); end

  sig {params(hash: T.untyped).returns(T.untyped)}
  def self.handle_interrupt(hash); end

  sig {params(thread: Thread).returns(T.untyped)}
  def self.kill(thread); end

  sig {returns(T.untyped)}
  def self.list; end

  sig {returns(T.untyped)}
  def self.pass; end

  sig {params(args: T.untyped).returns(T::Boolean)}
  def self.pending_interrupt?(*args); end

  sig {returns(T.untyped)}
  def self.report_on_exception; end

  sig {params(report_on_exception: T.untyped).returns(T.untyped)}
  def self.report_on_exception=(report_on_exception); end

  sig {params(args: T.untyped).returns(T.untyped)}
  def self.start(*args); end

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

class Thread::ConditionVariable < Object
  sig {returns(T.untyped)}
  def broadcast; end

  sig {returns(T.untyped)}
  def marshal_dump; end

  sig {returns(T.untyped)}
  def signal; end

  sig {params(_: T.untyped).returns(T.untyped)}
  def wait(*_); end
end

class Thread::Mutex < Object
  sig {returns(T.untyped)}
  def lock; end

  sig {returns(T::Boolean)}
  def locked?; end

  sig {returns(T::Boolean)}
  def owned?; end

  sig {returns(T.untyped)}
  def synchronize; end

  sig {returns(T::Boolean)}
  def try_lock; end

  sig {returns(T.untyped)}
  def unlock; end
end

class Thread::Queue < Object
  sig {params(obj: T.untyped).returns(T.untyped)}
  def <<(obj); end

  sig {returns(T.untyped)}
  def clear; end

  sig {returns(T.untyped)}
  def close; end

  sig {returns(T::Boolean)}
  def closed?; end

  sig {params(args: T.untyped).returns(T.untyped)}
  def deq(*args); end

  sig {returns(T::Boolean)}
  def empty?; end

  sig {params(obj: T.untyped).returns(T.untyped)}
  def enq(obj); end

  sig {returns(Integer)}
  def length; end

  sig {returns(T.untyped)}
  def marshal_dump; end

  sig {returns(T.untyped)}
  def num_waiting; end

  sig {params(args: T.untyped).returns(T.untyped)}
  def pop(*args); end

  sig {params(obj: T.untyped).returns(T.untyped)}
  def push(obj); end

  sig {params(args: T.untyped).returns(T.untyped)}
  def shift(*args); end

  sig {returns(Integer)}
  def size; end
end

class Thread::SizedQueue < Thread::Queue
  sig {params(obj: T.untyped).returns(T.untyped)}
  def <<(*args); end

  sig {params(args: T.untyped).returns(T.untyped)}
  def enq(*args); end

  sig {params(max: T.untyped).returns(SizedQueue)}
  def initialize(max); end

  sig {returns(Integer)}
  def max; end

  sig {params(max: Integer).returns(T.untyped)}
  def max=(max); end

  sig {params(args: T.untyped).returns(T.untyped)}
  def push(*args); end
end

ConditionVariable = Thread::ConditionVariable
Mutex = Thread::Mutex
Queue = Thread::Queue
SizedQueue = Thread::SizedQueue
