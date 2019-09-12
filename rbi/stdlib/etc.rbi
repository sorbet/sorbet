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
  Elem = type_member(:out, fixed: T.untyped)
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
  Elem = type_member(:out, fixed: T.untyped)
end
