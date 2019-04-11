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

  sig do
    params(
        msg: String,
    )
    .returns(T.untyped)
  end
  def self.abort(msg=T.unsafe(nil)); end

  sig {returns(String)}
  def self.argv0(); end

  sig do
    params(
        clock_id: T.any(Symbol, Integer),
        unit: Symbol,
    )
    .returns(T.any(Float, Integer))
  end
  def self.clock_getres(clock_id, unit=T.unsafe(nil)); end

  sig do
    params(
        clock_id: T.any(Symbol, Integer),
        unit: Symbol,
    )
    .returns(T.any(Float, Integer))
  end
  def self.clock_gettime(clock_id, unit=T.unsafe(nil)); end

  sig do
    params(
        nochdir: BasicObject,
        noclose: BasicObject,
    )
    .returns(Integer)
  end
  def self.daemon(nochdir=T.unsafe(nil), noclose=T.unsafe(nil)); end

  sig do
    params(
        pid: Integer,
    )
    .returns(Thread)
  end
  def self.detach(pid); end

  sig {returns(Integer)}
  def self.egid(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.egid=(arg0); end

  sig {returns(Integer)}
  def self.euid(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.euid=(arg0); end

  sig do
    params(
        status: Integer,
    )
    .returns(T.untyped)
  end
  def self.exit(status=T.unsafe(nil)); end

  sig do
    params(
        status: Integer,
    )
    .returns(T.untyped)
  end
  def self.exit!(status=T.unsafe(nil)); end

  sig {returns(T.nilable(Integer))}
  sig do
    params(
        blk: T.proc.params().returns(BasicObject),
    )
    .returns(T.nilable(Integer))
  end
  def self.fork(&blk); end

  sig do
    params(
        pid: Integer,
    )
    .returns(Integer)
  end
  def self.getpgid(pid); end

  sig {returns(Integer)}
  def self.getpgrp(); end

  sig do
    params(
        kind: Integer,
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.getpriority(kind, arg0); end

  sig do
    params(
        resource: T.any(Symbol, String, Integer),
    )
    .returns([Integer, Integer])
  end
  def self.getrlimit(resource); end

  sig do
    params(
        pid: Integer,
    )
    .returns(Integer)
  end
  def self.getsid(pid=T.unsafe(nil)); end

  sig {returns(Integer)}
  def self.gid(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.gid=(arg0); end

  sig {returns(T::Array[Integer])}
  def self.groups(); end

  sig do
    params(
        arg0: T::Array[Integer],
    )
    .returns(T::Array[Integer])
  end
  def self.groups=(arg0); end

  sig do
    params(
        username: String,
        gid: Integer,
    )
    .returns(T::Array[Integer])
  end
  def self.initgroups(username, gid); end

  sig do
    params(
        signal: T.any(Integer, Symbol, String),
        pids: Integer,
    )
    .returns(Integer)
  end
  def self.kill(signal, *pids); end

  sig {returns(Integer)}
  def self.maxgroups(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.maxgroups=(arg0); end

  sig {returns(Integer)}
  def self.pid(); end

  sig {returns(Integer)}
  def self.ppid(); end

  sig do
    params(
        pid: Integer,
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.setpgid(pid, arg0); end

  sig do
    params(
        kind: Integer,
        arg0: Integer,
        priority: Integer,
    )
    .returns(Integer)
  end
  def self.setpriority(kind, arg0, priority); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def self.setproctitle(arg0); end

  sig do
    params(
        resource: T.any(Symbol, String, Integer),
        cur_limit: Integer,
        max_limit: Integer,
    )
    .returns(NilClass)
  end
  def self.setrlimit(resource, cur_limit, max_limit=T.unsafe(nil)); end

  sig {returns(Integer)}
  def self.setsid(); end

  sig {returns(Process::Tms)}
  def self.times(); end

  sig {returns(Integer)}
  def self.uid(); end

  sig do
    params(
        user: Integer,
    )
    .returns(Integer)
  end
  def self.uid=(user); end

  sig do
    params(
        pid: Integer,
        flags: Integer,
    )
    .returns(Integer)
  end
  def self.wait(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  sig do
    params(
        pid: Integer,
        flags: Integer,
    )
    .returns([Integer, Process::Status])
  end
  def self.wait2(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  sig {returns(T::Array[[Integer, Process::Status]])}
  def self.waitall(); end

  sig do
    params(
        pid: Integer,
        flags: Integer,
    )
    .returns(Integer)
  end
  def self.waitpid(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  sig do
    params(
        pid: Integer,
        flags: Integer,
    )
    .returns([Integer, Process::Status])
  end
  def self.waitpid2(pid=T.unsafe(nil), flags=T.unsafe(nil)); end
end

module Process::GID
  sig do
    params(
        group: Integer,
    )
    .returns(Integer)
  end
  def self.change_privilege(group); end

  sig {returns(Integer)}
  def self.eid(); end

  sig do
    params(
        name: String,
    )
    .returns(Integer)
  end
  def self.from_name(name); end

  sig do
    params(
        group: Integer,
    )
    .returns(Integer)
  end
  def self.grant_privilege(group); end

  sig {returns(Integer)}
  def self.re_exchange(); end

  sig {returns(T::Boolean)}
  def self.re_exchangeable?(); end

  sig {returns(Integer)}
  def self.rid(); end

  sig {returns(T::Boolean)}
  def self.sid_available?(); end

  sig {returns(Integer)}
  sig do
    type_parameters(:T).params(
        blk: T.proc.params().returns(T.type_parameter(:T)),
    )
    .returns(T.type_parameter(:T))
  end
  def self.switch(&blk); end

  sig do
    params(
        group: Integer,
    )
    .returns(Integer)
  end
  def self.eid=(group); end
end

class Process::Status < Object
  sig do
    params(
        num: Integer,
    )
    .returns(Integer)
  end
  def &(num); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end

  sig do
    params(
        num: Integer,
    )
    .returns(Integer)
  end
  def >>(num); end

  sig {returns(T::Boolean)}
  def coredump?(); end

  sig {returns(T::Boolean)}
  def exited?(); end

  sig {returns(T.nilable(Integer))}
  def exitstatus(); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(Integer)}
  def pid(); end

  sig {returns(T::Boolean)}
  def signaled?(); end

  sig {returns(T::Boolean)}
  def stopped?(); end

  sig {returns(T.nilable(Integer))}
  def stopsig(); end

  sig {returns(T::Boolean)}
  def success?(); end

  sig {returns(T.nilable(Integer))}
  def termsig(); end

  sig {returns(Integer)}
  def to_i(); end

  sig {returns(String)}
  def to_s(); end
end

module Process::Sys
  sig {returns(Integer)}
  def self.geteuid(); end

  sig {returns(Integer)}
  def self.getgid(); end

  sig {returns(Integer)}
  def self.getuid(); end

  sig {returns(T::Boolean)}
  def self.issetugid(); end

  sig do
    params(
        group: Integer,
    )
    .returns(NilClass)
  end
  def self.setegid(group); end

  sig do
    params(
        user: Integer,
    )
    .returns(NilClass)
  end
  def self.seteuid(user); end

  sig do
    params(
        group: Integer,
    )
    .returns(NilClass)
  end
  def self.setgid(group); end

  sig do
    params(
        rid: Integer,
        eid: Integer,
    )
    .returns(NilClass)
  end
  def self.setregid(rid, eid); end

  sig do
    params(
        rid: Integer,
        eid: Integer,
        sid: Integer,
    )
    .returns(NilClass)
  end
  def self.setresgid(rid, eid, sid); end

  sig do
    params(
        rid: Integer,
        eid: Integer,
        sid: Integer,
    )
    .returns(NilClass)
  end
  def self.setresuid(rid, eid, sid); end

  sig do
    params(
        rid: Integer,
        eid: Integer,
    )
    .returns(NilClass)
  end
  def self.setreuid(rid, eid); end

  sig do
    params(
        group: Integer,
    )
    .returns(NilClass)
  end
  def self.setrgid(group); end

  sig do
    params(
        user: Integer,
    )
    .returns(NilClass)
  end
  def self.setruid(user); end

  sig do
    params(
        user: Integer,
    )
    .returns(NilClass)
  end
  def self.setuid(user); end
end

module Process::UID
  sig do
    params(
        user: Integer,
    )
    .returns(Integer)
  end
  def self.change_privilege(user); end

  sig {returns(Integer)}
  def self.eid(); end

  sig do
    params(
        name: String,
    )
    .returns(Integer)
  end
  def self.from_name(name); end

  sig do
    params(
        user: Integer,
    )
    .returns(Integer)
  end
  def self.grant_privilege(user); end

  sig {returns(Integer)}
  def self.re_exchange(); end

  sig {returns(T::Boolean)}
  def self.re_exchangeable?(); end

  sig {returns(Integer)}
  def self.rid(); end

  sig {returns(T::Boolean)}
  def self.sid_available?(); end

  sig {returns(Integer)}
  sig do
    type_parameters(:T).params(
        blk: T.proc.params().returns(T.type_parameter(:T)),
    )
    .returns(T.type_parameter(:T))
  end
  def self.switch(&blk); end

  sig do
    params(
        user: Integer,
    )
    .returns(Integer)
  end
  def self.eid=(user); end
end

class Process::Waiter < Thread
  sig {returns(Integer)}
  def pid(); end
end
