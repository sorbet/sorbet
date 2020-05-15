# typed: __STDLIB_INTERNAL

# [`Fcntl`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html) loads the constants
# defined in the system's <fcntl.h> C header file, and used with both the
# fcntl(2) and open(2) POSIX system calls.
#
# To perform a fcntl(2) operation, use IO::fcntl.
#
# To perform an open(2) operation, use
# [`IO::sysopen`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-sysopen).
#
# The set of operations and constants available depends upon specific operating
# system. Some values listed below may not be supported on your system.
#
# See your fcntl(2) man page for complete details.
#
# Open /tmp/tempfile as a write-only file that is created if it doesn't exist:
#
# ```ruby
# require 'fcntl'
#
# fd = IO.sysopen('/tmp/tempfile',
#                 Fcntl::O_WRONLY | Fcntl::O_EXCL | Fcntl::O_CREAT)
# f = IO.open(fd)
# f.syswrite("TEMP DATA")
# f.close
# ```
#
# Get the flags on file `s`:
#
# ```ruby
# m = s.fcntl(Fcntl::F_GETFL, 0)
# ```
#
# [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the non-blocking flag on
# `f` in addition to the existing flags in `m`.
#
# ```ruby
# f.fcntl(Fcntl::F_SETFL, Fcntl::O_NONBLOCK|m)
# ```
module Fcntl
  # [`FD_CLOEXEC`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#FD_CLOEXEC)
  #
  # the value of the close-on-exec flag.
  FD_CLOEXEC = T.let(T.unsafe(nil), Integer)

  # [`F_DUPFD`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_DUPFD)
  #
  # Duplicate a file descriptor to the minimum unused file descriptor greater
  # than or equal to the argument.
  #
  # The close-on-exec flag of the duplicated file descriptor is set. (Ruby uses
  # F\_DUPFD\_CLOEXEC internally if available to avoid race condition.
  # [`F_SETFD`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_SETFD) is used
  # if F\_DUPFD\_CLOEXEC is not available.)
  F_DUPFD = T.let(T.unsafe(nil), Integer)

  # [`F_GETFD`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_GETFD)
  #
  # Read the close-on-exec flag of a file descriptor.
  F_GETFD = T.let(T.unsafe(nil), Integer)

  # [`F_GETFL`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_GETFL)
  #
  # Get the file descriptor flags. This will be one or more of the O\_\* flags.
  F_GETFL = T.let(T.unsafe(nil), Integer)

  # [`F_GETLK`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_GETLK)
  #
  # Determine whether a given region of a file is locked. This uses one of the
  # F\_\*LK flags.
  F_GETLK = T.let(T.unsafe(nil), Integer)

  # [`F_RDLCK`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_RDLCK)
  #
  # Read lock for a region of a file
  F_RDLCK = T.let(T.unsafe(nil), Integer)

  # [`F_SETFD`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_SETFD)
  #
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the close-on-exec flag
  # of a file descriptor.
  F_SETFD = T.let(T.unsafe(nil), Integer)

  # [`F_SETFL`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_SETFL)
  #
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the file descriptor
  # flags. This will be one or more of the O\_\* flags.
  F_SETFL = T.let(T.unsafe(nil), Integer)

  # [`F_SETLK`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_SETLK)
  #
  # Acquire a lock on a region of a file. This uses one of the F\_\*LCK flags.
  F_SETLK = T.let(T.unsafe(nil), Integer)

  # [`F_SETLKW`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_SETLKW)
  #
  # Acquire a lock on a region of a file, waiting if necessary. This uses one of
  # the F\_\*LCK flags
  F_SETLKW = T.let(T.unsafe(nil), Integer)

  # [`F_UNLCK`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_UNLCK)
  #
  # Remove lock for a region of a file
  F_UNLCK = T.let(T.unsafe(nil), Integer)

  # [`F_WRLCK`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#F_WRLCK)
  #
  # Write lock for a region of a file
  F_WRLCK = T.let(T.unsafe(nil), Integer)

  # [`O_ACCMODE`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_ACCMODE)
  #
  # Mask to extract the read/write flags
  O_ACCMODE = T.let(T.unsafe(nil), Integer)

  # [`O_APPEND`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_APPEND)
  #
  # Open the file in append mode
  O_APPEND = T.let(T.unsafe(nil), Integer)

  # [`O_CREAT`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_CREAT)
  #
  # Create the file if it doesn't exist
  O_CREAT = T.let(T.unsafe(nil), Integer)

  # [`O_EXCL`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_EXCL)
  #
  # Used with
  # [`O_CREAT`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_CREAT), fail if
  # the file exists
  O_EXCL = T.let(T.unsafe(nil), Integer)

  # [`O_NDELAY`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_NDELAY)
  #
  # Open the file in non-blocking mode
  O_NDELAY = T.let(T.unsafe(nil), Integer)

  # [`O_NOCTTY`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_NOCTTY)
  #
  # Open TTY without it becoming the controlling TTY
  O_NOCTTY = T.let(T.unsafe(nil), Integer)

  # [`O_NONBLOCK`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_NONBLOCK)
  #
  # Open the file in non-blocking mode
  O_NONBLOCK = T.let(T.unsafe(nil), Integer)

  # [`O_RDONLY`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_RDONLY)
  #
  # Open the file in read-only mode
  O_RDONLY = T.let(T.unsafe(nil), Integer)

  # [`O_RDWR`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_RDWR)
  #
  # Open the file in read-write mode
  O_RDWR = T.let(T.unsafe(nil), Integer)

  # [`O_TRUNC`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_TRUNC)
  #
  # Truncate the file on open
  O_TRUNC = T.let(T.unsafe(nil), Integer)

  # [`O_WRONLY`](https://docs.ruby-lang.org/en/2.6.0/Fcntl.html#O_WRONLY)
  #
  # Open the file in write-only mode.
  O_WRONLY = T.let(T.unsafe(nil), Integer)
end
