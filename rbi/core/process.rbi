# typed: true
module Process
  CLOCK_BOOTTIME = T.let(T.unsafe(nil), Integer)
  CLOCK_BOOTTIME_ALARM = T.let(T.unsafe(nil), Integer)
  CLOCK_MONOTONIC = T.let(T.unsafe(nil), Integer)
  CLOCK_MONOTONIC_COARSE = T.let(T.unsafe(nil), Integer)
  CLOCK_MONOTONIC_RAW = T.let(T.unsafe(nil), Integer)
  CLOCK_PROCESS_CPUTIME_ID = T.let(T.unsafe(nil), Integer)
  CLOCK_REALTIME = T.let(T.unsafe(nil), Integer)
  CLOCK_REALTIME_ALARM = T.let(T.unsafe(nil), Integer)
  CLOCK_REALTIME_COARSE = T.let(T.unsafe(nil), Integer)
  CLOCK_THREAD_CPUTIME_ID = T.let(T.unsafe(nil), Integer)
  PRIO_PGRP = T.let(T.unsafe(nil), Integer)
  PRIO_PROCESS = T.let(T.unsafe(nil), Integer)
  PRIO_USER = T.let(T.unsafe(nil), Integer)
  RLIMIT_AS = T.let(T.unsafe(nil), Integer)
  RLIMIT_CORE = T.let(T.unsafe(nil), Integer)
  RLIMIT_CPU = T.let(T.unsafe(nil), Integer)
  RLIMIT_DATA = T.let(T.unsafe(nil), Integer)
  RLIMIT_FSIZE = T.let(T.unsafe(nil), Integer)
  RLIMIT_MEMLOCK = T.let(T.unsafe(nil), Integer)
  RLIMIT_MSGQUEUE = T.let(T.unsafe(nil), Integer)
  RLIMIT_NICE = T.let(T.unsafe(nil), Integer)
  RLIMIT_NOFILE = T.let(T.unsafe(nil), Integer)
  RLIMIT_NPROC = T.let(T.unsafe(nil), Integer)
  RLIMIT_RSS = T.let(T.unsafe(nil), Integer)
  RLIMIT_RTPRIO = T.let(T.unsafe(nil), Integer)
  RLIMIT_RTTIME = T.let(T.unsafe(nil), Integer)
  RLIMIT_SIGPENDING = T.let(T.unsafe(nil), Integer)
  RLIMIT_STACK = T.let(T.unsafe(nil), Integer)
  RLIM_INFINITY = T.let(T.unsafe(nil), Integer)
  RLIM_SAVED_CUR = T.let(T.unsafe(nil), Integer)
  RLIM_SAVED_MAX = T.let(T.unsafe(nil), Integer)
  WNOHANG = T.let(T.unsafe(nil), Integer)
  WUNTRACED = T.let(T.unsafe(nil), Integer)

  sig(
      msg: String,
  )
  .returns(T.untyped)
  def self.abort(msg=_); end

  sig.returns(String)
  def self.argv0(); end

  sig(
      clock_id: T.any(Symbol, Integer),
      unit: Symbol,
  )
  .returns(T.any(Float, Integer))
  def self.clock_getres(clock_id, unit=_); end

  sig(
      clock_id: T.any(Symbol, Integer),
      unit: Symbol,
  )
  .returns(T.any(Float, Integer))
  def self.clock_gettime(clock_id, unit=_); end

  sig(
      nochdir: BasicObject,
      noclose: BasicObject,
  )
  .returns(Integer)
  def self.daemon(nochdir=_, noclose=_); end

  sig(
      pid: Integer,
  )
  .returns(Thread)
  def self.detach(pid); end

  sig.returns(Integer)
  def self.egid(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.egid=(arg0); end

  sig.returns(Integer)
  def self.euid(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.euid=(arg0); end

  sig(
      status: Integer,
  )
  .returns(T.untyped)
  def self.exit(status=_); end

  sig(
      status: Integer,
  )
  .returns(T.untyped)
  def self.exit!(status=_); end

  sig.returns(T.nilable(Integer))
  sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(T.nilable(Integer))
  def self.fork(&blk); end

  sig(
      pid: Integer,
  )
  .returns(Integer)
  def self.getpgid(pid); end

  sig.returns(Integer)
  def self.getpgrp(); end

  sig(
      kind: Integer,
      arg0: Integer,
  )
  .returns(Integer)
  def self.getpriority(kind, arg0); end

  sig(
      resource: T.any(Symbol, String, Integer),
  )
  .returns([Integer, Integer])
  def self.getrlimit(resource); end

  sig(
      pid: Integer,
  )
  .returns(Integer)
  def self.getsid(pid=_); end

  sig.returns(Integer)
  def self.gid(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.gid=(arg0); end

  sig.returns(T::Array[Integer])
  def self.groups(); end

  sig(
      arg0: T::Array[Integer],
  )
  .returns(T::Array[Integer])
  def self.groups=(arg0); end

  sig(
      username: String,
      gid: Integer,
  )
  .returns(T::Array[Integer])
  def self.initgroups(username, gid); end

  sig(
      signal: T.any(Integer, Symbol, String),
      pids: Integer,
  )
  .returns(Integer)
  def self.kill(signal, *pids); end

  sig.returns(Integer)
  def self.maxgroups(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.maxgroups=(arg0); end

  sig.returns(Integer)
  def self.pid(); end

  sig.returns(Integer)
  def self.ppid(); end

  sig(
      pid: Integer,
      arg0: Integer,
  )
  .returns(Integer)
  def self.setpgid(pid, arg0); end

  sig(
      kind: Integer,
      arg0: Integer,
      priority: Integer,
  )
  .returns(Integer)
  def self.setpriority(kind, arg0, priority); end

  sig(
      arg0: String,
  )
  .returns(String)
  def self.setproctitle(arg0); end

  sig(
      resource: T.any(Symbol, String, Integer),
      cur_limit: Integer,
      max_limit: Integer,
  )
  .returns(NilClass)
  def self.setrlimit(resource, cur_limit, max_limit=_); end

  sig.returns(Integer)
  def self.setsid(); end

  sig.returns(Process::Tms)
  def self.times(); end

  sig.returns(Integer)
  def self.uid(); end

  sig(
      user: Integer,
  )
  .returns(Integer)
  def self.uid=(user); end

  sig(
      pid: Integer,
      flags: Integer,
  )
  .returns(Integer)
  def self.wait(pid=_, flags=_); end

  sig(
      pid: Integer,
      flags: Integer,
  )
  .returns([Integer, Integer])
  def self.wait2(pid=_, flags=_); end

  sig.returns(T::Array[[Integer, Integer]])
  def self.waitall(); end

  sig(
      pid: Integer,
      flags: Integer,
  )
  .returns(Integer)
  def self.waitpid(pid=_, flags=_); end

  sig(
      pid: Integer,
      flags: Integer,
  )
  .returns([Integer, Integer])
  def self.waitpid2(pid=_, flags=_); end
end

module Process::GID
  sig(
      group: Integer,
  )
  .returns(Integer)
  def self.change_privilege(group); end

  sig.returns(Integer)
  def self.eid(); end

  sig(
      name: String,
  )
  .returns(Integer)
  def self.from_name(name); end

  sig(
      group: Integer,
  )
  .returns(Integer)
  def self.grant_privilege(group); end

  sig.returns(Integer)
  def self.re_exchange(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.re_exchangeable?(); end

  sig.returns(Integer)
  def self.rid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.sid_available?(); end

  sig.returns(Integer)
  type_parameters(:T).sig(
      blk: T.proc().returns(T.type_parameter(:T)),
  )
  .returns(T.type_parameter(:T))
  def self.switch(&blk); end

  sig(
      group: Integer,
  )
  .returns(Integer)
  def self.eid=(group); end
end

class Process::Status < Object
  sig(
      num: Integer,
  )
  .returns(Integer)
  def &(num); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  sig(
      num: Integer,
  )
  .returns(Integer)
  def >>(num); end

  sig.returns(T.any(TrueClass, FalseClass))
  def coredump?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def exited?(); end

  sig.returns(T.nilable(Integer))
  def exitstatus(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Integer)
  def pid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def signaled?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def stopped?(); end

  sig.returns(T.nilable(Integer))
  def stopsig(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def success?(); end

  sig.returns(T.nilable(Integer))
  def termsig(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(String)
  def to_s(); end
end

module Process::Sys
  sig.returns(Integer)
  def self.geteuid(); end

  sig.returns(Integer)
  def self.getgid(); end

  sig.returns(Integer)
  def self.getuid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.issetugid(); end

  sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setegid(group); end

  sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.seteuid(user); end

  sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setgid(group); end

  sig(
      rid: Integer,
      eid: Integer,
  )
  .returns(NilClass)
  def self.setregid(rid, eid); end

  sig(
      rid: Integer,
      eid: Integer,
      sid: Integer,
  )
  .returns(NilClass)
  def self.setresgid(rid, eid, sid); end

  sig(
      rid: Integer,
      eid: Integer,
      sid: Integer,
  )
  .returns(NilClass)
  def self.setresuid(rid, eid, sid); end

  sig(
      rid: Integer,
      eid: Integer,
  )
  .returns(NilClass)
  def self.setreuid(rid, eid); end

  sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setrgid(group); end

  sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.setruid(user); end

  sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.setuid(user); end
end

class Process::Tms < Struct
end

module Process::UID
  sig(
      user: Integer,
  )
  .returns(Integer)
  def self.change_privilege(user); end

  sig.returns(Integer)
  def self.eid(); end

  sig(
      name: String,
  )
  .returns(Integer)
  def self.from_name(name); end

  sig(
      user: Integer,
  )
  .returns(Integer)
  def self.grant_privilege(user); end

  sig.returns(Integer)
  def self.re_exchange(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.re_exchangeable?(); end

  sig.returns(Integer)
  def self.rid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.sid_available?(); end

  sig.returns(Integer)
  type_parameters(:T).sig(
      blk: T.proc().returns(T.type_parameter(:T)),
  )
  .returns(T.type_parameter(:T))
  def self.switch(&blk); end

  sig(
      user: Integer,
  )
  .returns(Integer)
  def self.eid=(user); end
end

class Process::Waiter < Thread
  sig.returns(Integer)
  def pid(); end
end
