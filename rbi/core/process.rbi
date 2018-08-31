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

  Sorbet.sig(
      msg: String,
  )
  .returns(T.untyped)
  def self.abort(msg=T.unsafe(nil)); end

  Sorbet.sig.returns(String)
  def self.argv0(); end

  Sorbet.sig(
      clock_id: T.any(Symbol, Integer),
      unit: Symbol,
  )
  .returns(T.any(Float, Integer))
  def self.clock_getres(clock_id, unit=T.unsafe(nil)); end

  Sorbet.sig(
      clock_id: T.any(Symbol, Integer),
      unit: Symbol,
  )
  .returns(T.any(Float, Integer))
  def self.clock_gettime(clock_id, unit=T.unsafe(nil)); end

  Sorbet.sig(
      nochdir: BasicObject,
      noclose: BasicObject,
  )
  .returns(Integer)
  def self.daemon(nochdir=T.unsafe(nil), noclose=T.unsafe(nil)); end

  Sorbet.sig(
      pid: Integer,
  )
  .returns(Thread)
  def self.detach(pid); end

  Sorbet.sig.returns(Integer)
  def self.egid(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.egid=(arg0); end

  Sorbet.sig.returns(Integer)
  def self.euid(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.euid=(arg0); end

  Sorbet.sig(
      status: Integer,
  )
  .returns(T.untyped)
  def self.exit(status=T.unsafe(nil)); end

  Sorbet.sig(
      status: Integer,
  )
  .returns(T.untyped)
  def self.exit!(status=T.unsafe(nil)); end

  Sorbet.sig.returns(T.nilable(Integer))
  Sorbet.sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(T.nilable(Integer))
  def self.fork(&blk); end

  Sorbet.sig(
      pid: Integer,
  )
  .returns(Integer)
  def self.getpgid(pid); end

  Sorbet.sig.returns(Integer)
  def self.getpgrp(); end

  Sorbet.sig(
      kind: Integer,
      arg0: Integer,
  )
  .returns(Integer)
  def self.getpriority(kind, arg0); end

  Sorbet.sig(
      resource: T.any(Symbol, String, Integer),
  )
  .returns([Integer, Integer])
  def self.getrlimit(resource); end

  Sorbet.sig(
      pid: Integer,
  )
  .returns(Integer)
  def self.getsid(pid=T.unsafe(nil)); end

  Sorbet.sig.returns(Integer)
  def self.gid(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.gid=(arg0); end

  Sorbet.sig.returns(T::Array[Integer])
  def self.groups(); end

  Sorbet.sig(
      arg0: T::Array[Integer],
  )
  .returns(T::Array[Integer])
  def self.groups=(arg0); end

  Sorbet.sig(
      username: String,
      gid: Integer,
  )
  .returns(T::Array[Integer])
  def self.initgroups(username, gid); end

  Sorbet.sig(
      signal: T.any(Integer, Symbol, String),
      pids: Integer,
  )
  .returns(Integer)
  def self.kill(signal, *pids); end

  Sorbet.sig.returns(Integer)
  def self.maxgroups(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.maxgroups=(arg0); end

  Sorbet.sig.returns(Integer)
  def self.pid(); end

  Sorbet.sig.returns(Integer)
  def self.ppid(); end

  Sorbet.sig(
      pid: Integer,
      arg0: Integer,
  )
  .returns(Integer)
  def self.setpgid(pid, arg0); end

  Sorbet.sig(
      kind: Integer,
      arg0: Integer,
      priority: Integer,
  )
  .returns(Integer)
  def self.setpriority(kind, arg0, priority); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def self.setproctitle(arg0); end

  Sorbet.sig(
      resource: T.any(Symbol, String, Integer),
      cur_limit: Integer,
      max_limit: Integer,
  )
  .returns(NilClass)
  def self.setrlimit(resource, cur_limit, max_limit=T.unsafe(nil)); end

  Sorbet.sig.returns(Integer)
  def self.setsid(); end

  Sorbet.sig.returns(Process::Tms)
  def self.times(); end

  Sorbet.sig.returns(Integer)
  def self.uid(); end

  Sorbet.sig(
      user: Integer,
  )
  .returns(Integer)
  def self.uid=(user); end

  Sorbet.sig(
      pid: Integer,
      flags: Integer,
  )
  .returns(Integer)
  def self.wait(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  Sorbet.sig(
      pid: Integer,
      flags: Integer,
  )
  .returns([Integer, Integer])
  def self.wait2(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  Sorbet.sig.returns(T::Array[[Integer, Integer]])
  def self.waitall(); end

  Sorbet.sig(
      pid: Integer,
      flags: Integer,
  )
  .returns(Integer)
  def self.waitpid(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  Sorbet.sig(
      pid: Integer,
      flags: Integer,
  )
  .returns([Integer, Integer])
  def self.waitpid2(pid=T.unsafe(nil), flags=T.unsafe(nil)); end
end

module Process::GID
  Sorbet.sig(
      group: Integer,
  )
  .returns(Integer)
  def self.change_privilege(group); end

  Sorbet.sig.returns(Integer)
  def self.eid(); end

  Sorbet.sig(
      name: String,
  )
  .returns(Integer)
  def self.from_name(name); end

  Sorbet.sig(
      group: Integer,
  )
  .returns(Integer)
  def self.grant_privilege(group); end

  Sorbet.sig.returns(Integer)
  def self.re_exchange(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def self.re_exchangeable?(); end

  Sorbet.sig.returns(Integer)
  def self.rid(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def self.sid_available?(); end

  Sorbet.sig.returns(Integer)
  type_parameters(:T).sig(
      blk: T.proc().returns(T.type_parameter(:T)),
  )
  .returns(T.type_parameter(:T))
  def self.switch(&blk); end

  Sorbet.sig(
      group: Integer,
  )
  .returns(Integer)
  def self.eid=(group); end
end

class Process::Status < Object
  Sorbet.sig(
      num: Integer,
  )
  .returns(Integer)
  def &(num); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  Sorbet.sig(
      num: Integer,
  )
  .returns(Integer)
  def >>(num); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def coredump?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def exited?(); end

  Sorbet.sig.returns(T.nilable(Integer))
  def exitstatus(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(Integer)
  def pid(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def signaled?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def stopped?(); end

  Sorbet.sig.returns(T.nilable(Integer))
  def stopsig(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def success?(); end

  Sorbet.sig.returns(T.nilable(Integer))
  def termsig(); end

  Sorbet.sig.returns(Integer)
  def to_i(); end

  Sorbet.sig.returns(String)
  def to_s(); end
end

module Process::Sys
  Sorbet.sig.returns(Integer)
  def self.geteuid(); end

  Sorbet.sig.returns(Integer)
  def self.getgid(); end

  Sorbet.sig.returns(Integer)
  def self.getuid(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def self.issetugid(); end

  Sorbet.sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setegid(group); end

  Sorbet.sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.seteuid(user); end

  Sorbet.sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setgid(group); end

  Sorbet.sig(
      rid: Integer,
      eid: Integer,
  )
  .returns(NilClass)
  def self.setregid(rid, eid); end

  Sorbet.sig(
      rid: Integer,
      eid: Integer,
      sid: Integer,
  )
  .returns(NilClass)
  def self.setresgid(rid, eid, sid); end

  Sorbet.sig(
      rid: Integer,
      eid: Integer,
      sid: Integer,
  )
  .returns(NilClass)
  def self.setresuid(rid, eid, sid); end

  Sorbet.sig(
      rid: Integer,
      eid: Integer,
  )
  .returns(NilClass)
  def self.setreuid(rid, eid); end

  Sorbet.sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setrgid(group); end

  Sorbet.sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.setruid(user); end

  Sorbet.sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.setuid(user); end
end

module Process::UID
  Sorbet.sig(
      user: Integer,
  )
  .returns(Integer)
  def self.change_privilege(user); end

  Sorbet.sig.returns(Integer)
  def self.eid(); end

  Sorbet.sig(
      name: String,
  )
  .returns(Integer)
  def self.from_name(name); end

  Sorbet.sig(
      user: Integer,
  )
  .returns(Integer)
  def self.grant_privilege(user); end

  Sorbet.sig.returns(Integer)
  def self.re_exchange(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def self.re_exchangeable?(); end

  Sorbet.sig.returns(Integer)
  def self.rid(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def self.sid_available?(); end

  Sorbet.sig.returns(Integer)
  type_parameters(:T).sig(
      blk: T.proc().returns(T.type_parameter(:T)),
  )
  .returns(T.type_parameter(:T))
  def self.switch(&blk); end

  Sorbet.sig(
      user: Integer,
  )
  .returns(Integer)
  def self.eid=(user); end
end

class Process::Waiter < Thread
  Sorbet.sig.returns(Integer)
  def pid(); end
end
