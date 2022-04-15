# typed: __STDLIB_INTERNAL

# The module contains several groups of functionality for handling OS processes:
#
# *   Low-level property introspection and management of the current process,
#     like
#     [`Process.argv0`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-argv0),
#     [`Process.pid`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-pid);
# *   Low-level introspection of other processes, like
#     [`Process.getpgid`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-getpgid),
#     [`Process.getpriority`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-getpriority);
# *   Management of the current process:
#     [`Process.abort`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-abort),
#     [`Process.exit`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-exit),
#     [`Process.daemon`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-daemon),
#     etc. (for convenience, most of those are also available as global
#     functions and module functions of
#     [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html));
# *   Creation and management of child processes:
#     [`Process.fork`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-fork),
#     [`Process.spawn`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-spawn),
#     and related methods;
# *   Management of low-level system clock:
#     [`Process.times`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-times)
#     and
#     [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime),
#     which could be important for proper benchmarking and other elapsed time
#     measurement tasks.
module Process
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_BOOTTIME = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_BOOTTIME_ALARM = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_MONOTONIC = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_MONOTONIC_COARSE = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_MONOTONIC_RAW = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_MONOTONIC_RAW_APPROX = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_PROCESS_CPUTIME_ID = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_REALTIME = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_REALTIME_ALARM = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_REALTIME_COARSE = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_THREAD_CPUTIME_ID = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_UPTIME_RAW = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  CLOCK_UPTIME_RAW_APPROX = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.setpriority`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setpriority)
  PRIO_PGRP = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.setpriority`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setpriority)
  PRIO_PROCESS = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.setpriority`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setpriority)
  PRIO_USER = T.let(T.unsafe(nil), Integer)
  # Maximum size of the process's virtual memory (address space) in bytes.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_AS = T.let(T.unsafe(nil), Integer)
  # Maximum size of the core file.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_CORE = T.let(T.unsafe(nil), Integer)
  # CPU time limit in seconds.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_CPU = T.let(T.unsafe(nil), Integer)
  # Maximum size of the process's data segment.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_DATA = T.let(T.unsafe(nil), Integer)
  # Maximum size of files that the process may create.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_FSIZE = T.let(T.unsafe(nil), Integer)
  # Maximum number of bytes of memory that may be locked into RAM.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_MEMLOCK = T.let(T.unsafe(nil), Integer)
  # Specifies the limit on the number of bytes that can be allocated for POSIX
  # message queues for the real user ID of the calling process.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_MSGQUEUE = T.let(T.unsafe(nil), Integer)
  # Specifies a ceiling to which the process's nice value can be raised.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_NICE = T.let(T.unsafe(nil), Integer)
  # Specifies a value one greater than the maximum file descriptor number that
  # can be opened by this process.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_NOFILE = T.let(T.unsafe(nil), Integer)
  # The maximum number of processes that can be created for the real user ID of
  # the calling process.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_NPROC = T.let(T.unsafe(nil), Integer)
  # Specifies the limit (in pages) of the process's resident set.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_RSS = T.let(T.unsafe(nil), Integer)
  # Specifies a ceiling on the real-time priority that may be set for this
  # process.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_RTPRIO = T.let(T.unsafe(nil), Integer)
  # Specifies limit on CPU time this process scheduled under a real-time
  # scheduling policy can consume.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_RTTIME = T.let(T.unsafe(nil), Integer)
  # Specifies a limit on the number of signals that may be queued for the real
  # user ID of the calling process.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_SIGPENDING = T.let(T.unsafe(nil), Integer)
  # Maximum size of the stack, in bytes.
  #
  # see the system getrlimit(2) manual for details.
  RLIMIT_STACK = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.setrlimit`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setrlimit)
  RLIM_INFINITY = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.setrlimit`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setrlimit)
  RLIM_SAVED_CUR = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.setrlimit`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setrlimit)
  RLIM_SAVED_MAX = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.wait`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-wait)
  WNOHANG = T.let(T.unsafe(nil), Integer)
  # see
  # [`Process.wait`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-wait)
  WUNTRACED = T.let(T.unsafe(nil), Integer)

  # Returns the name of the script being executed. The value is not affected by
  # assigning a new value to $0.
  #
  # This method first appeared in Ruby 2.1 to serve as a global variable free
  # means to get the script name.
  sig {returns(String)}
  def self.argv0(); end

  # Returns the time resolution returned by POSIX
  # [`clock_getres`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_getres)()
  # function.
  #
  # `clock_id` specifies a kind of clock. See the document of
  # `Process.clock_gettime` for details.
  #
  # `clock_id` can be a symbol as `Process.clock_gettime`. However the result
  # may not be accurate. For example,
  # `Process.clock_getres(:GETTIMEOFDAY_BASED_CLOCK_REALTIME)` returns 1.0e-06
  # which means 1 microsecond, but actual resolution can be more coarse.
  #
  # If the given `clock_id` is not supported, Errno::EINVAL is raised.
  #
  # `unit` specifies a type of the return value. `Process.clock_getres` accepts
  # `unit` as `Process.clock_gettime`. The default value, `:float_second`, is
  # also same as `Process.clock_gettime`.
  #
  # `Process.clock_getres` also accepts `:hertz` as `unit`. `:hertz` means a the
  # reciprocal of `:float_second`.
  #
  # `:hertz` can be used to obtain the exact value of the clock ticks per second
  # for times() function and CLOCKS\_PER\_SEC for clock() function.
  #
  # `Process.clock_getres(:TIMES_BASED_CLOCK_PROCESS_CPUTIME_ID, :hertz)`
  # returns the clock ticks per second.
  #
  # `Process.clock_getres(:CLOCK_BASED_CLOCK_PROCESS_CPUTIME_ID, :hertz)`
  # returns CLOCKS\_PER\_SEC.
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
  # [`clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)()
  # function.
  #
  # ```ruby
  # p Process.clock_gettime(Process::CLOCK_MONOTONIC)
  # #=> 896053.968060096
  # ```
  #
  # `clock_id` specifies a kind of clock. It is specified as a constant which
  # begins with `Process::CLOCK_` such as
  # [`Process::CLOCK_REALTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME)
  # and
  # [`Process::CLOCK_MONOTONIC`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC).
  #
  # The supported constants depends on OS and version. Ruby provides following
  # types of `clock_id` if available.
  #
  # [`CLOCK_REALTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME)
  # :   SUSv2 to 4, Linux 2.5.63, FreeBSD 3.0, NetBSD 2.0, OpenBSD 2.1, macOS
  #     10.12
  # [`CLOCK_MONOTONIC`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC)
  # :   SUSv3 to 4, Linux 2.5.63, FreeBSD 3.0, NetBSD 2.0, OpenBSD 3.4, macOS
  #     10.12
  # [`CLOCK_PROCESS_CPUTIME_ID`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_PROCESS_CPUTIME_ID)
  # :   SUSv3 to 4, Linux 2.5.63, FreeBSD 9.3, OpenBSD 5.4, macOS 10.12
  # [`CLOCK_THREAD_CPUTIME_ID`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_THREAD_CPUTIME_ID)
  # :   SUSv3 to 4, Linux 2.5.63, FreeBSD 7.1, OpenBSD 5.4, macOS 10.12
  # [`CLOCK_VIRTUAL`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_VIRTUAL)
  # :   FreeBSD 3.0, OpenBSD 2.1
  # [`CLOCK_PROF`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_PROF)
  # :   FreeBSD 3.0, OpenBSD 2.1
  # [`CLOCK_REALTIME_FAST`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME_FAST)
  # :   FreeBSD 8.1
  # [`CLOCK_REALTIME_PRECISE`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME_PRECISE)
  # :   FreeBSD 8.1
  # [`CLOCK_REALTIME_COARSE`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME_COARSE)
  # :   Linux 2.6.32
  # [`CLOCK_REALTIME_ALARM`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME_ALARM)
  # :   Linux 3.0
  # [`CLOCK_MONOTONIC_FAST`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC_FAST)
  # :   FreeBSD 8.1
  # [`CLOCK_MONOTONIC_PRECISE`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC_PRECISE)
  # :   FreeBSD 8.1
  # [`CLOCK_MONOTONIC_COARSE`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC_COARSE)
  # :   Linux 2.6.32
  # [`CLOCK_MONOTONIC_RAW`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC_RAW)
  # :   Linux 2.6.28, macOS 10.12
  # [`CLOCK_MONOTONIC_RAW_APPROX`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC_RAW_APPROX)
  # :   macOS 10.12
  # [`CLOCK_BOOTTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_BOOTTIME)
  # :   Linux 2.6.39
  # [`CLOCK_BOOTTIME_ALARM`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_BOOTTIME_ALARM)
  # :   Linux 3.0
  # [`CLOCK_UPTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_UPTIME)
  # :   FreeBSD 7.0, OpenBSD 5.5
  # [`CLOCK_UPTIME_FAST`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_UPTIME_FAST)
  # :   FreeBSD 8.1
  # [`CLOCK_UPTIME_RAW`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_UPTIME_RAW)
  # :   macOS 10.12
  # [`CLOCK_UPTIME_RAW_APPROX`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_UPTIME_RAW_APPROX)
  # :   macOS 10.12
  # [`CLOCK_UPTIME_PRECISE`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_UPTIME_PRECISE)
  # :   FreeBSD 8.1
  # [`CLOCK_SECOND`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_SECOND)
  # :   FreeBSD 8.1
  # [`CLOCK_TAI`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_TAI)
  # :   Linux 3.10
  #
  #
  # Note that SUS stands for Single Unix Specification. SUS contains POSIX and
  # [`clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)
  # is defined in the POSIX part. SUS defines
  # [`CLOCK_REALTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME)
  # mandatory but
  # [`CLOCK_MONOTONIC`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC),
  # [`CLOCK_PROCESS_CPUTIME_ID`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_PROCESS_CPUTIME_ID)
  # and
  # [`CLOCK_THREAD_CPUTIME_ID`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_THREAD_CPUTIME_ID)
  # are optional.
  #
  # Also, several symbols are accepted as `clock_id`. There are emulations for
  # [`clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)().
  #
  # For example,
  # [`Process::CLOCK_REALTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME)
  # is defined as `:GETTIMEOFDAY_BASED_CLOCK_REALTIME` when
  # [`clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)()
  # is not available.
  #
  # Emulations for `CLOCK_REALTIME`:
  # :GETTIMEOFDAY\_BASED\_CLOCK\_REALTIME
  # :   Use gettimeofday() defined by SUS. (SUSv4 obsoleted it, though.) The
  #     resolution is 1 microsecond.
  # :TIME\_BASED\_CLOCK\_REALTIME
  # :   Use time() defined by ISO C. The resolution is 1 second.
  #
  #
  # Emulations for `CLOCK_MONOTONIC`:
  # :MACH\_ABSOLUTE\_TIME\_BASED\_CLOCK\_MONOTONIC
  # :   Use mach\_absolute\_time(), available on Darwin. The resolution is CPU
  #     dependent.
  # :TIMES\_BASED\_CLOCK\_MONOTONIC
  # :   Use the result value of times() defined by POSIX. POSIX defines it as
  #     "times() shall return the elapsed real time, in clock ticks, since an
  #     arbitrary point in the past (for example, system start-up time)". For
  #     example, GNU/Linux returns a value based on jiffies and it is monotonic.
  #     However, 4.4BSD uses gettimeofday() and it is not monotonic. (FreeBSD
  #     uses
  #     [`clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)([`CLOCK_MONOTONIC`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_MONOTONIC))
  #     instead, though.) The resolution is the clock tick. "getconf CLK\_TCK"
  #     command shows the clock ticks per second. (The clock ticks per second is
  #     defined by HZ macro in older systems.) If it is 100 and clock\_t is 32
  #     bits integer type, the resolution is 10 millisecond and cannot represent
  #     over 497 days.
  #
  #
  # Emulations for `CLOCK_PROCESS_CPUTIME_ID`:
  # :GETRUSAGE\_BASED\_CLOCK\_PROCESS\_CPUTIME\_ID
  # :   Use getrusage() defined by SUS. getrusage() is used with RUSAGE\_SELF to
  #     obtain the time only for the calling process (excluding the time for
  #     child processes). The result is addition of user time (ru\_utime) and
  #     system time (ru\_stime). The resolution is 1 microsecond.
  # :TIMES\_BASED\_CLOCK\_PROCESS\_CPUTIME\_ID
  # :   Use times() defined by POSIX. The result is addition of user time
  #     (tms\_utime) and system time (tms\_stime). tms\_cutime and tms\_cstime
  #     are ignored to exclude the time for child processes. The resolution is
  #     the clock tick. "getconf CLK\_TCK" command shows the clock ticks per
  #     second. (The clock ticks per second is defined by HZ macro in older
  #     systems.) If it is 100, the resolution is 10 millisecond.
  # :CLOCK\_BASED\_CLOCK\_PROCESS\_CPUTIME\_ID
  # :   Use clock() defined by ISO C. The resolution is 1/CLOCKS\_PER\_SEC.
  #     CLOCKS\_PER\_SEC is the C-level macro defined by time.h. SUS defines
  #     CLOCKS\_PER\_SEC is 1000000. Non-Unix systems may define it a different
  #     value, though. If CLOCKS\_PER\_SEC is 1000000 as SUS, the resolution is
  #     1 microsecond. If CLOCKS\_PER\_SEC is 1000000 and clock\_t is 32 bits
  #     integer type, it cannot represent over 72 minutes.
  #
  #
  # If the given `clock_id` is not supported, Errno::EINVAL is raised.
  #
  # `unit` specifies a type of the return value.
  #
  # :float\_second
  # :   number of seconds as a float (default)
  # :float\_millisecond
  # :   number of milliseconds as a float
  # :float\_microsecond
  # :   number of microseconds as a float
  # :second
  # :   number of seconds as an integer
  # :millisecond
  # :   number of milliseconds as an integer
  # :microsecond
  # :   number of microseconds as an integer
  # :nanosecond
  # :   number of nanoseconds as an integer
  #
  #
  # The underlying function,
  # [`clock_gettime`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-clock_gettime)(),
  # returns a number of nanoseconds.
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) object (IEEE 754
  # double) is not enough to represent the return value for
  # [`CLOCK_REALTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME).
  # If the exact nanoseconds value is required, use `:nanoseconds` as the
  # `unit`.
  #
  # The origin (zero) of the returned value varies. For example, system start up
  # time, process start up time, the Epoch, etc.
  #
  # The origin in
  # [`CLOCK_REALTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME)
  # is defined as the Epoch (1970-01-01 00:00:00 UTC). But some systems count
  # leap seconds and others doesn't. So the result can be interpreted
  # differently across systems.
  # [`Time.now`](https://docs.ruby-lang.org/en/2.7.0/Time.html#method-c-now) is
  # recommended over
  # [`CLOCK_REALTIME`](https://docs.ruby-lang.org/en/2.7.0/Process.html#CLOCK_REALTIME).
  sig do
    params(
        clock_id: T.any(Symbol, Integer),
        unit: Symbol,
    )
    .returns(T.any(Float, Integer))
  end
  def self.clock_gettime(clock_id, unit=T.unsafe(nil)); end

  # Detach the process from controlling terminal and run in the background as
  # system daemon. Unless the argument nochdir is true (i.e. non false), it
  # changes the current working directory to the root ("/"). Unless the argument
  # noclose is true, daemon() will redirect standard input, standard output and
  # standard error to /dev/null. Return zero on success, or raise one of
  # Errno::\*.
  sig do
    params(
        nochdir: BasicObject,
        noclose: BasicObject,
    )
    .returns(Integer)
  end
  def self.daemon(nochdir=T.unsafe(nil), noclose=T.unsafe(nil)); end

  # Some operating systems retain the status of terminated child processes until
  # the parent collects that status (normally using some variant of `wait()`).
  # If the parent never collects this status, the child stays around as a
  # *zombie* process.
  # [`Process::detach`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-detach)
  # prevents this by setting up a separate Ruby thread whose sole job is to reap
  # the status of the process *pid* when it terminates. Use detach only when you
  # do not intend to explicitly wait for the child to terminate.
  #
  # The waiting thread returns the exit status of the detached process when it
  # terminates, so you can use
  # [`Thread#join`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-join)
  # to know the result. If specified *pid* is not a valid child process ID, the
  # thread returns `nil` immediately.
  #
  # The waiting thread has pid method which returns the pid.
  #
  # In this first example, we don't reap the first child process, so it appears
  # as a zombie in the process status display.
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
  # ```
  # 27389 Z
  # ```
  #
  # In the next example,
  # [`Process::detach`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-detach)
  # is used to reap the child automatically.
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

  # Sets the effective user ID for this process. Not available on all platforms.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.euid=(arg0); end

  # Replaces the current process by running the given external *command*, which
  # can take one of the following forms:
  #
  # `exec(commandline)`
  # :   command line string which is passed to the standard shell
  # `exec(cmdname, arg1, ...)`
  # :   command name and one or more arguments (no shell)
  # `exec([cmdname, argv0], arg1, ...)`
  # :   command name, [argv](0) and zero or more arguments (no shell)
  #
  #
  # In the first form, the string is taken as a command line that is subject to
  # shell expansion before being executed.
  #
  # The standard shell always means `"/bin/sh"` on Unix-like systems, same as
  # `ENV["RUBYSHELL"]` (or `ENV["COMSPEC"]` on Windows NT series), and similar.
  #
  # If the string from the first form (`exec("command")`) follows these simple
  # rules:
  #
  # *   no meta characters
  # *   no shell reserved word and no special built-in
  # *   Ruby invokes the command directly without shell
  #
  #
  # You can force shell invocation by adding ";" to the string (because ";" is a
  # meta character).
  #
  # Note that this behavior is observable by pid obtained (return value of
  # spawn() and
  # [`IO#pid`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pid) for
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen)) is
  # the pid of the invoked command, not shell.
  #
  # In the second form (`exec("command1", "arg1", ...)`), the first is taken as
  # a command name and the rest are passed as parameters to command with no
  # shell expansion.
  #
  # In the third form (`exec(["command", "argv0"], "arg1", ...)`), starting a
  # two-element array at the beginning of the command, the first element is the
  # command to be executed, and the second argument is used as the `argv[0]`
  # value, which may show up in process listings.
  #
  # In order to execute the command, one of the `exec(2)` system calls are used,
  # so the running command may inherit some of the environment of the original
  # program (including open file descriptors).
  #
  # This behavior is modified by the given `env` and `options` parameters. See
  # [`::spawn`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-spawn)
  # for details.
  #
  # If the command fails to execute (typically Errno::ENOENT when it was not
  # found) a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # exception is raised.
  #
  # This method modifies process attributes according to given `options` before
  # `exec(2)` system call. See
  # [`::spawn`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-spawn)
  # for more details about the given `options`.
  #
  # The modified attributes may be retained when `exec(2)` system call fails.
  #
  # For example, hard resource limits are not restorable.
  #
  # Consider to create a child process using
  # [`::spawn`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-spawn)
  # or
  # [`Kernel#system`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-system)
  # if this is not acceptable.
  #
  # ```ruby
  # exec "echo *"       # echoes list of files in current directory
  # # never get here
  #
  # exec "echo", "*"    # echoes an asterisk
  # # never get here
  # ```
  def self.exec(*_); end

  # Returns the process group ID for the given process id. Not available on all
  # platforms.
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

  # Gets the scheduling priority for specified process, process group, or user.
  # *kind* indicates the kind of entity to find: one of
  # [`Process::PRIO_PGRP`](https://docs.ruby-lang.org/en/2.7.0/Process.html#PRIO_PGRP),
  # [`Process::PRIO_USER`](https://docs.ruby-lang.org/en/2.7.0/Process.html#PRIO_USER),
  # or
  # [`Process::PRIO_PROCESS`](https://docs.ruby-lang.org/en/2.7.0/Process.html#PRIO_PROCESS).
  # *integer* is an id indicating the particular process, process group, or user
  # (an id of 0 means *current*). Lower priorities are more favorable for
  # scheduling. Not available on all platforms.
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

  # Gets the resource limit of the process. *cur\_limit* means current (soft)
  # limit and *max\_limit* means maximum (hard) limit.
  #
  # *resource* indicates the kind of resource to limit. It is specified as a
  # symbol such as `:CORE`, a string such as `"CORE"` or a constant such as
  # [`Process::RLIMIT_CORE`](https://docs.ruby-lang.org/en/2.7.0/Process.html#RLIMIT_CORE).
  # See
  # [`Process.setrlimit`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setrlimit)
  # for details.
  #
  # *cur\_limit* and *max\_limit* may be
  # [`Process::RLIM_INFINITY`](https://docs.ruby-lang.org/en/2.7.0/Process.html#RLIM_INFINITY),
  # [`Process::RLIM_SAVED_MAX`](https://docs.ruby-lang.org/en/2.7.0/Process.html#RLIM_SAVED_MAX)
  # or
  # [`Process::RLIM_SAVED_CUR`](https://docs.ruby-lang.org/en/2.7.0/Process.html#RLIM_SAVED_CUR).
  # See
  # [`Process.setrlimit`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setrlimit)
  # and the system getrlimit(2) manual for details.
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

  # Get an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the
  # group IDs in the supplemental group access list for this process.
  #
  # ```ruby
  # Process.groups   #=> [27, 6, 10, 11]
  # ```
  #
  # Note that this method is just a wrapper of getgroups(2). This means that the
  # following characteristics of the result completely depend on your system:
  #
  # *   the result is sorted
  # *   the result includes effective GIDs
  # *   the result does not include duplicated GIDs
  #
  #
  # You can make sure to get a sorted unique GID list of the current process by
  # this expression:
  #
  # ```ruby
  # Process.groups.uniq.sort
  # ```
  sig {returns(T::Array[Integer])}
  def self.groups(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the supplemental group
  # access list to the given
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of group IDs.
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

  # Initializes the supplemental group access list by reading the system group
  # database and using all groups of which the given user is a member. The group
  # with the specified *gid* is also added to the list. Returns the resulting
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the gids of all
  # the groups in the supplementary group access list. Not available on all
  # platforms.
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

  # Sends the given signal to the specified process id(s) if *pid* is positive.
  # If *pid* is zero, *signal* is sent to all processes whose group ID is equal
  # to the group ID of the process. If *pid* is negative, results are dependent
  # on the operating system. *signal* may be an integer signal number or a POSIX
  # signal name (either with or without a `SIG` prefix). If *signal* is negative
  # (or starts with a minus sign), kills process groups instead of processes.
  # Not all signals are available on all platforms. The keys and values of
  # [`Signal.list`](https://docs.ruby-lang.org/en/2.7.0/Signal.html#method-c-list)
  # are known signal names and numbers, respectively.
  #
  # ```ruby
  # pid = fork do
  #    Signal.trap("HUP") { puts "Ouch!"; exit }
  #    # ... do some work ...
  # end
  # # ...
  # Process.kill("HUP", pid)
  # Process.wait
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Ouch!
  # ```
  #
  # If *signal* is an integer but wrong for signal, Errno::EINVAL or
  # [`RangeError`](https://docs.ruby-lang.org/en/2.7.0/RangeError.html) will be
  # raised. Otherwise unless *signal* is a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or a
  # [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html), and a known
  # signal name,
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html)
  # will be raised.
  #
  # Also, Errno::ESRCH or
  # [`RangeError`](https://docs.ruby-lang.org/en/2.7.0/RangeError.html) for
  # invalid *pid*, Errno::EPERM when failed because of no privilege, will be
  # raised. In these cases, signals may have been sent to preceding processes.
  sig do
    params(
        signal: T.any(Integer, Symbol, String),
        pids: Integer,
    )
    .returns(Integer)
  end
  def self.kill(signal, *pids); end

  # Returns the status of the last executed child process in the current thread.
  #
  # ```ruby
  # Process.wait Process.spawn("ruby", "-e", "exit 13")
  # Process.last_status   #=> #<Process::Status: pid 4825 exit 13>
  # ```
  #
  # If no child process has ever been executed in the current thread, this
  # returns `nil`.
  #
  # ```ruby
  # Process.last_status   #=> nil
  # ```
  def self.last_status; end

  # Returns the maximum number of gids allowed in the supplemental group access
  # list.
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

  # Returns the process id of the parent of this process. Returns untrustworthy
  # value on Win32/64. Not available on all platforms.
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

  # Sets the process group ID of *pid* (0 indicates this process) to *integer*.
  # Not available on all platforms.
  sig do
    params(
        pid: Integer,
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.setpgid(pid, arg0); end

  # Equivalent to `setpgid(0,0)`. Not available on all platforms.
  def self.setpgrp; end

  # See
  # [`Process.getpriority`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-getpriority).
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

  # Sets the process title that appears on the ps(1) command. Not necessarily
  # effective on all platforms. No exception will be raised regardless of the
  # result, nor will
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html)
  # be raised even if the platform does not support the feature.
  #
  # Calling this method does not affect the value of $0.
  #
  # ```ruby
  # Process.setproctitle('myapp: worker #%d' % worker_id)
  # ```
  #
  # This method first appeared in Ruby 2.1 to serve as a global variable free
  # means to change the process title.
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def self.setproctitle(arg0); end

  # Sets the resource limit of the process. *cur\_limit* means current (soft)
  # limit and *max\_limit* means maximum (hard) limit.
  #
  # If *max\_limit* is not given, *cur\_limit* is used.
  #
  # *resource* indicates the kind of resource to limit. It should be a symbol
  # such as `:CORE`, a string such as `"CORE"` or a constant such as
  # [`Process::RLIMIT_CORE`](https://docs.ruby-lang.org/en/2.7.0/Process.html#RLIMIT_CORE).
  # The available resources are OS dependent. Ruby may support following
  # resources.
  #
  # AS
  # :   total available memory (bytes) (SUSv3, NetBSD, FreeBSD, OpenBSD but
  #     4.4BSD-Lite)
  # CORE
  # :   core size (bytes) (SUSv3)
  # CPU
  # :   CPU time (seconds) (SUSv3)
  # DATA
  # :   data segment (bytes) (SUSv3)
  # FSIZE
  # :   file size (bytes) (SUSv3)
  # MEMLOCK
  # :   total size for mlock(2) (bytes) (4.4BSD, GNU/Linux)
  # MSGQUEUE
  # :   allocation for POSIX message queues (bytes) (GNU/Linux)
  # NICE
  # :   ceiling on process's nice(2) value (number) (GNU/Linux)
  # NOFILE
  # :   file descriptors (number) (SUSv3)
  # NPROC
  # :   number of processes for the user (number) (4.4BSD, GNU/Linux)
  # [`RSS`](https://docs.ruby-lang.org/en/2.7.0/RSS.html)
  # :   resident memory size (bytes) (4.2BSD, GNU/Linux)
  # RTPRIO
  # :   ceiling on the process's real-time priority (number) (GNU/Linux)
  # RTTIME
  # :   CPU time for real-time process (us) (GNU/Linux)
  # SBSIZE
  # :   all socket buffers (bytes) (NetBSD, FreeBSD)
  # SIGPENDING
  # :   number of queued signals allowed (signals) (GNU/Linux)
  # STACK
  # :   stack size (bytes) (SUSv3)
  #
  #
  # *cur\_limit* and *max\_limit* may be `:INFINITY`, `"INFINITY"` or
  # [`Process::RLIM_INFINITY`](https://docs.ruby-lang.org/en/2.7.0/Process.html#RLIM_INFINITY),
  # which means that the resource is not limited. They may be
  # [`Process::RLIM_SAVED_MAX`](https://docs.ruby-lang.org/en/2.7.0/Process.html#RLIM_SAVED_MAX),
  # [`Process::RLIM_SAVED_CUR`](https://docs.ruby-lang.org/en/2.7.0/Process.html#RLIM_SAVED_CUR)
  # and corresponding symbols and strings too. See system setrlimit(2) manual
  # for details.
  #
  # The following example raises the soft limit of core size to the hard limit
  # to try to make core dump possible.
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

  # Establishes this process as a new session and process group leader, with no
  # controlling tty. Returns the session id. Not available on all platforms.
  #
  # ```ruby
  # Process.setsid   #=> 27422
  # ```
  sig {returns(Integer)}
  def self.setsid(); end

  # spawn executes specified command and return its pid.
  #
  # ```ruby
  # pid = spawn("tar xf ruby-2.0.0-p195.tar.bz2")
  # Process.wait pid
  #
  # pid = spawn(RbConfig.ruby, "-eputs'Hello, world!'")
  # Process.wait pid
  # ```
  #
  # This method is similar to
  # [`Kernel#system`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-system)
  # but it doesn't wait for the command to finish.
  #
  # The parent process should use
  # [`Process.wait`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-wait)
  # to collect the termination status of its child or use
  # [`Process.detach`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-detach)
  # to register disinterest in their status; otherwise, the operating system may
  # accumulate zombie processes.
  #
  # spawn has bunch of options to specify process attributes:
  #
  # ```
  # env: hash
  #   name => val : set the environment variable
  #   name => nil : unset the environment variable
  #
  #   the keys and the values except for +nil+ must be strings.
  # command...:
  #   commandline                 : command line string which is passed to the standard shell
  #   cmdname, arg1, ...          : command name and one or more arguments (This form does not use the shell. See below for caveats.)
  #   [cmdname, argv0], arg1, ... : command name, argv[0] and zero or more arguments (no shell)
  # options: hash
  #   clearing environment variables:
  #     :unsetenv_others => true   : clear environment variables except specified by env
  #     :unsetenv_others => false  : don't clear (default)
  #   process group:
  #     :pgroup => true or 0 : make a new process group
  #     :pgroup => pgid      : join the specified process group
  #     :pgroup => nil       : don't change the process group (default)
  #   create new process group: Windows only
  #     :new_pgroup => true  : the new process is the root process of a new process group
  #     :new_pgroup => false : don't create a new process group (default)
  #   resource limit: resourcename is core, cpu, data, etc.  See Process.setrlimit.
  #     :rlimit_resourcename => limit
  #     :rlimit_resourcename => [cur_limit, max_limit]
  #   umask:
  #     :umask => int
  #   redirection:
  #     key:
  #       FD              : single file descriptor in child process
  #       [FD, FD, ...]   : multiple file descriptor in child process
  #     value:
  #       FD                        : redirect to the file descriptor in parent process
  #       string                    : redirect to file with open(string, "r" or "w")
  #       [string]                  : redirect to file with open(string, File::RDONLY)
  #       [string, open_mode]       : redirect to file with open(string, open_mode, 0644)
  #       [string, open_mode, perm] : redirect to file with open(string, open_mode, perm)
  #       [:child, FD]              : redirect to the redirected file descriptor
  #       :close                    : close the file descriptor in child process
  #     FD is one of follows
  #       :in     : the file descriptor 0 which is the standard input
  #       :out    : the file descriptor 1 which is the standard output
  #       :err    : the file descriptor 2 which is the standard error
  #       integer : the file descriptor of specified the integer
  #       io      : the file descriptor specified as io.fileno
  #   file descriptor inheritance: close non-redirected non-standard fds (3, 4, 5, ...) or not
  #     :close_others => false  : inherit
  #   current directory:
  #     :chdir => str
  # ```
  #
  # The `cmdname, arg1, ...` form does not use the shell. However, on different
  # OSes, different things are provided as built-in commands. An example of this
  # is +'echo'+, which is a built-in on Windows, but is a normal program on
  # Linux and Mac OS X. This means that `Process.spawn 'echo', '%Path%'` will
  # display the contents of the `%Path%` environment variable on Windows, but
  # `Process.spawn 'echo', '$PATH'` prints the literal `$PATH`.
  #
  # If a hash is given as `env`, the environment is updated by `env` before
  # `exec(2)` in the child process. If a pair in `env` has nil as the value, the
  # variable is deleted.
  #
  # ```ruby
  # # set FOO as BAR and unset BAZ.
  # pid = spawn({"FOO"=>"BAR", "BAZ"=>nil}, command)
  # ```
  #
  # If a hash is given as `options`, it specifies process group, create new
  # process group, resource limit, current directory, umask and redirects for
  # the child process. Also, it can be specified to clear environment variables.
  #
  # The `:unsetenv_others` key in `options` specifies to clear environment
  # variables, other than specified by `env`.
  #
  # ```ruby
  # pid = spawn(command, :unsetenv_others=>true) # no environment variable
  # pid = spawn({"FOO"=>"BAR"}, command, :unsetenv_others=>true) # FOO only
  # ```
  #
  # The `:pgroup` key in `options` specifies a process group. The corresponding
  # value should be true, zero, a positive integer, or nil. true and zero cause
  # the process to be a process leader of a new process group. A non-zero
  # positive integer causes the process to join the provided process group. The
  # default value, nil, causes the process to remain in the same process group.
  #
  # ```ruby
  # pid = spawn(command, :pgroup=>true) # process leader
  # pid = spawn(command, :pgroup=>10) # belongs to the process group 10
  # ```
  #
  # The `:new_pgroup` key in `options` specifies to pass
  # `CREATE_NEW_PROCESS_GROUP` flag to `CreateProcessW()` that is Windows API.
  # This option is only for Windows. true means the new process is the root
  # process of the new process group. The new process has CTRL+C disabled. This
  # flag is necessary for `Process.kill(:SIGINT, pid)` on the subprocess.
  # :new\_pgroup is false by default.
  #
  # ```ruby
  # pid = spawn(command, :new_pgroup=>true)  # new process group
  # pid = spawn(command, :new_pgroup=>false) # same process group
  # ```
  #
  # The `:rlimit_`*foo* key specifies a resource limit. *foo* should be one of
  # resource types such as `core`. The corresponding value should be an integer
  # or an array which have one or two integers: same as cur\_limit and
  # max\_limit arguments for
  # [`Process.setrlimit`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-setrlimit).
  #
  # ```ruby
  # cur, max = Process.getrlimit(:CORE)
  # pid = spawn(command, :rlimit_core=>[0,max]) # disable core temporary.
  # pid = spawn(command, :rlimit_core=>max) # enable core dump
  # pid = spawn(command, :rlimit_core=>0) # never dump core.
  # ```
  #
  # The `:umask` key in `options` specifies the umask.
  #
  # ```ruby
  # pid = spawn(command, :umask=>077)
  # ```
  #
  # The :in, :out, :err, an integer, an
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) and an array key
  # specifies a redirection. The redirection maps a file descriptor in the child
  # process.
  #
  # For example, stderr can be merged into stdout as follows:
  #
  # ```ruby
  # pid = spawn(command, :err=>:out)
  # pid = spawn(command, 2=>1)
  # pid = spawn(command, STDERR=>:out)
  # pid = spawn(command, STDERR=>STDOUT)
  # ```
  #
  # The hash keys specifies a file descriptor in the child process started by
  # spawn. :err, 2 and STDERR specifies the standard error stream (stderr).
  #
  # The hash values specifies a file descriptor in the parent process which
  # invokes spawn. :out, 1 and STDOUT specifies the standard output stream
  # (stdout).
  #
  # In the above example, the standard output in the child process is not
  # specified. So it is inherited from the parent process.
  #
  # The standard input stream (stdin) can be specified by :in, 0 and STDIN.
  #
  # A filename can be specified as a hash value.
  #
  # ```ruby
  # pid = spawn(command, :in=>"/dev/null") # read mode
  # pid = spawn(command, :out=>"/dev/null") # write mode
  # pid = spawn(command, :err=>"log") # write mode
  # pid = spawn(command, [:out, :err]=>"/dev/null") # write mode
  # pid = spawn(command, 3=>"/dev/null") # read mode
  # ```
  #
  # For stdout and stderr (and combination of them), it is opened in write mode.
  # Otherwise read mode is used.
  #
  # For specifying flags and permission of file creation explicitly, an array is
  # used instead.
  #
  # ```ruby
  # pid = spawn(command, :in=>["file"]) # read mode is assumed
  # pid = spawn(command, :in=>["file", "r"])
  # pid = spawn(command, :out=>["log", "w"]) # 0644 assumed
  # pid = spawn(command, :out=>["log", "w", 0600])
  # pid = spawn(command, :out=>["log", File::WRONLY|File::EXCL|File::CREAT, 0600])
  # ```
  #
  # The array specifies a filename, flags and permission. The flags can be a
  # string or an integer. If the flags is omitted or nil, File::RDONLY is
  # assumed. The permission should be an integer. If the permission is omitted
  # or nil, 0644 is assumed.
  #
  # If an array of IOs and integers are specified as a hash key, all the
  # elements are redirected.
  #
  # ```ruby
  # # stdout and stderr is redirected to log file.
  # # The file "log" is opened just once.
  # pid = spawn(command, [:out, :err]=>["log", "w"])
  # ```
  #
  # Another way to merge multiple file descriptors is [:child, fd]. [:child, fd]
  # means the file descriptor in the child process. This is different from fd.
  # For example, :err=>:out means redirecting child stderr to parent stdout. But
  # :err=>[:child, :out] means redirecting child stderr to child stdout. They
  # differ if stdout is redirected in the child process as follows.
  #
  # ```ruby
  # # stdout and stderr is redirected to log file.
  # # The file "log" is opened just once.
  # pid = spawn(command, :out=>["log", "w"], :err=>[:child, :out])
  # ```
  #
  # [:child, :out] can be used to merge stderr into stdout in
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen). In
  # this case,
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen)
  # redirects stdout to a pipe in the child process and [:child, :out] refers
  # the redirected stdout.
  #
  # ```ruby
  # io = IO.popen(["sh", "-c", "echo out; echo err >&2", :err=>[:child, :out]])
  # p io.read #=> "out\nerr\n"
  # ```
  #
  # The `:chdir` key in `options` specifies the current directory.
  #
  # ```ruby
  # pid = spawn(command, :chdir=>"/var/tmp")
  # ```
  #
  # spawn closes all non-standard unspecified descriptors by default. The
  # "standard" descriptors are 0, 1 and 2. This behavior is specified by
  # :close\_others option. :close\_others doesn't affect the standard
  # descriptors which are closed only if :close is specified explicitly.
  #
  # ```ruby
  # pid = spawn(command, :close_others=>true)  # close 3,4,5,... (default)
  # pid = spawn(command, :close_others=>false) # don't close 3,4,5,...
  # ```
  #
  # :close\_others is false by default for spawn and
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen).
  #
  # Note that fds which close-on-exec flag is already set are closed regardless
  # of :close\_others option.
  #
  # So [`IO.pipe`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-pipe)
  # and spawn can be used as
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen).
  #
  # ```ruby
  # # similar to r = IO.popen(command)
  # r, w = IO.pipe
  # pid = spawn(command, :out=>w)   # r, w is closed in the child process.
  # w.close
  # ```
  #
  # :close is specified as a hash value to close a fd individually.
  #
  # ```ruby
  # f = open(foo)
  # system(command, f=>:close)        # don't inherit f.
  # ```
  #
  # If a file descriptor need to be inherited, io=>io can be used.
  #
  # ```ruby
  # # valgrind has --log-fd option for log destination.
  # # log_w=>log_w indicates log_w.fileno inherits to child process.
  # log_r, log_w = IO.pipe
  # pid = spawn("valgrind", "--log-fd=#{log_w.fileno}", "echo", "a", log_w=>log_w)
  # log_w.close
  # p log_r.read
  # ```
  #
  # It is also possible to exchange file descriptors.
  #
  # ```ruby
  # pid = spawn(command, :out=>:err, :err=>:out)
  # ```
  #
  # The hash keys specify file descriptors in the child process. The hash values
  # specifies file descriptors in the parent process. So the above specifies
  # exchanging stdout and stderr. Internally, `spawn` uses an extra file
  # descriptor to resolve such cyclic file descriptor mapping.
  #
  # See
  # [`Kernel.exec`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-exec)
  # for the standard shell.
  sig do
    params(
      args: T.any(
        String,
        T::Hash[String, T.untyped]
      )
    ).returns(Integer)
  end
  def self.spawn(*args); end

  # Returns a `Tms` structure (see Process::Tms) that contains user and system
  # CPU times for this process, and also for children processes.
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

  # Sets the (user) user ID for this process. Not available on all platforms.
  sig do
    params(
        user: Integer,
    )
    .returns(Integer)
  end
  def self.uid=(user); end

  # Waits for a child process to exit, returns its process id, and sets `$?` to
  # a
  # [`Process::Status`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html)
  # object containing information on that process. Which child it waits on
  # depends on the value of *pid*:
  #
  # > 0
  # :   Waits for the child whose process ID equals *pid*.
  #
  # 0
  # :   Waits for any child whose process group ID equals that of the calling
  #     process.
  #
  # -1
  # :   Waits for any child process (the default if no *pid* is given).
  #
  # < -1
  # :   Waits for any child whose process group ID equals the absolute value of
  #     *pid*.
  #
  #
  # The *flags* argument may be a logical or of the flag values
  # [`Process::WNOHANG`](https://docs.ruby-lang.org/en/2.7.0/Process.html#WNOHANG)
  # (do not block if no child available) or
  # [`Process::WUNTRACED`](https://docs.ruby-lang.org/en/2.7.0/Process.html#WUNTRACED)
  # (return stopped children that haven't been reported). Not all flags are
  # available on all platforms, but a flag value of zero will work on all
  # platforms.
  #
  # Calling this method raises a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
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
    .returns(T.nilable(Integer))
  end
  def self.wait(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  # Waits for a child process to exit (see
  # [`Process::waitpid`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-waitpid)
  # for exact semantics) and returns an array containing the process id and the
  # exit status (a
  # [`Process::Status`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html)
  # object) of that child. Raises a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
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
    .returns(T.nilable([Integer, Process::Status]))
  end
  def self.wait2(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  # Waits for all children, returning an array of *pid*/*status* pairs (where
  # *status* is a
  # [`Process::Status`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html)
  # object).
  #
  # ```ruby
  # fork { sleep 0.2; exit 2 }   #=> 27432
  # fork { sleep 0.1; exit 1 }   #=> 27433
  # fork {            exit 0 }   #=> 27434
  # p Process.waitall
  # ```
  #
  # *produces*:
  #
  # ```
  # [[30982, #<Process::Status: pid 30982 exit 0>],
  #  [30979, #<Process::Status: pid 30979 exit 1>],
  #  [30976, #<Process::Status: pid 30976 exit 2>]]
  # ```
  sig {returns(T::Array[[Integer, Process::Status]])}
  def self.waitall(); end

  # Waits for a child process to exit, returns its process id, and sets `$?` to
  # a
  # [`Process::Status`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html)
  # object containing information on that process. Which child it waits on
  # depends on the value of *pid*:
  #
  # > 0
  # :   Waits for the child whose process ID equals *pid*.
  #
  # 0
  # :   Waits for any child whose process group ID equals that of the calling
  #     process.
  #
  # -1
  # :   Waits for any child process (the default if no *pid* is given).
  #
  # < -1
  # :   Waits for any child whose process group ID equals the absolute value of
  #     *pid*.
  #
  #
  # The *flags* argument may be a logical or of the flag values
  # [`Process::WNOHANG`](https://docs.ruby-lang.org/en/2.7.0/Process.html#WNOHANG)
  # (do not block if no child available) or
  # [`Process::WUNTRACED`](https://docs.ruby-lang.org/en/2.7.0/Process.html#WUNTRACED)
  # (return stopped children that haven't been reported). Not all flags are
  # available on all platforms, but a flag value of zero will work on all
  # platforms.
  #
  # Calling this method raises a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
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
    .returns(T.nilable(Integer))
  end
  def self.waitpid(pid=T.unsafe(nil), flags=T.unsafe(nil)); end

  # Waits for a child process to exit (see
  # [`Process::waitpid`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-waitpid)
  # for exact semantics) and returns an array containing the process id and the
  # exit status (a
  # [`Process::Status`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html)
  # object) of that child. Raises a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
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
    .returns(T.nilable([Integer, Process::Status]))
  end
  def self.waitpid2(pid=T.unsafe(nil), flags=T.unsafe(nil)); end
end

# The [`Process::GID`](https://docs.ruby-lang.org/en/2.7.0/Process/GID.html)
# module contains a collection of module functions which can be used to portably
# get, set, and switch the current process's real, effective, and saved group
# IDs.
module Process::GID
  # Change the current process's real and effective group ID to that specified
  # by *group*. Returns the new group ID. Not available on all platforms.
  #
  # ```ruby
  # [Process.gid, Process.egid]          #=> [0, 0]
  # Process::GID.change_privilege(33)    #=> 33
  # [Process.gid, Process.egid]          #=> [33, 33]
  # ```
  sig do
    params(
        group: Integer,
    )
    .returns(Integer)
  end
  def self.change_privilege(group); end

  # Returns the effective group ID for this process. Not available on all
  # platforms.
  #
  # ```ruby
  # Process.egid   #=> 500
  # ```
  sig {returns(Integer)}
  def self.eid(); end

  # Get the group ID by the *name*. If the group is not found, `ArgumentError`
  # will be raised.
  #
  # ```ruby
  # Process::GID.from_name("wheel") #=> 0
  # Process::GID.from_name("nosuchgroup") #=> can't find group for nosuchgroup (ArgumentError)
  # ```
  sig do
    params(
        name: String,
    )
    .returns(Integer)
  end
  def self.from_name(name); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the effective group
  # ID, and if possible, the saved group ID of the process to the given *group*.
  # Returns the new effective group ID. Not available on all platforms.
  #
  # ```ruby
  # [Process.gid, Process.egid]          #=> [0, 0]
  # Process::GID.grant_privilege(31)     #=> 33
  # [Process.gid, Process.egid]          #=> [0, 33]
  # ```
  sig do
    params(
        group: Integer,
    )
    .returns(Integer)
  end
  def self.grant_privilege(group); end

  # Exchange real and effective group IDs and return the new effective group ID.
  # Not available on all platforms.
  #
  # ```ruby
  # [Process.gid, Process.egid]   #=> [0, 33]
  # Process::GID.re_exchange      #=> 0
  # [Process.gid, Process.egid]   #=> [33, 0]
  # ```
  sig {returns(Integer)}
  def self.re_exchange(); end

  # Returns `true` if the real and effective group IDs of a process may be
  # exchanged on the current platform.
  sig {returns(T::Boolean)}
  def self.re_exchangeable?(); end

  # Returns the (real) group ID for this process.
  #
  # ```ruby
  # Process.gid   #=> 500
  # ```
  sig {returns(Integer)}
  def self.rid(); end

  # Returns `true` if the current platform has saved group ID functionality.
  sig {returns(T::Boolean)}
  def self.sid_available?(); end

  # Switch the effective and real group IDs of the current process. If a *block*
  # is given, the group IDs will be switched back after the block is executed.
  # Returns the new effective group ID if called without a block, and the return
  # value of the block if one is given.
  sig {returns(Integer)}
  sig do
    type_parameters(:T).params(
        blk: T.proc.returns(T.type_parameter(:T)),
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

# [`Process::Status`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html)
# encapsulates the information on the status of a running or terminated system
# process. The built-in variable `$?` is either `nil` or a
# [`Process::Status`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html)
# object.
#
# ```ruby
# fork { exit 99 }   #=> 26557
# Process.wait       #=> 26557
# $?.class           #=> Process::Status
# $?.to_i            #=> 25344
# $? >> 8            #=> 99
# $?.stopped?        #=> false
# $?.exited?         #=> true
# $?.exitstatus      #=> 99
# ```
#
# Posix systems record information on processes using a 16-bit integer. The
# lower bits record the process status (stopped, exited, signaled) and the upper
# bits possibly contain additional information (for example the program's return
# code in the case of exited processes). Pre Ruby 1.8, these bits were exposed
# directly to the Ruby program. Ruby now encapsulates these in a
# [`Process::Status`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html)
# object. To maximize compatibility, however, these objects retain a
# bit-oriented interface. In the descriptions that follow, when we talk about
# the integer value of *stat*, we're referring to this 16 bit value.
class Process::Status < Object
  # Logical AND of the bits in *stat* with *num*.
  #
  # ```ruby
  # fork { exit 0x37 }
  # Process.wait
  # sprintf('%04x', $?.to_i)       #=> "3700"
  # sprintf('%04x', $? & 0x1e00)   #=> "1600"
  # ```
  sig do
    params(
        num: Integer,
    )
    .returns(Integer)
  end
  def &(num); end

  # Returns `true` if the integer value of *stat* equals *other*.
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end

  # Shift the bits in *stat* right *num* places.
  #
  # ```ruby
  # fork { exit 99 }   #=> 26563
  # Process.wait       #=> 26563
  # $?.to_i            #=> 25344
  # $? >> 8            #=> 99
  # ```
  sig do
    params(
        num: Integer,
    )
    .returns(Integer)
  end
  def >>(num); end

  # Returns `true` if *stat* generated a coredump when it terminated. Not
  # available on all platforms.
  sig {returns(T::Boolean)}
  def coredump?(); end

  # Returns `true` if *stat* exited normally (for example using an `exit()` call
  # or finishing the program).
  sig {returns(T::Boolean)}
  def exited?(); end

  # Returns the least significant eight bits of the return code of *stat*. Only
  # available if
  # [`exited?`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html#method-i-exited-3F)
  # is `true`.
  #
  # ```ruby
  # fork { }           #=> 26572
  # Process.wait       #=> 26572
  # $?.exited?         #=> true
  # $?.exitstatus      #=> 0
  #
  # fork { exit 99 }   #=> 26573
  # Process.wait       #=> 26573
  # $?.exited?         #=> true
  # $?.exitstatus      #=> 99
  # ```
  sig {returns(T.nilable(Integer))}
  def exitstatus(); end

  # Override the inspection method.
  #
  # ```ruby
  # system("false")
  # p $?.inspect #=> "#<Process::Status: pid 12861 exit 1>"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns the process ID that this status object represents.
  #
  # ```ruby
  # fork { exit }   #=> 26569
  # Process.wait    #=> 26569
  # $?.pid          #=> 26569
  # ```
  sig {returns(Integer)}
  def pid(); end

  # Returns `true` if *stat* terminated because of an uncaught signal.
  sig {returns(T::Boolean)}
  def signaled?(); end

  # Returns `true` if this process is stopped. This is only returned if the
  # corresponding wait call had the Process::WUNTRACED flag set.
  sig {returns(T::Boolean)}
  def stopped?(); end

  # Returns the number of the signal that caused *stat* to stop (or `nil` if
  # self is not stopped).
  sig {returns(T.nilable(Integer))}
  def stopsig(); end

  # Returns `true` if *stat* is successful, `false` if not. Returns `nil` if
  # [`exited?`](https://docs.ruby-lang.org/en/2.7.0/Process/Status.html#method-i-exited-3F)
  # is not `true`.
  sig {returns(T.nilable(T::Boolean))}
  def success?(); end

  # Returns the number of the signal that caused *stat* to terminate (or `nil`
  # if self was not terminated by an uncaught signal).
  sig {returns(T.nilable(Integer))}
  def termsig(); end

  # Returns the bits in *stat* as a
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html). Poking around
  # in these bits is platform dependent.
  #
  # ```ruby
  # fork { exit 0xab }         #=> 26566
  # Process.wait               #=> 26566
  # sprintf('%04x', $?.to_i)   #=> "ab00"
  # ```
  sig {returns(Integer)}
  def to_i(); end

  # Show pid and exit status as a string.
  #
  # ```ruby
  # system("false")
  # p $?.to_s         #=> "pid 12766 exit 1"
  # ```
  sig {returns(String)}
  def to_s(); end
end

# The [`Process::Sys`](https://docs.ruby-lang.org/en/2.7.0/Process/Sys.html)
# module contains UID and GID functions which provide direct bindings to the
# system calls of the same names instead of the more-portable versions of the
# same functionality found in the
# [`Process`](https://docs.ruby-lang.org/en/2.7.0/Process.html),
# [`Process::UID`](https://docs.ruby-lang.org/en/2.7.0/Process/UID.html), and
# [`Process::GID`](https://docs.ruby-lang.org/en/2.7.0/Process/GID.html)
# modules.
module Process::Sys
  # Returns the effective group ID for this process. Not available on all
  # platforms.
  #
  # ```ruby
  # Process.egid   #=> 500
  # ```
  def self.getegid; end

  # Returns the effective user ID for this process.
  #
  # ```ruby
  # Process.euid   #=> 501
  # ```
  sig {returns(Integer)}
  def self.geteuid(); end

  # Returns the (real) group ID for this process.
  #
  # ```ruby
  # Process.gid   #=> 500
  # ```
  sig {returns(Integer)}
  def self.getgid(); end

  # Returns the (real) user ID of this process.
  #
  # ```ruby
  # Process.uid   #=> 501
  # ```
  sig {returns(Integer)}
  def self.getuid(); end

  # Returns `true` if the process was created as a result of an execve(2) system
  # call which had either of the setuid or setgid bits set (and extra privileges
  # were given as a result) or if it has changed any of its real, effective or
  # saved user or group IDs since it began execution.
  sig {returns(T::Boolean)}
  def self.issetugid(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the effective group ID
  # of the calling process to *group*. Not available on all platforms.
  sig do
    params(
        group: Integer,
    )
    .returns(NilClass)
  end
  def self.setegid(group); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the effective user ID
  # of the calling process to *user*. Not available on all platforms.
  sig do
    params(
        user: Integer,
    )
    .returns(NilClass)
  end
  def self.seteuid(user); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the group ID of the
  # current process to *group*. Not available on all platforms.
  sig do
    params(
        group: Integer,
    )
    .returns(NilClass)
  end
  def self.setgid(group); end

  # Sets the (group) real and/or effective group IDs of the current process to
  # *rid* and *eid*, respectively. A value of `-1` for either means to leave
  # that ID unchanged. Not available on all platforms.
  sig do
    params(
        rid: Integer,
        eid: Integer,
    )
    .returns(NilClass)
  end
  def self.setregid(rid, eid); end

  # Sets the (group) real, effective, and saved user IDs of the current process
  # to *rid*, *eid*, and *sid* respectively. A value of `-1` for any value means
  # to leave that ID unchanged. Not available on all platforms.
  sig do
    params(
        rid: Integer,
        eid: Integer,
        sid: Integer,
    )
    .returns(NilClass)
  end
  def self.setresgid(rid, eid, sid); end

  # Sets the (user) real, effective, and saved user IDs of the current process
  # to *rid*, *eid*, and *sid* respectively. A value of `-1` for any value means
  # to leave that ID unchanged. Not available on all platforms.
  sig do
    params(
        rid: Integer,
        eid: Integer,
        sid: Integer,
    )
    .returns(NilClass)
  end
  def self.setresuid(rid, eid, sid); end

  # Sets the (user) real and/or effective user IDs of the current process to
  # *rid* and *eid*, respectively. A value of `-1` for either means to leave
  # that ID unchanged. Not available on all platforms.
  sig do
    params(
        rid: Integer,
        eid: Integer,
    )
    .returns(NilClass)
  end
  def self.setreuid(rid, eid); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the real group ID of
  # the calling process to *group*. Not available on all platforms.
  sig do
    params(
        group: Integer,
    )
    .returns(NilClass)
  end
  def self.setrgid(group); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the real user ID of
  # the calling process to *user*. Not available on all platforms.
  sig do
    params(
        user: Integer,
    )
    .returns(NilClass)
  end
  def self.setruid(user); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the user ID of the
  # current process to *user*. Not available on all platforms.
  sig do
    params(
        user: Integer,
    )
    .returns(NilClass)
  end
  def self.setuid(user); end
end

# The [`Process::UID`](https://docs.ruby-lang.org/en/2.7.0/Process/UID.html)
# module contains a collection of module functions which can be used to portably
# get, set, and switch the current process's real, effective, and saved user
# IDs.
module Process::UID
  # Change the current process's real and effective user ID to that specified by
  # *user*. Returns the new user ID. Not available on all platforms.
  #
  # ```ruby
  # [Process.uid, Process.euid]          #=> [0, 0]
  # Process::UID.change_privilege(31)    #=> 31
  # [Process.uid, Process.euid]          #=> [31, 31]
  # ```
  sig do
    params(
        user: Integer,
    )
    .returns(Integer)
  end
  def self.change_privilege(user); end

  # Returns the effective user ID for this process.
  #
  # ```ruby
  # Process.euid   #=> 501
  # ```
  sig {returns(Integer)}
  def self.eid(); end

  # Get the user ID by the *name*. If the user is not found, `ArgumentError`
  # will be raised.
  #
  # ```ruby
  # Process::UID.from_name("root") #=> 0
  # Process::UID.from_name("nosuchuser") #=> can't find user for nosuchuser (ArgumentError)
  # ```
  sig do
    params(
        name: String,
    )
    .returns(Integer)
  end
  def self.from_name(name); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the effective user ID,
  # and if possible, the saved user ID of the process to the given *user*.
  # Returns the new effective user ID. Not available on all platforms.
  #
  # ```ruby
  # [Process.uid, Process.euid]          #=> [0, 0]
  # Process::UID.grant_privilege(31)     #=> 31
  # [Process.uid, Process.euid]          #=> [0, 31]
  # ```
  sig do
    params(
        user: Integer,
    )
    .returns(Integer)
  end
  def self.grant_privilege(user); end

  # Exchange real and effective user IDs and return the new effective user ID.
  # Not available on all platforms.
  #
  # ```ruby
  # [Process.uid, Process.euid]   #=> [0, 31]
  # Process::UID.re_exchange      #=> 0
  # [Process.uid, Process.euid]   #=> [31, 0]
  # ```
  sig {returns(Integer)}
  def self.re_exchange(); end

  # Returns `true` if the real and effective user IDs of a process may be
  # exchanged on the current platform.
  sig {returns(T::Boolean)}
  def self.re_exchangeable?(); end

  # Returns the (real) user ID of this process.
  #
  # ```ruby
  # Process.uid   #=> 501
  # ```
  sig {returns(Integer)}
  def self.rid(); end

  # Returns `true` if the current platform has saved user ID functionality.
  sig {returns(T::Boolean)}
  def self.sid_available?(); end

  # Switch the effective and real user IDs of the current process. If a *block*
  # is given, the user IDs will be switched back after the block is executed.
  # Returns the new effective user ID if called without a block, and the return
  # value of the block if one is given.
  sig {returns(Integer)}
  sig do
    type_parameters(:T).params(
        blk: T.proc.returns(T.type_parameter(:T)),
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
  Elem = type_member(:out) {{fixed: T.untyped}}

  def cstime; end

  def cstime=(_); end

  def cutime; end

  def cutime=(_); end

  def stime; end

  def stime=(_); end

  def utime; end

  def utime=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

class Process::Waiter < Thread
  sig {returns(Integer)}
  def pid(); end
end
