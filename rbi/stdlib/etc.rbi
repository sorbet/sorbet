# typed: __STDLIB_INTERNAL

# The [`Etc`](https://docs.ruby-lang.org/en/2.6.0/Etc.html) module provides
# access to information typically stored in files in the /etc directory on Unix
# systems.
#
# The information accessible consists of the information found in the
# /etc/passwd and /etc/group files, plus information about the system's
# temporary directory (/tmp) and configuration directory (/etc).
#
# The [`Etc`](https://docs.ruby-lang.org/en/2.6.0/Etc.html) module provides a
# more reliable way to access information about the logged in user than
# environment variables such as +$USER+.
#
# ## Example:
#
# ```ruby
# require 'etc'
#
# login = Etc.getlogin
# info = Etc.getpwnam(login)
# username = info.gecos.split(/,/).first
# puts "Hello #{username}, I see your login name is #{login}"
# ```
#
# Note that the methods provided by this module are not always secure. It should
# be used for informational purposes, and not for security.
#
# All operations defined in this module are class methods, so that you can
# include the [`Etc`](https://docs.ruby-lang.org/en/2.6.0/Etc.html) module into
# your class.
module Etc
  CS_GNU_LIBC_VERSION = T.let(T.unsafe(nil), Integer)
  CS_GNU_LIBPTHREAD_VERSION = T.let(T.unsafe(nil), Integer)
  CS_PATH = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_ILP32_OFF32_CFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_ILP32_OFF32_LDFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_ILP32_OFF32_LIBS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_ILP32_OFFBIG_CFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_ILP32_OFFBIG_LIBS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_LP64_OFF64_CFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_LP64_OFF64_LDFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_LP64_OFF64_LIBS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_LPBIG_OFFBIG_LIBS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V6_WIDTH_RESTRICTED_ENVS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_ILP32_OFF32_CFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_ILP32_OFF32_LDFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_ILP32_OFF32_LIBS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_ILP32_OFFBIG_CFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_ILP32_OFFBIG_LIBS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_LP64_OFF64_CFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_LP64_OFF64_LDFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_LP64_OFF64_LIBS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_LPBIG_OFFBIG_LIBS = T.let(T.unsafe(nil), Integer)
  CS_POSIX_V7_WIDTH_RESTRICTED_ENVS = T.let(T.unsafe(nil), Integer)
  CS_V6_ENV = T.let(T.unsafe(nil), Integer)
  CS_V7_ENV = T.let(T.unsafe(nil), Integer)
  PC_2_SYMLINKS = T.let(T.unsafe(nil), Integer)
  PC_ALLOC_SIZE_MIN = T.let(T.unsafe(nil), Integer)
  PC_ASYNC_IO = T.let(T.unsafe(nil), Integer)
  PC_CHOWN_RESTRICTED = T.let(T.unsafe(nil), Integer)
  PC_FILESIZEBITS = T.let(T.unsafe(nil), Integer)
  PC_LINK_MAX = T.let(T.unsafe(nil), Integer)
  PC_MAX_CANON = T.let(T.unsafe(nil), Integer)
  PC_MAX_INPUT = T.let(T.unsafe(nil), Integer)
  PC_NAME_MAX = T.let(T.unsafe(nil), Integer)
  PC_NO_TRUNC = T.let(T.unsafe(nil), Integer)
  PC_PATH_MAX = T.let(T.unsafe(nil), Integer)
  PC_PIPE_BUF = T.let(T.unsafe(nil), Integer)
  PC_PRIO_IO = T.let(T.unsafe(nil), Integer)
  PC_REC_INCR_XFER_SIZE = T.let(T.unsafe(nil), Integer)
  PC_REC_MAX_XFER_SIZE = T.let(T.unsafe(nil), Integer)
  PC_REC_MIN_XFER_SIZE = T.let(T.unsafe(nil), Integer)
  PC_REC_XFER_ALIGN = T.let(T.unsafe(nil), Integer)
  PC_SYMLINK_MAX = T.let(T.unsafe(nil), Integer)
  PC_SYNC_IO = T.let(T.unsafe(nil), Integer)
  PC_VDISABLE = T.let(T.unsafe(nil), Integer)
  SC_2_CHAR_TERM = T.let(T.unsafe(nil), Integer)
  SC_2_C_BIND = T.let(T.unsafe(nil), Integer)
  SC_2_C_DEV = T.let(T.unsafe(nil), Integer)
  SC_2_FORT_DEV = T.let(T.unsafe(nil), Integer)
  SC_2_FORT_RUN = T.let(T.unsafe(nil), Integer)
  SC_2_LOCALEDEF = T.let(T.unsafe(nil), Integer)
  SC_2_PBS = T.let(T.unsafe(nil), Integer)
  SC_2_PBS_ACCOUNTING = T.let(T.unsafe(nil), Integer)
  SC_2_PBS_CHECKPOINT = T.let(T.unsafe(nil), Integer)
  SC_2_PBS_LOCATE = T.let(T.unsafe(nil), Integer)
  SC_2_PBS_MESSAGE = T.let(T.unsafe(nil), Integer)
  SC_2_PBS_TRACK = T.let(T.unsafe(nil), Integer)
  SC_2_SW_DEV = T.let(T.unsafe(nil), Integer)
  SC_2_UPE = T.let(T.unsafe(nil), Integer)
  SC_2_VERSION = T.let(T.unsafe(nil), Integer)
  SC_ADVISORY_INFO = T.let(T.unsafe(nil), Integer)
  SC_AIO_LISTIO_MAX = T.let(T.unsafe(nil), Integer)
  SC_AIO_MAX = T.let(T.unsafe(nil), Integer)
  SC_AIO_PRIO_DELTA_MAX = T.let(T.unsafe(nil), Integer)
  SC_ARG_MAX = T.let(T.unsafe(nil), Integer)
  SC_ASYNCHRONOUS_IO = T.let(T.unsafe(nil), Integer)
  SC_ATEXIT_MAX = T.let(T.unsafe(nil), Integer)
  SC_AVPHYS_PAGES = T.let(T.unsafe(nil), Integer)
  SC_BARRIERS = T.let(T.unsafe(nil), Integer)
  SC_BC_BASE_MAX = T.let(T.unsafe(nil), Integer)
  SC_BC_DIM_MAX = T.let(T.unsafe(nil), Integer)
  SC_BC_SCALE_MAX = T.let(T.unsafe(nil), Integer)
  SC_BC_STRING_MAX = T.let(T.unsafe(nil), Integer)
  SC_CHILD_MAX = T.let(T.unsafe(nil), Integer)
  SC_CLK_TCK = T.let(T.unsafe(nil), Integer)
  SC_CLOCK_SELECTION = T.let(T.unsafe(nil), Integer)
  SC_COLL_WEIGHTS_MAX = T.let(T.unsafe(nil), Integer)
  SC_CPUTIME = T.let(T.unsafe(nil), Integer)
  SC_DELAYTIMER_MAX = T.let(T.unsafe(nil), Integer)
  SC_EXPR_NEST_MAX = T.let(T.unsafe(nil), Integer)
  SC_FSYNC = T.let(T.unsafe(nil), Integer)
  SC_GETGR_R_SIZE_MAX = T.let(T.unsafe(nil), Integer)
  SC_GETPW_R_SIZE_MAX = T.let(T.unsafe(nil), Integer)
  SC_HOST_NAME_MAX = T.let(T.unsafe(nil), Integer)
  SC_IOV_MAX = T.let(T.unsafe(nil), Integer)
  SC_IPV6 = T.let(T.unsafe(nil), Integer)
  SC_JOB_CONTROL = T.let(T.unsafe(nil), Integer)
  SC_LINE_MAX = T.let(T.unsafe(nil), Integer)
  SC_LOGIN_NAME_MAX = T.let(T.unsafe(nil), Integer)
  SC_MAPPED_FILES = T.let(T.unsafe(nil), Integer)
  SC_MEMLOCK = T.let(T.unsafe(nil), Integer)
  SC_MEMLOCK_RANGE = T.let(T.unsafe(nil), Integer)
  SC_MEMORY_PROTECTION = T.let(T.unsafe(nil), Integer)
  SC_MESSAGE_PASSING = T.let(T.unsafe(nil), Integer)
  SC_MONOTONIC_CLOCK = T.let(T.unsafe(nil), Integer)
  SC_MQ_OPEN_MAX = T.let(T.unsafe(nil), Integer)
  SC_MQ_PRIO_MAX = T.let(T.unsafe(nil), Integer)
  SC_NGROUPS_MAX = T.let(T.unsafe(nil), Integer)
  SC_NPROCESSORS_CONF = T.let(T.unsafe(nil), Integer)
  SC_NPROCESSORS_ONLN = T.let(T.unsafe(nil), Integer)
  SC_OPEN_MAX = T.let(T.unsafe(nil), Integer)
  SC_PAGESIZE = T.let(T.unsafe(nil), Integer)
  SC_PAGE_SIZE = T.let(T.unsafe(nil), Integer)
  SC_PHYS_PAGES = T.let(T.unsafe(nil), Integer)
  SC_PRIORITIZED_IO = T.let(T.unsafe(nil), Integer)
  SC_PRIORITY_SCHEDULING = T.let(T.unsafe(nil), Integer)
  SC_RAW_SOCKETS = T.let(T.unsafe(nil), Integer)
  SC_READER_WRITER_LOCKS = T.let(T.unsafe(nil), Integer)
  SC_REALTIME_SIGNALS = T.let(T.unsafe(nil), Integer)
  SC_REGEXP = T.let(T.unsafe(nil), Integer)
  SC_RE_DUP_MAX = T.let(T.unsafe(nil), Integer)
  SC_RTSIG_MAX = T.let(T.unsafe(nil), Integer)
  SC_SAVED_IDS = T.let(T.unsafe(nil), Integer)
  SC_SEMAPHORES = T.let(T.unsafe(nil), Integer)
  SC_SEM_NSEMS_MAX = T.let(T.unsafe(nil), Integer)
  SC_SEM_VALUE_MAX = T.let(T.unsafe(nil), Integer)
  SC_SHARED_MEMORY_OBJECTS = T.let(T.unsafe(nil), Integer)
  SC_SHELL = T.let(T.unsafe(nil), Integer)
  SC_SIGQUEUE_MAX = T.let(T.unsafe(nil), Integer)
  SC_SPAWN = T.let(T.unsafe(nil), Integer)
  SC_SPIN_LOCKS = T.let(T.unsafe(nil), Integer)
  SC_SPORADIC_SERVER = T.let(T.unsafe(nil), Integer)
  SC_SS_REPL_MAX = T.let(T.unsafe(nil), Integer)
  SC_STREAM_MAX = T.let(T.unsafe(nil), Integer)
  SC_SYMLOOP_MAX = T.let(T.unsafe(nil), Integer)
  SC_SYNCHRONIZED_IO = T.let(T.unsafe(nil), Integer)
  SC_THREADS = T.let(T.unsafe(nil), Integer)
  SC_THREAD_ATTR_STACKADDR = T.let(T.unsafe(nil), Integer)
  SC_THREAD_ATTR_STACKSIZE = T.let(T.unsafe(nil), Integer)
  SC_THREAD_CPUTIME = T.let(T.unsafe(nil), Integer)
  SC_THREAD_DESTRUCTOR_ITERATIONS = T.let(T.unsafe(nil), Integer)
  SC_THREAD_KEYS_MAX = T.let(T.unsafe(nil), Integer)
  SC_THREAD_PRIORITY_SCHEDULING = T.let(T.unsafe(nil), Integer)
  SC_THREAD_PRIO_INHERIT = T.let(T.unsafe(nil), Integer)
  SC_THREAD_PRIO_PROTECT = T.let(T.unsafe(nil), Integer)
  SC_THREAD_PROCESS_SHARED = T.let(T.unsafe(nil), Integer)
  SC_THREAD_ROBUST_PRIO_INHERIT = T.let(T.unsafe(nil), Integer)
  SC_THREAD_ROBUST_PRIO_PROTECT = T.let(T.unsafe(nil), Integer)
  SC_THREAD_SAFE_FUNCTIONS = T.let(T.unsafe(nil), Integer)
  SC_THREAD_SPORADIC_SERVER = T.let(T.unsafe(nil), Integer)
  SC_THREAD_STACK_MIN = T.let(T.unsafe(nil), Integer)
  SC_THREAD_THREADS_MAX = T.let(T.unsafe(nil), Integer)
  SC_TIMEOUTS = T.let(T.unsafe(nil), Integer)
  SC_TIMERS = T.let(T.unsafe(nil), Integer)
  SC_TIMER_MAX = T.let(T.unsafe(nil), Integer)
  SC_TRACE = T.let(T.unsafe(nil), Integer)
  SC_TRACE_EVENT_FILTER = T.let(T.unsafe(nil), Integer)
  SC_TRACE_EVENT_NAME_MAX = T.let(T.unsafe(nil), Integer)
  SC_TRACE_INHERIT = T.let(T.unsafe(nil), Integer)
  SC_TRACE_LOG = T.let(T.unsafe(nil), Integer)
  SC_TRACE_NAME_MAX = T.let(T.unsafe(nil), Integer)
  SC_TRACE_SYS_MAX = T.let(T.unsafe(nil), Integer)
  SC_TRACE_USER_EVENT_MAX = T.let(T.unsafe(nil), Integer)
  SC_TTY_NAME_MAX = T.let(T.unsafe(nil), Integer)
  SC_TYPED_MEMORY_OBJECTS = T.let(T.unsafe(nil), Integer)
  SC_TZNAME_MAX = T.let(T.unsafe(nil), Integer)
  SC_V6_ILP32_OFF32 = T.let(T.unsafe(nil), Integer)
  SC_V6_ILP32_OFFBIG = T.let(T.unsafe(nil), Integer)
  SC_V6_LP64_OFF64 = T.let(T.unsafe(nil), Integer)
  SC_V6_LPBIG_OFFBIG = T.let(T.unsafe(nil), Integer)
  SC_V7_ILP32_OFF32 = T.let(T.unsafe(nil), Integer)
  SC_V7_ILP32_OFFBIG = T.let(T.unsafe(nil), Integer)
  SC_V7_LP64_OFF64 = T.let(T.unsafe(nil), Integer)
  SC_V7_LPBIG_OFFBIG = T.let(T.unsafe(nil), Integer)
  SC_VERSION = T.let(T.unsafe(nil), Integer)
  SC_XOPEN_CRYPT = T.let(T.unsafe(nil), Integer)
  SC_XOPEN_ENH_I18N = T.let(T.unsafe(nil), Integer)
  SC_XOPEN_REALTIME = T.let(T.unsafe(nil), Integer)
  SC_XOPEN_REALTIME_THREADS = T.let(T.unsafe(nil), Integer)
  SC_XOPEN_SHM = T.let(T.unsafe(nil), Integer)
  SC_XOPEN_STREAMS = T.let(T.unsafe(nil), Integer)
  SC_XOPEN_UNIX = T.let(T.unsafe(nil), Integer)
  SC_XOPEN_VERSION = T.let(T.unsafe(nil), Integer)

  # Returns system configuration variable using confstr().
  #
  # *name* should be a constant under `Etc` which begins with `CS_`.
  #
  # The return value is a string or nil. nil means no configuration-defined
  # value. (confstr() returns 0 but errno is not set.)
  #
  # ```ruby
  # Etc.confstr(Etc::CS_PATH) #=> "/bin:/usr/bin"
  #
  # # GNU/Linux
  # Etc.confstr(Etc::CS_GNU_LIBC_VERSION) #=> "glibc 2.18"
  # Etc.confstr(Etc::CS_GNU_LIBPTHREAD_VERSION) #=> "NPTL 2.18"
  # ```
  sig do
    params(p1: Integer).returns(T.nilable(String))
  end
  def self.confstr(p1); end

  # Ends the process of scanning through the /etc/group file begun by
  # [`::getgrent`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getgrent),
  # and closes the file.
  sig do
    void
  end
  def self.endgrent; end

  # Ends the process of scanning through the /etc/passwd file begun with
  # [`::getpwent`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getpwent),
  # and closes the file.
  sig do
    void
  end
  def self.endpwent; end

  # Returns an entry from the /etc/group file.
  #
  # The first time it is called it opens the file and returns the first entry;
  # each successive call returns the next entry, or `nil` if the end of the file
  # has been reached.
  #
  # To close the file when processing is complete, call
  # [`::endgrent`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-endgrent).
  #
  # Each entry is returned as a
  # [`Group`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Group) struct
  sig do
    returns(T.nilable(Etc::Group))
  end
  def self.getgrent; end

  # Returns information about the group with specified integer `group_id`, as
  # found in /etc/group.
  #
  # The information is returned as a
  # [`Group`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Group) struct.
  #
  # See the unix manpage for `getgrgid(3)` for more detail.
  #
  # ### Example:
  #
  # ```ruby
  # Etc.getgrgid(100)
  # #=> #<struct Etc::Group name="users", passwd="x", gid=100, mem=["meta", "root"]>
  # ```
  sig do
    params(group_id: Integer).returns(T.nilable(Etc::Group))
  end
  def self.getgrgid(group_id); end

  # Returns information about the group with specified `name`, as found in
  # /etc/group.
  #
  # The information is returned as a
  # [`Group`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Group) struct.
  #
  # See the unix manpage for `getgrnam(3)` for more detail.
  #
  # ### Example:
  #
  # ```ruby
  # Etc.getgrnam('users')
  # #=> #<struct Etc::Group name="users", passwd="x", gid=100, mem=["meta", "root"]>
  # ```
  sig do
    params(name: String).returns(T.nilable(Etc::Group))
  end
  def self.getgrnam(name); end

  # Returns the short user name of the currently logged in user. Unfortunately,
  # it is often rather easy to fool
  # [`::getlogin`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getlogin).
  #
  # Avoid
  # [`::getlogin`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getlogin)
  # for security-related purposes.
  #
  # If
  # [`::getlogin`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getlogin)
  # fails, try
  # [`::getpwuid`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getpwuid).
  #
  # See the unix manpage for `getpwuid(3)` for more detail.
  #
  # e.g.
  #
  # ```
  # Etc.getlogin -> 'guest'
  # ```
  sig do
    returns(T.nilable(String))
  end
  def self.getlogin; end

  # Returns an entry from the /etc/passwd file.
  #
  # The first time it is called it opens the file and returns the first entry;
  # each successive call returns the next entry, or `nil` if the end of the file
  # has been reached.
  #
  # To close the file when processing is complete, call
  # [`::endpwent`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-endpwent).
  #
  # Each entry is returned as a
  # [`Passwd`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Passwd) struct.
  sig do
    returns(T.nilable(Etc::Passwd))
  end
  def self.getpwent; end

  # Returns the /etc/passwd information for the user with specified login
  # `name`.
  #
  # The information is returned as a
  # [`Passwd`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Passwd) struct.
  #
  # See the unix manpage for `getpwnam(3)` for more detail.
  #
  # ### Example:
  #
  # ```ruby
  # Etc.getpwnam('root')
  # #=> #<struct Etc::Passwd name="root", passwd="x", uid=0, gid=0, gecos="root",dir="/root", shell="/bin/bash">
  # ```
  sig do
    params(name: String).returns(T.nilable(Etc::Passwd))
  end
  def self.getpwnam(name); end

  # Returns the /etc/passwd information for the user with the given integer
  # `uid`.
  #
  # The information is returned as a
  # [`Passwd`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Passwd) struct.
  #
  # If `uid` is omitted, the value from `Passwd[:uid]` is returned instead.
  #
  # See the unix manpage for `getpwuid(3)` for more detail.
  #
  # ### Example:
  #
  # ```ruby
  # Etc.getpwuid(0)
  # #=> #<struct Etc::Passwd name="root", passwd="x", uid=0, gid=0, gecos="root",dir="/root", shell="/bin/bash">
  # ```
  sig do
    params(uid: Integer).returns(T.nilable(Etc::Passwd))
  end
  def self.getpwuid(uid=T.unsafe(nil)); end

  # Provides a convenient Ruby iterator which executes a block for each entry in
  # the /etc/group file.
  #
  # The code block is passed an
  # [`Group`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Group) struct.
  #
  # See
  # [`::getgrent`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getgrent)
  # above for details.
  #
  # Example:
  #
  # ```ruby
  # require 'etc'
  #
  # Etc.group {|g|
  #   puts g.name + ": " + g.mem.join(', ')
  # }
  # ```
  sig do
    params(blk: T.nilable(T.proc.params(arg0: Etc::Group).void))
    .returns(T.nilable(Etc::Group))
  end
  def self.group(&blk); end

  # Returns the number of online processors.
  #
  # The result is intended as the number of processes to use all available
  # processors.
  #
  # This method is implemented using:
  # *   sched\_getaffinity(): Linux
  # *   sysconf(\_SC\_NPROCESSORS\_ONLN): GNU/Linux, NetBSD, FreeBSD, OpenBSD,
  #     DragonFly BSD, OpenIndiana, Mac OS X, AIX
  #
  #
  # Example:
  #
  # ```ruby
  # require 'etc'
  # p Etc.nprocessors #=> 4
  # ```
  #
  # The result might be smaller number than physical cpus especially when ruby
  # process is bound to specific cpus. This is intended for getting better
  # parallel processing.
  #
  # Example: (Linux)
  #
  # ```
  # linux$ taskset 0x3 ./ruby -retc -e "p Etc.nprocessors"  #=> 2
  # ```
  sig do
    returns(Integer)
  end
  def self.nprocessors; end

  # Provides a convenient Ruby iterator which executes a block for each entry in
  # the /etc/passwd file.
  #
  # The code block is passed an
  # [`Passwd`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Passwd) struct.
  #
  # See
  # [`::getpwent`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getpwent)
  # above for details.
  #
  # Example:
  #
  # ```ruby
  # require 'etc'
  #
  # Etc.passwd {|u|
  #   puts u.name + " = " + u.gecos
  # }
  # ```
  sig do
    params(
      blk: T.nilable(T.proc.params(struct: Etc::Passwd).void)
    ).returns(T.nilable(Etc::Passwd))
  end
  def self.passwd(&blk); end

  # Resets the process of reading the /etc/group file, so that the next call to
  # [`::getgrent`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getgrent)
  # will return the first entry again.
  sig do
    void
  end
  def self.setgrent; end

  # Resets the process of reading the /etc/passwd file, so that the next call to
  # [`::getpwent`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-getpwent)
  # will return the first entry again.
  sig do
    void
  end
  def self.setpwent; end

  # Returns system configuration variable using sysconf().
  #
  # *name* should be a constant under `Etc` which begins with `SC_`.
  #
  # The return value is an integer or nil. nil means indefinite limit.
  # (sysconf() returns -1 but errno is not set.)
  #
  # ```ruby
  # Etc.sysconf(Etc::SC_ARG_MAX) #=> 2097152
  # Etc.sysconf(Etc::SC_LOGIN_NAME_MAX) #=> 256
  # ```
  sig do
    params(p1: Integer).returns(Integer)
  end
  def self.sysconf(p1); end

  # Returns system configuration directory.
  #
  # This is typically "/etc", but is modified by the prefix used when Ruby was
  # compiled. For example, if Ruby is built and installed in /usr/local, returns
  # "/usr/local/etc" on other platforms than Windows. On Windows, this always
  # returns the directory provided by the system.
  sig do
    returns(String)
  end
  def self.sysconfdir; end

  # Returns system temporary directory; typically "/tmp".
  sig do
    returns(String)
  end
  def self.systmpdir; end

  # Returns the system information obtained by uname system call.
  #
  # The return value is a hash which has 5 keys at least:
  #
  # ```
  # :sysname, :nodename, :release, :version, :machine
  # ```
  #
  # Example:
  #
  # ```ruby
  # require 'etc'
  # require 'pp'
  #
  # pp Etc.uname
  # #=> {:sysname=>"Linux",
  # #    :nodename=>"boron",
  # #    :release=>"2.6.18-6-xen-686",
  # #    :version=>"#1 SMP Thu Nov 5 19:54:42 UTC 2009",
  # #    :machine=>"i686"}
  # ```
  sig do
    returns(T::Hash[Symbol, String])
  end
  def self.uname; end
end

# [`Group`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Group)
#
# [`Group`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Group) is a
# [`Struct`](https://docs.ruby-lang.org/en/2.6.0/Struct.html) that is only
# available when compiled with `HAVE_GETGRENT`.
#
# The struct contains the following members:
#
# name
# :   contains the name of the group as a
#     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
# passwd
# :   contains the encrypted password as a
#     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). An 'x' is
#     returned if password access to the group is not available; an empty string
#     is returned if no password is needed to obtain membership of the group.
#
#     Must be compiled with `HAVE_STRUCT_GROUP_GR_PASSWD`.
# gid
# :   contains the group's numeric ID as an integer.
# mem
# :   is an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of Strings
#     containing the short login names of the members of the group.
class Etc::Group < Struct
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}
  class << self
    extend T::Generic
    Elem = type_member {{fixed: T.untyped}}
  end

  sig { returns(Integer) }
  def gid; end
  sig { returns(T::Array[String]) }
  def mem; end
  sig { returns(String) }
  def name; end
  sig { returns(String) }
  def passwd; end
end

# [`Passwd`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Passwd)
#
# [`Passwd`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#Passwd) is a
# [`Struct`](https://docs.ruby-lang.org/en/2.6.0/Struct.html) that contains the
# following members:
#
# name
# :   contains the short login name of the user as a
#     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
# passwd
# :   contains the encrypted password of the user as a
#     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). an 'x' is
#     returned if shadow passwords are in use. An '\*' is returned if the user
#     cannot log in using a password.
# uid
# :   contains the integer user ID (uid) of the user.
# gid
# :   contains the integer group ID (gid) of the user's primary group.
# dir
# :   contains the path to the home directory of the user as a
#     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
# shell
# :   contains the path to the login shell of the user as a
#     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
#
#
# ### The following members below are optional, and must be compiled with special flags:
#
# gecos
# :   contains a longer
#     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) description of
#     the user, such as a full name. Some Unix systems provide structured
#     information in the gecos field, but this is system-dependent. must be
#     compiled with `HAVE_STRUCT_PASSWD_PW_GECOS`
# change
# :   password change time(integer) must be compiled with
#     `HAVE_STRUCT_PASSWD_PW_CHANGE`
# quota
# :   quota value(integer) must be compiled with `HAVE_STRUCT_PASSWD_PW_QUOTA`
# age
# :   password age(integer) must be compiled with `HAVE_STRUCT_PASSWD_PW_AGE`
# class
# :   user access class(string) must be compiled with
#     `HAVE_STRUCT_PASSWD_PW_CLASS`
# comment
# :   comment(string) must be compiled with `HAVE_STRUCT_PASSWD_PW_COMMENT`
# expire
# :   account expiration time(integer) must be compiled with
#     `HAVE_STRUCT_PASSWD_PW_EXPIRE`
class Etc::Passwd < Struct
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  # Contains the short login name of the user as a String.
  sig { returns(String) }
  def name; end

  # Contains the encrypted password of the user as a String.
  #
  # An 'x' is returned if shadow passwords are in use.
  # An '*' is returned if the user cannot log in using a password.
  sig { returns(String) }
  def passwd; end

  # Contains the integer user ID (uid) of the user.
  sig { returns(Integer) }
  def uid; end

  # Contains the integer group ID (gid) of the user's primary group.
  sig { returns(Integer) }
  def gid; end

  # Contains the path to the home directory of the user as a String.
  sig { returns(String) }
  def dir; end

  # Contains the path to the login shell of the user as a String.
  sig { returns(String) }
  def shell; end

  class << self
    extend T::Generic
    Elem = type_member {{fixed: T.untyped}}
  end
end
