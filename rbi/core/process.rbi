# typed: __STDLIB_INTERNAL

# [Module](https://ruby-doc.org/core-2.6.3/Module.html) to handle
# processes.
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

  # Returns the name of the script being executed. The value is not affected
  # by assigning a new value to $0.
  #
  # This method first appeared in Ruby 2.1 to serve as a global variable
  # free means to get the script name.
  sig {returns(String)}
  def self.argv0(); end

  # Returns the time resolution returned by POSIX
  # [::clock\_getres](Process.downloaded.ruby_doc#method-c-clock_getres) ()
  # function.
  #
  # `clock_id` specifies a kind of clock. See the document of
  # `Process.clock_gettime` for details.
  #
  # `clock_id` can be a symbol as `Process.clock_gettime` . However the
  # result may not be accurate. For example,
  # +Process.clock\_getres(:GETTIMEOFDAY\_BASED\_CLOCK\_REALTIME)+ returns
  # 1.0e-06 which means 1 microsecond, but actual resolution can be more
  # coarse.
  #
  # If the given `clock_id` is not supported, Errno::EINVAL is raised.
  #
  # `unit` specifies a type of the return value. `Process.clock_getres`
  # accepts `unit` as `Process.clock_gettime` . The default value,
  # `:float_second`, is also same as `Process.clock_gettime` .
  #
  # `Process.clock_getres` also accepts `:hertz` as `unit` . `:hertz` means
  # a the reciprocal of `:float_second` .
  #
  # `:hertz` can be used to obtain the exact value of the clock ticks per
  # second for times() function and CLOCKS\_PER\_SEC for clock() function.
  #
  # \+Process.clock\_getres(:TIMES\_BASED\_CLOCK\_PROCESS\_CPUTIME\_ID,
  # :hertz)+ returns the clock ticks per second.
  #
  # \+Process.clock\_getres(:CLOCK\_BASED\_CLOCK\_PROCESS\_CPUTIME\_ID,
  # :hertz)+ returns CLOCKS\_PER\_SEC.
  #
  # ```ruby
  # p Process.clock_getres(Process::CLOCK_MONOTONIC)
  # #=> 1.0e-09
  # ```
  sig do
    params(
        clock_id: T.any(Symbol, Integer),
        unit: Symbol,
    )
    .returns(T.any(Float, Integer))
  end
  def self.clock_getres(clock_id, unit=T.unsafe(nil)); end

  # Returns a time returned by POSIX
  # [::clock\_gettime](Process.downloaded.ruby_doc#method-c-clock_gettime)
  # () function.
  #
  # ```ruby
  # p Process.clock_gettime(Process::CLOCK_MONOTONIC)
  # #=> 896053.968060096
  # ```
  #
  # `clock_id` specifies a kind of clock. It is specified as a constant
  # which begins with `Process::CLOCK_` such as Process::CLOCK\_REALTIME and
  # Process::CLOCK\_MONOTONIC.
  #
  # The supported constants depends on OS and version. Ruby provides
  # following types of `clock_id` if available.
  #
  #   - [CLOCK\_REALTIME](Process.downloaded.ruby_doc#CLOCK_REALTIME)
  #     SUSv2 to 4, Linux 2.5.63, FreeBSD 3.0, NetBSD 2.0, OpenBSD 2.1,
  #     macOS 10.12
  #
  #   - [CLOCK\_MONOTONIC](Process.downloaded.ruby_doc#CLOCK_MONOTONIC)
  #     SUSv3 to 4, Linux 2.5.63, FreeBSD 3.0, NetBSD 2.0, OpenBSD 3.4,
  #     macOS 10.12
  #
  #   - [CLOCK\_PROCESS\_CPUTIME\_ID](Process.downloaded.ruby_doc#CLOCK_PROCESS_CPUTIME_ID)
  #     SUSv3 to 4, Linux 2.5.63, OpenBSD 5.4, macOS 10.12
  #
  #   - [CLOCK\_THREAD\_CPUTIME\_ID](Process.downloaded.ruby_doc#CLOCK_THREAD_CPUTIME_ID)
  #     SUSv3 to 4, Linux 2.5.63, FreeBSD 7.1, OpenBSD 5.4, macOS 10.12
  #
  #   - [CLOCK\_VIRTUAL](Process.downloaded.ruby_doc#CLOCK_VIRTUAL)
  #     FreeBSD 3.0, OpenBSD 2.1
  #
  #   - [CLOCK\_PROF](Process.downloaded.ruby_doc#CLOCK_PROF)
  #     FreeBSD 3.0, OpenBSD 2.1
  #
  #   - [CLOCK\_REALTIME\_FAST](Process.downloaded.ruby_doc#CLOCK_REALTIME_FAST)
  #     FreeBSD 8.1
  #
  #   - [CLOCK\_REALTIME\_PRECISE](Process.downloaded.ruby_doc#CLOCK_REALTIME_PRECISE)
  #     FreeBSD 8.1
  #
  #   - [CLOCK\_REALTIME\_COARSE](Process.downloaded.ruby_doc#CLOCK_REALTIME_COARSE)
  #     Linux 2.6.32
  #
  #   - [CLOCK\_REALTIME\_ALARM](Process.downloaded.ruby_doc#CLOCK_REALTIME_ALARM)
  #     Linux 3.0
  #
  #   - [CLOCK\_MONOTONIC\_FAST](Process.downloaded.ruby_doc#CLOCK_MONOTONIC_FAST)
  #     FreeBSD 8.1
  #
  #   - [CLOCK\_MONOTONIC\_PRECISE](Process.downloaded.ruby_doc#CLOCK_MONOTONIC_PRECISE)
  #     FreeBSD 8.1
  #
  #   - [CLOCK\_MONOTONIC\_COARSE](Process.downloaded.ruby_doc#CLOCK_MONOTONIC_COARSE)
  #     Linux 2.6.32
  #
  #   - [CLOCK\_MONOTONIC\_RAW](Process.downloaded.ruby_doc#CLOCK_MONOTONIC_RAW)
  #     Linux 2.6.28, macOS 10.12
  #
  #   - [CLOCK\_MONOTONIC\_RAW\_APPROX](Process.downloaded.ruby_doc#CLOCK_MONOTONIC_RAW_APPROX)
  #     macOS 10.12
  #
  #   - [CLOCK\_BOOTTIME](Process.downloaded.ruby_doc#CLOCK_BOOTTIME)
  #     Linux 2.6.39
  #
  #   - [CLOCK\_BOOTTIME\_ALARM](Process.downloaded.ruby_doc#CLOCK_BOOTTIME_ALARM)
  #     Linux 3.0
  #
  #   - [CLOCK\_UPTIME](Process.downloaded.ruby_doc#CLOCK_UPTIME)
  #     FreeBSD 7.0, OpenBSD 5.5
  #
  #   - [CLOCK\_UPTIME\_FAST](Process.downloaded.ruby_doc#CLOCK_UPTIME_FAST)
  #     FreeBSD 8.1
  #
  #   - [CLOCK\_UPTIME\_RAW](Process.downloaded.ruby_doc#CLOCK_UPTIME_RAW)
  #     macOS 10.12
  #
  #   - [CLOCK\_UPTIME\_RAW\_APPROX](Process.downloaded.ruby_doc#CLOCK_UPTIME_RAW_APPROX)
  #     macOS 10.12
  #
  #   - [CLOCK\_UPTIME\_PRECISE](Process.downloaded.ruby_doc#CLOCK_UPTIME_PRECISE)
  #     FreeBSD 8.1
  #
  #   - [CLOCK\_SECOND](Process.downloaded.ruby_doc#CLOCK_SECOND)
  #     FreeBSD 8.1
  #
  # Note that SUS stands for Single Unix Specification. SUS contains POSIX
  # and
  # [::clock\_gettime](Process.downloaded.ruby_doc#method-c-clock_gettime)
  # is defined in the POSIX part. SUS defines
  # [CLOCK\_REALTIME](Process.downloaded.ruby_doc#CLOCK_REALTIME) mandatory
  # but [CLOCK\_MONOTONIC](Process.downloaded.ruby_doc#CLOCK_MONOTONIC) ,
  # [CLOCK\_PROCESS\_CPUTIME\_ID](Process.downloaded.ruby_doc#CLOCK_PROCESS_CPUTIME_ID)
  # and
  # [CLOCK\_THREAD\_CPUTIME\_ID](Process.downloaded.ruby_doc#CLOCK_THREAD_CPUTIME_ID)
  # are optional.
  #
  # Also, several symbols are accepted as `clock_id` . There are emulations
  # for
  # [::clock\_gettime](Process.downloaded.ruby_doc#method-c-clock_gettime)
  # ().
  #
  # For example, Process::CLOCK\_REALTIME is defined as
  # `:GETTIMEOFDAY_BASED_CLOCK_REALTIME` when
  # [::clock\_gettime](Process.downloaded.ruby_doc#method-c-clock_gettime)
  # () is not available.
  #
  # Emulations for `CLOCK_REALTIME` :
  #
  #   - :GETTIMEOFDAY\_BASED\_CLOCK\_REALTIME
  #     Use gettimeofday() defined by SUS. (SUSv4 obsoleted it, though.) The
  #     resolution is 1 microsecond.
  #
  #   - :TIME\_BASED\_CLOCK\_REALTIME
  #     Use time() defined by ISO C. The resolution is 1 second.
  #
  # Emulations for `CLOCK_MONOTONIC` :
  #
  #   - :MACH\_ABSOLUTE\_TIME\_BASED\_CLOCK\_MONOTONIC
  #     Use mach\_absolute\_time(), available on Darwin. The resolution is
  #     CPU dependent.
  #
  #   - :TIMES\_BASED\_CLOCK\_MONOTONIC
  #     Use the result value of times() defined by POSIX. POSIX defines it
  #     as “times() shall return the elapsed real time, in clock ticks,
  #     since an arbitrary point in the past (for example, system start-up
  #     time)”. For example, GNU/Linux returns a value based on jiffies and
  #     it is monotonic. However, 4.4BSD uses gettimeofday() and it is not
  #     monotonic. (FreeBSD uses
  #     [::clock\_gettime](Process.downloaded.ruby_doc#method-c-clock_gettime)
  #     (CLOCK\_MONOTONIC) instead, though.) The resolution is the clock
  #     tick. “getconf CLK\_TCK” command shows the clock ticks per second.
  #     (The clock ticks per second is defined by HZ macro in older
  #     systems.) If it is 100 and clock\_t is 32 bits integer type, the
  #     resolution is 10 millisecond and cannot represent over 497 days.
  #
  # Emulations for `CLOCK_PROCESS_CPUTIME_ID` :
  #
  #   - :GETRUSAGE\_BASED\_CLOCK\_PROCESS\_CPUTIME\_ID
  #     Use getrusage() defined by SUS. getrusage() is used with
  #     RUSAGE\_SELF to obtain the time only for the calling process
  #     (excluding the time for child processes). The result is addition of
  #     user time (ru\_utime) and system time (ru\_stime). The resolution is
  #     1 microsecond.
  #
  #   - :TIMES\_BASED\_CLOCK\_PROCESS\_CPUTIME\_ID
  #     Use times() defined by POSIX. The result is addition of user time
  #     (tms\_utime) and system time (tms\_stime). tms\_cutime and
  #     tms\_cstime are ignored to exclude the time for child processes. The
  #     resolution is the clock tick. “getconf CLK\_TCK” command shows the
  #     clock ticks per second. (The clock ticks per second is defined by HZ
  #     macro in older systems.) If it is 100, the resolution is 10
  #     millisecond.
  #
  #   - :CLOCK\_BASED\_CLOCK\_PROCESS\_CPUTIME\_ID
  #     Use clock() defined by ISO C. The resolution is 1/CLOCKS\_PER\_SEC.
  #     CLOCKS\_PER\_SEC is the C-level macro defined by time.h. SUS defines
  #     CLOCKS\_PER\_SEC is 1000000. Non-Unix systems may define it a
  #     different value, though. If CLOCKS\_PER\_SEC is 1000000 as SUS, the
  #     resolution is 1 microsecond. If CLOCKS\_PER\_SEC is 1000000 and
  #     clock\_t is 32 bits integer type, it cannot represent over 72
  #     minutes.
  #
  # If the given `clock_id` is not supported, Errno::EINVAL is raised.
  #
  # `unit` specifies a type of the return value.
  #
  #   - :float\_second
  #     number of seconds as a float (default)
  #
  #   - :float\_millisecond
  #     number of milliseconds as a float
  #
  #   - :float\_microsecond
  #     number of microseconds as a float
  #
  #   - :second
  #     number of seconds as an integer
  #
  #   - :millisecond
  #     number of milliseconds as an integer
  #
  #   - :microsecond
  #     number of microseconds as an integer
  #
  #   - :nanosecond
  #     number of nanoseconds as an integer
  #
  # The underlying function,
  # [::clock\_gettime](Process.downloaded.ruby_doc#method-c-clock_gettime)
  # (), returns a number of nanoseconds.
  # [Float](https://ruby-doc.org/core-2.6.3/Float.html) object (IEEE 754
  # double) is not enough to represent the return value for
  # [CLOCK\_REALTIME](Process.downloaded.ruby_doc#CLOCK_REALTIME) . If the
  # exact nanoseconds value is required, use `:nanoseconds` as the `unit` .
  #
  # The origin (zero) of the returned value varies. For example, system
  # start up time, process start up time, the Epoch, etc.
  #
  # The origin in
  # [CLOCK\_REALTIME](Process.downloaded.ruby_doc#CLOCK_REALTIME) is defined
  # as the Epoch (1970-01-01 00:00:00 UTC). But some systems count leap
  # seconds and others doesn’t. So the result can be interpreted differently
  # across systems.
  # [Time.now](https://ruby-doc.org/core-2.6.3/Time.html#method-c-now) is
  # recommended over
  # [CLOCK\_REALTIME](Process.downloaded.ruby_doc#CLOCK_REALTIME) .
  sig do
    params(
        clock_id: T.any(Symbol, Integer),
        unit: Symbol,
    )
    .returns(T.any(Float, Integer))
  end
  def self.clock_gettime(clock_id, unit=T.unsafe(nil)); end

  # Detach the process from controlling terminal and run in the background
  # as system daemon. Unless the argument nochdir is true (i.e. non false),
  # it changes the current working directory to the root (“/”). Unless the
  # argument noclose is true, daemon() will redirect standard input,
  # standard output and standard error to /dev/null. Return zero on success,
  # or raise one of Errno::\*.
  sig do
    params(
        nochdir: BasicObject,
        noclose: BasicObject,
    )
    .returns(Integer)
  end
  def self.daemon(nochdir=T.unsafe(nil), noclose=T.unsafe(nil)); end

  # Some operating systems retain the status of terminated child processes
  # until the parent collects that status (normally using some variant of
  # `wait()` ). If the parent never collects this status, the child stays
  # around as a *zombie* process. `Process::detach` prevents this by setting
  # up a separate Ruby thread whose sole job is to reap the status of the
  # process *pid* when it terminates. Use `detach` only when you do not
  # intend to explicitly wait for the child to terminate.
  #
  # The waiting thread returns the exit status of the detached process when
  # it terminates, so you can use `Thread#join` to know the result. If
  # specified *pid* is not a valid child process ID, the thread returns
  # `nil` immediately.
  #
  # The waiting thread has `pid` method which returns the pid.
  #
  # In this first example, we don’t reap the first child process, so it
  # appears as a zombie in the process status display.
  #
  # ```ruby
  # p1 = fork { sleep 0.1 }
  # p2 = fork { sleep 0.2 }
  # Process.waitpid(p2)
  # sleep 2
  # system("ps -ho pid,state -p #{p1}")
  # ```
  #
  # *produces:*
  #
  #     27389 Z
  #
  # In the next example, `Process::detach` is used to reap the child
  # automatically.
  #
  # ```ruby
  # p1 = fork { sleep 0.1 }
  # p2 = fork { sleep 0.2 }
  # Process.detach(p1)
  # Process.waitpid(p2)
  # sleep 2
  # system("ps -ho pid,state -p #{p1}")
  # ```
  #
  # *(produces no output)*
  sig do
    params(
        pid: Integer,
    )
    .returns(Thread)
  end
  def self.detach(pid); end

  # Returns the effective group ID for this process. Not available on all
  # platforms.
  #
  # ```ruby
  # Process.egid   #=> 500
  # ```
  sig {returns(Integer)}
  def self.egid(); end

  # Sets the effective group ID for this process. Not available on all
  # platforms.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.egid=(arg0); end

  # Returns the effective user ID for this process.
  #
  # ```ruby
  # Process.euid   #=> 501
  # ```
  sig {returns(Integer)}
  def self.euid(); end

  # Sets the effective user ID for this process. Not available on all
  # platforms.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.euid=(arg0); end

  # Returns the process group ID for the given process id. Not available on
  # all platforms.
  #
  # ```ruby
  # Process.getpgid(Process.ppid())   #=> 25527
  # ```
  sig do
    params(
        pid: Integer,
    )
    .returns(Integer)
  end
  def self.getpgid(pid); end

  # Returns the process group ID for this process. Not available on all
  # platforms.
  #
  # ```ruby
  # Process.getpgid(0)   #=> 25527
  # Process.getpgrp      #=> 25527
  # ```
  sig {returns(Integer)}
  def self.getpgrp(); end

  # Gets the scheduling priority for specified process, process group, or
  # user. *kind* indicates the kind of entity to find: one of
  # `Process::PRIO_PGRP`, `Process::PRIO_USER`, or `Process::PRIO_PROCESS`
  # . *integer* is an id indicating the particular process, process group,
  # or user (an id of 0 means *current* ). Lower priorities are more
  # favorable for scheduling. Not available on all platforms.
  #
  # ```ruby
  # Process.getpriority(Process::PRIO_USER, 0)      #=> 19
  # Process.getpriority(Process::PRIO_PROCESS, 0)   #=> 19
  # ```
  sig do
    params(
        kind: Integer,
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.getpriority(kind, arg0); end

  # Gets the resource limit of the process. *cur\_limit* means current
  # (soft) limit and *max\_limit* means maximum (hard) limit.
  #
  # *resource* indicates the kind of resource to limit. It is specified as a
  # symbol such as `:CORE`, a string such as `"CORE"` or a constant such as
  # `Process::RLIMIT_CORE` . See
  # [::setrlimit](Process.downloaded.ruby_doc#method-c-setrlimit) for
  # details.
  #
  # *cur\_limit* and *max\_limit* may be `Process::RLIM_INFINITY`,
  # `Process::RLIM_SAVED_MAX` or `Process::RLIM_SAVED_CUR` . See
  # [::setrlimit](Process.downloaded.ruby_doc#method-c-setrlimit) and the
  # system getrlimit(2) manual for details.
  sig do
    params(
        resource: T.any(Symbol, String, Integer),
    )
    .returns([Integer, Integer])
  end
  def self.getrlimit(resource); end

  # Returns the session ID for the given process id. If not given, return
  # current process sid. Not available on all platforms.
  #
  # ```ruby
  # Process.getsid()                #=> 27422
  # Process.getsid(0)               #=> 27422
  # Process.getsid(Process.pid())   #=> 27422
  # ```
  sig do
    params(
        pid: Integer,
    )
    .returns(Integer)
  end
  def self.getsid(pid=T.unsafe(nil)); end

  # Returns the (real) group ID for this process.
  #
  # ```ruby
  # Process.gid   #=> 500
  # ```
  sig {returns(Integer)}
  def self.gid(); end

  # Sets the group ID for this process.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.gid=(arg0); end

  # Get an `Array` of the group IDs in the supplemental group access list
  # for this process.
  #
  # ```ruby
  # Process.groups   #=> [27, 6, 10, 11]
  # ```
  #
  # Note that this method is just a wrapper of getgroups(2). This means that
  # the following characteristics of the result completely depend on your
  # system:
  #
  #   - the result is sorted
  #
  #   - the result includes effective GIDs
  #
  #   - the result does not include duplicated GIDs
  #
  # You can make sure to get a sorted unique
  # [GID](https://ruby-doc.org/core-2.6.3/Process/GID.html) list of the
  # current process by this expression:
  #
  # ```ruby
  # Process.groups.uniq.sort
  # ```
  sig {returns(T::Array[Integer])}
  def self.groups(); end

  # Set the supplemental group access list to the given `Array` of group
  # IDs.
  #
  # ```ruby
  # Process.groups   #=> [0, 1, 2, 3, 4, 6, 10, 11, 20, 26, 27]
  # Process.groups = [27, 6, 10, 11]   #=> [27, 6, 10, 11]
  # Process.groups   #=> [27, 6, 10, 11]
  # ```
  sig do
    params(
        arg0: T::Array[Integer],
    )
    .returns(T::Array[Integer])
  end
  def self.groups=(arg0); end

  # Initializes the supplemental group access list by reading the system
  # group database and using all groups of which the given user is a member.
  # The group with the specified *gid* is also added to the list. Returns
  # the resulting `Array` of the gids of all the groups in the supplementary
  # group access list. Not available on all platforms.
  #
  # ```ruby
  # Process.groups   #=> [0, 1, 2, 3, 4, 6, 10, 11, 20, 26, 27]
  # Process.initgroups( "mgranger", 30 )   #=> [30, 6, 10, 11]
  # Process.groups   #=> [30, 6, 10, 11]
  # ```
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

  # Returns the maximum number of gids allowed in the supplemental group
  # access list.
  #
  # ```ruby
  # Process.maxgroups   #=> 32
  # ```
  sig {returns(Integer)}
  def self.maxgroups(); end

  # Sets the maximum number of gids allowed in the supplemental group access
  # list.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.maxgroups=(arg0); end

  # Returns the process id of this process. Not available on all platforms.
  #
  # ```ruby
  # Process.pid   #=> 27415
  # ```
  sig {returns(Integer)}
  def self.pid(); end

  # Returns the process id of the parent of this process. Returns
  # untrustworthy value on Win32/64. Not available on all platforms.
  #
  # ```ruby
  # puts "I am #{Process.pid}"
  # Process.fork { puts "Dad is #{Process.ppid}" }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # I am 27417
  # Dad is 27417
  # ```
  sig {returns(Integer)}
  def self.ppid(); end

  # Sets the process group ID of *pid* (0 indicates this process) to
  # *integer* . Not available on all platforms.
  sig do
    params(
        pid: Integer,
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.setpgid(pid, arg0); end

  # See `Process#getpriority` .
  #
  # ```ruby
  # Process.setpriority(Process::PRIO_USER, 0, 19)      #=> 0
  # Process.setpriority(Process::PRIO_PROCESS, 0, 19)   #=> 0
  # Process.getpriority(Process::PRIO_USER, 0)          #=> 19
  # Process.getpriority(Process::PRIO_PROCESS, 0)       #=> 19
  # ```
  sig do
    params(
        kind: Integer,
        arg0: Integer,
        priority: Integer,
    )
    .returns(Integer)
  end
  def self.setpriority(kind, arg0, priority); end

  # Sets the process title that appears on the ps(1) command. Not
  # necessarily effective on all platforms. No exception will be raised
  # regardless of the result, nor will
  # [NotImplementedError](https://ruby-doc.org/core-2.6.3/NotImplementedError.html)
  # be raised even if the platform does not support the feature.
  #
  # Calling this method does not affect the value of $0.
  #
  # ```ruby
  # Process.setproctitle('myapp: worker #%d' % worker_id)
  # ```
  #
  # This method first appeared in Ruby 2.1 to serve as a global variable
  # free means to change the process title.
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def self.setproctitle(arg0); end

  # Sets the resource limit of the process. *cur\_limit* means current
  # (soft) limit and *max\_limit* means maximum (hard) limit.
  #
  # If *max\_limit* is not given, *cur\_limit* is used.
  #
  # *resource* indicates the kind of resource to limit. It should be a
  # symbol such as `:CORE`, a string such as `"CORE"` or a constant such as
  # `Process::RLIMIT_CORE` . The available resources are OS dependent. Ruby
  # may support following resources.
  #
  #   - AS
  #     total available memory (bytes) (SUSv3, NetBSD, FreeBSD, OpenBSD but
  #     4.4BSD-Lite)
  #
  #   - CORE
  #     core size (bytes) (SUSv3)
  #
  #   - CPU
  #     CPU time (seconds) (SUSv3)
  #
  #   - DATA
  #     data segment (bytes) (SUSv3)
  #
  #   - FSIZE
  #     file size (bytes) (SUSv3)
  #
  #   - MEMLOCK
  #     total size for mlock(2) (bytes) (4.4BSD, GNU/Linux)
  #
  #   - MSGQUEUE
  #     allocation for POSIX message queues (bytes) (GNU/Linux)
  #
  #   - NICE
  #     ceiling on process’s nice(2) value (number) (GNU/Linux)
  #
  #   - NOFILE
  #     file descriptors (number) (SUSv3)
  #
  #   - NPROC
  #     number of processes for the user (number) (4.4BSD, GNU/Linux)
  #
  #   - RSS
  #     resident memory size (bytes) (4.2BSD, GNU/Linux)
  #
  #   - RTPRIO
  #     ceiling on the process’s real-time priority (number) (GNU/Linux)
  #
  #   - RTTIME
  #     CPU time for real-time process (us) (GNU/Linux)
  #
  #   - SBSIZE
  #     all socket buffers (bytes) (NetBSD, FreeBSD)
  #
  #   - SIGPENDING
  #     number of queued signals allowed (signals) (GNU/Linux)
  #
  #   - STACK
  #     stack size (bytes) (SUSv3)
  #
  # *cur\_limit* and *max\_limit* may be `:INFINITY`, `"INFINITY"` or
  # `Process::RLIM_INFINITY`, which means that the resource is not limited.
  # They may be `Process::RLIM_SAVED_MAX`, `Process::RLIM_SAVED_CUR` and
  # corresponding symbols and strings too. See system setrlimit(2) manual
  # for details.
  #
  # The following example raises the soft limit of core size to the hard
  # limit to try to make core dump possible.
  #
  # ```ruby
  # Process.setrlimit(:CORE, Process.getrlimit(:CORE)[1])
  # ```
  sig do
    params(
        resource: T.any(Symbol, String, Integer),
        cur_limit: Integer,
        max_limit: Integer,
    )
    .returns(NilClass)
  end
  def self.setrlimit(resource, cur_limit, max_limit=T.unsafe(nil)); end

  # Establishes this process as a new session and process group leader, with
  # no controlling tty. Returns the session id. Not available on all
  # platforms.
  #
  # ```ruby
  # Process.setsid   #=> 27422
  # ```
  sig {returns(Integer)}
  def self.setsid(); end

  # Returns a `Tms` structure (see `Process::Tms` ) that contains user and
  # system CPU times for this process, and also for children processes.
  #
  # ```ruby
  # t = Process.times
  # [ t.utime, t.stime, t.cutime, t.cstime ]   #=> [0.0, 0.02, 0.00, 0.00]
  # ```
  sig {returns(Process::Tms)}
  def self.times(); end

  # Returns the (real) user ID of this process.
  #
  # ```ruby
  # Process.uid   #=> 501
  # ```
  sig {returns(Integer)}
  def self.uid(); end

  # Sets the (user) user ID for this process. Not available on all
  # platforms.
  sig do
    params(
        user: Integer,
    )
    .returns(Integer)
  end
  def self.uid=(user); end

  # Waits for a child process to exit, returns its process id, and sets `$?`
  # to a `Process::Status` object containing information on that process.
  # Which child it waits on depends on the value of *pid* :
  #
  #   - \> 0
  #     Waits for the child whose process ID equals *pid* .
  #
  #   - 0
  #     Waits for any child whose process group ID equals that of the
  #     calling process.
  #
  #   - \-1
  #     Waits for any child process (the default if no *pid* is given).
  #
  #   - \< -1
  #     Waits for any child whose process group ID equals the absolute value
  #     of *pid* .
  #
  # The *flags* argument may be a logical or of the flag values
  # `Process::WNOHANG` (do not block if no child available) or
  # `Process::WUNTRACED` (return stopped children that haven’t been
  # reported). Not all flags are available on all platforms, but a flag
  # value of zero will work on all platforms.
  #
  # Calling this method raises a
  # [SystemCallError](https://ruby-doc.org/core-2.6.3/SystemCallError.html)
  # if there are no child processes. Not available on all platforms.
  #
  # ```ruby
  # include Process
  # fork { exit 99 }                 #=> 27429
  # wait                             #=> 27429
  # $?.exitstatus                    #=> 99
  #
  # pid = fork { sleep 3 }           #=> 27440
  # Time.now                         #=> 2008-03-08 19:56:16 +0900
  # waitpid(pid, Process::WNOHANG)   #=> nil
  # Time.now                         #=> 2008-03-08 19:56:16 +0900
  # waitpid(pid, 0)                  #=> 27440
  # Time.now                         #=> 2008-03-08 19:56:19 +0900
  # ```
  sig do
    params(
        pid: Integer,
        flags: Integer,
    )
    .returns(Integer)
  end
  def self.wait(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  # Waits for a child process to exit (see
  # [::waitpid](Process.downloaded.ruby_doc#method-c-waitpid) for exact
  # semantics) and returns an array containing the process id and the exit
  # status (a `Process::Status` object) of that child. Raises a
  # [SystemCallError](https://ruby-doc.org/core-2.6.3/SystemCallError.html)
  # if there are no child processes.
  #
  # ```ruby
  # Process.fork { exit 99 }   #=> 27437
  # pid, status = Process.wait2
  # pid                        #=> 27437
  # status.exitstatus          #=> 99
  # ```
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

  # Waits for a child process to exit, returns its process id, and sets `$?`
  # to a `Process::Status` object containing information on that process.
  # Which child it waits on depends on the value of *pid* :
  #
  #   - \> 0
  #     Waits for the child whose process ID equals *pid* .
  #
  #   - 0
  #     Waits for any child whose process group ID equals that of the
  #     calling process.
  #
  #   - \-1
  #     Waits for any child process (the default if no *pid* is given).
  #
  #   - \< -1
  #     Waits for any child whose process group ID equals the absolute value
  #     of *pid* .
  #
  # The *flags* argument may be a logical or of the flag values
  # `Process::WNOHANG` (do not block if no child available) or
  # `Process::WUNTRACED` (return stopped children that haven’t been
  # reported). Not all flags are available on all platforms, but a flag
  # value of zero will work on all platforms.
  #
  # Calling this method raises a
  # [SystemCallError](https://ruby-doc.org/core-2.6.3/SystemCallError.html)
  # if there are no child processes. Not available on all platforms.
  #
  # ```ruby
  # include Process
  # fork { exit 99 }                 #=> 27429
  # wait                             #=> 27429
  # $?.exitstatus                    #=> 99
  #
  # pid = fork { sleep 3 }           #=> 27440
  # Time.now                         #=> 2008-03-08 19:56:16 +0900
  # waitpid(pid, Process::WNOHANG)   #=> nil
  # Time.now                         #=> 2008-03-08 19:56:16 +0900
  # waitpid(pid, 0)                  #=> 27440
  # Time.now                         #=> 2008-03-08 19:56:19 +0900
  # ```
  sig do
    params(
        pid: Integer,
        flags: Integer,
    )
    .returns(Integer)
  end
  def self.waitpid(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  # Waits for a child process to exit (see
  # [::waitpid](Process.downloaded.ruby_doc#method-c-waitpid) for exact
  # semantics) and returns an array containing the process id and the exit
  # status (a `Process::Status` object) of that child. Raises a
  # [SystemCallError](https://ruby-doc.org/core-2.6.3/SystemCallError.html)
  # if there are no child processes.
  #
  # ```ruby
  # Process.fork { exit 99 }   #=> 27437
  # pid, status = Process.wait2
  # pid                        #=> 27437
  # status.exitstatus          #=> 99
  # ```
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

class Process::Tms < Struct
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)
end

class Process::Waiter < Thread
  sig {returns(Integer)}
  def pid(); end
end
