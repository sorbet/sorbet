# typed: __STDLIB_INTERNAL

# Expect library adds the [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html)
# instance method
# [`expect`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-expect), which
# does similar act to tcl's expect extension.
#
# In order to use this method, you must require expect:
#
# ```ruby
# require 'expect'
# ```
#
# Please see
# [`expect`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-expect) for
# usage.
# The [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) class is the basis for
# all input and output in Ruby. An I/O stream may be *duplexed* (that is,
# bidirectional), and so may use more than one native operating system stream.
#
# Many of the examples in this section use the
# [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) class, the only
# standard subclass of [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html). The
# two classes are closely associated. Like the
# [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) class, the
# [`Socket`](https://docs.ruby-lang.org/en/2.6.0/Socket.html) library subclasses
# from [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) (such as
# [`TCPSocket`](https://docs.ruby-lang.org/en/2.6.0/TCPSocket.html) or
# [`UDPSocket`](https://docs.ruby-lang.org/en/2.6.0/UDPSocket.html)).
#
# The
# [`Kernel#open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open)
# method can create an [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) (or
# [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html)) object for these
# types of arguments:
#
# *   A plain string represents a filename suitable for the underlying operating
#     system.
#
# *   A string starting with `"|"` indicates a subprocess. The remainder of the
#     string following the `"|"` is invoked as a process with appropriate
#     input/output channels connected to it.
#
# *   A string equal to `"|-"` will create another Ruby instance as a
#     subprocess.
#
#
# The [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) may be opened with
# different file modes (read-only, write-only) and encodings for proper
# conversion. See
# [`IO.new`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-new) for these
# options. See
# [`Kernel#open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open)
# for details of the various command formats described above.
#
# [`IO.popen`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-popen), the
# [`Open3`](https://docs.ruby-lang.org/en/2.6.0/Open3.html) library, or
# Process#spawn may also be used to communicate with subprocesses through an
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html).
#
# Ruby will convert pathnames between different operating system conventions if
# possible. For instance, on a Windows system the filename
# `"/gumby/ruby/test.rb"` will be opened as `"\gumby\ruby\test.rb"`. When
# specifying a Windows-style filename in a Ruby string, remember to escape the
# backslashes:
#
# ```ruby
# "C:\\gumby\\ruby\\test.rb"
# ```
#
# Our examples here will use the Unix-style forward slashes;
# File::ALT\_SEPARATOR can be used to get the platform-specific separator
# character.
#
# The global constant [`ARGF`](https://docs.ruby-lang.org/en/2.6.0/ARGF.html)
# (also accessible as `$<`) provides an IO-like stream which allows access to
# all files mentioned on the command line (or STDIN if no files are mentioned).
# [`ARGF#path`](https://docs.ruby-lang.org/en/2.6.0/ARGF.html#method-i-path) and
# its alias
# [`ARGF#filename`](https://docs.ruby-lang.org/en/2.6.0/ARGF.html#method-i-filename)
# are provided to access the name of the file currently being read.
#
# ## io/console
#
# The io/console extension provides methods for interacting with the console.
# The console can be accessed from
# [`IO.console`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-console)
# or the standard input/output/error
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) objects.
#
# Requiring io/console adds the following methods:
#
# *   [`IO::console`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-console)
# *   [`IO#raw`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-raw)
# *   [`IO#raw!`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-raw-21)
# *   [`IO#cooked`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-cooked)
# *   [`IO#cooked!`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-cooked-21)
# *   [`IO#getch`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-getch)
# *   [`IO#echo=`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-echo-3D)
# *   [`IO#echo?`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-echo-3F)
# *   [`IO#noecho`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-noecho)
# *   [`IO#winsize`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-winsize)
# *   [`IO#winsize=`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-winsize-3D)
# *   [`IO#iflush`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-iflush)
# *   [`IO#ioflush`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-ioflush)
# *   [`IO#oflush`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-oflush)
#
#
# Example:
#
# ```ruby
# require 'io/console'
# rows, columns = $stdout.winsize
# puts "Your screen is #{columns} wide and #{rows} tall"
# ```
class IO < Object
  include File::Constants
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: String)

  APPEND = T.let(T.unsafe(nil), Integer)
  BINARY = T.let(T.unsafe(nil), Integer)
  CREAT = T.let(T.unsafe(nil), Integer)
  DIRECT = T.let(T.unsafe(nil), Integer)
  DSYNC = T.let(T.unsafe(nil), Integer)
  EXCL = T.let(T.unsafe(nil), Integer)
  FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
  FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
  FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
  FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
  FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
  FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
  FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
  LOCK_EX = T.let(T.unsafe(nil), Integer)
  LOCK_NB = T.let(T.unsafe(nil), Integer)
  LOCK_SH = T.let(T.unsafe(nil), Integer)
  LOCK_UN = T.let(T.unsafe(nil), Integer)
  NOATIME = T.let(T.unsafe(nil), Integer)
  NOCTTY = T.let(T.unsafe(nil), Integer)
  NOFOLLOW = T.let(T.unsafe(nil), Integer)
  NONBLOCK = T.let(T.unsafe(nil), Integer)
  NULL = T.let(T.unsafe(nil), String)
  RDONLY = T.let(T.unsafe(nil), Integer)
  RDWR = T.let(T.unsafe(nil), Integer)
  RSYNC = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) I/O position from the
  # current position
  SEEK_CUR = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) I/O position to the
  # next location containing data
  SEEK_DATA = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) I/O position from the
  # end
  SEEK_END = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) I/O position to the
  # next hole
  SEEK_HOLE = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) I/O position from the
  # beginning
  SEEK_SET = T.let(T.unsafe(nil), Integer)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)

  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) Output---Writes
  # *obj* to *ios*. *obj* will be converted to a string using `to_s`.
  #
  # ```ruby
  # $stdout << "Hello " << "world!\n"
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Hello world!
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.self_type)
  end
  def <<(arg0); end

  # Announce an intention to access data from the current file in a specific
  # pattern. On platforms that do not support the *posix\_fadvise(2)* system
  # call, this method is a no-op.
  #
  # *advice* is one of the following symbols:
  #
  # :normal
  # :   No advice to give; the default assumption for an open file.
  # :sequential
  # :   The data will be accessed sequentially with lower offsets read before
  #     higher ones.
  # :random
  # :   The data will be accessed in random order.
  # :willneed
  # :   The data will be accessed in the near future.
  # :dontneed
  # :   The data will not be accessed in the near future.
  # :noreuse
  # :   The data will only be accessed once.
  #
  #
  # The semantics of a piece of advice are platform-dependent. See *man 2
  # posix\_fadvise* for details.
  #
  # "data" means the region of the current file that begins at *offset* and
  # extends for *len* bytes. If *len* is 0, the region ends at the last byte of
  # the file. By default, both *offset* and *len* are 0, meaning that the advice
  # applies to the entire file.
  #
  # If an error occurs, one of the following exceptions will be raised:
  #
  # `IOError`
  # :   The `IO` stream is closed.
  # `Errno::EBADF`
  # :   The file descriptor of the current file is invalid.
  # `Errno::EINVAL`
  # :   An invalid value for *advice* was given.
  # `Errno::ESPIPE`
  # :   The file descriptor of the current file refers to a FIFO or pipe. (Linux
  #     raises `Errno::EINVAL` in this case).
  # `TypeError`
  # :   Either *advice* was not a
  #     [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html), or one of
  #     the other arguments was not an `Integer`.
  # `RangeError`
  # :   One of the arguments given was too big/small.
  #
  # This list is not exhaustive; other [`Errno`](https://docs.ruby-lang.org/en/2.6.0/Errno.html)
  # :   exceptions are also possible.
  sig do
    params(
        arg0: Symbol,
        offset: Integer,
        len: Integer,
    )
    .returns(NilClass)
  end
  def advise(arg0, offset=T.unsafe(nil), len=T.unsafe(nil)); end

  # Sets auto-close flag.
  #
  # ```ruby
  # f = open("/dev/null")
  # IO.for_fd(f.fileno)
  # # ...
  # f.gets # may cause IOError
  #
  # f = open("/dev/null")
  # IO.for_fd(f.fileno).autoclose = true
  # # ...
  # f.gets # won't cause IOError
  # ```
  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def autoclose=(arg0); end

  # Returns `true` if the underlying file descriptor of *ios* will be closed
  # automatically at its finalization, otherwise `false`.
  sig {returns(T::Boolean)}
  def autoclose?(); end

  # Puts *ios* into binary mode. Once a stream is in binary mode, it cannot be
  # reset to nonbinary mode.
  #
  # *   newline conversion disabled
  # *   encoding conversion disabled
  # *   content is treated as ASCII-8BIT
  sig {returns(T.self_type)}
  def binmode(); end

  # Returns `true` if *ios* is binmode.
  sig {returns(T::Boolean)}
  def binmode?(); end

  # Closes *ios* and flushes any pending writes to the operating system. The
  # stream is unavailable for any further data operations; an `IOError` is
  # raised if such an attempt is made. I/O streams are automatically closed when
  # they are claimed by the garbage collector.
  #
  # If *ios* is opened by `IO.popen`, `close` sets `$?`.
  #
  # Calling this method on closed
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object is just ignored
  # since Ruby 2.3.
  sig {returns(NilClass)}
  def close(); end

  # Sets a close-on-exec flag.
  #
  # ```ruby
  # f = open("/dev/null")
  # f.close_on_exec = true
  # system("cat", "/proc/self/fd/#{f.fileno}") # cat: /proc/self/fd/3: No such file or directory
  # f.closed?                #=> false
  # ```
  #
  # Ruby sets close-on-exec flags of all file descriptors by default since Ruby
  # 2.0.0. So you don't need to set by yourself. Also, unsetting a close-on-exec
  # flag can cause file descriptor leak if another thread use fork() and exec()
  # (via system() method for example). If you really needs file descriptor
  # inheritance to child process, use spawn()'s argument such as fd=>fd.
  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def close_on_exec=(arg0); end

  # Returns `true` if *ios* will be closed on exec.
  #
  # ```ruby
  # f = open("/dev/null")
  # f.close_on_exec?                 #=> false
  # f.close_on_exec = true
  # f.close_on_exec?                 #=> true
  # f.close_on_exec = false
  # f.close_on_exec?                 #=> false
  # ```
  sig {returns(T::Boolean)}
  def close_on_exec?(); end

  # Closes the read end of a duplex I/O stream (i.e., one that contains both a
  # read and a write stream, such as a pipe). Will raise an `IOError` if the
  # stream is not duplexed.
  #
  # ```ruby
  # f = IO.popen("/bin/sh","r+")
  # f.close_read
  # f.readlines
  # ```
  #
  # *produces:*
  #
  # ```
  # prog.rb:3:in `readlines': not opened for reading (IOError)
  #  from prog.rb:3
  # ```
  #
  # Calling this method on closed
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object is just ignored
  # since Ruby 2.3.
  sig {returns(NilClass)}
  def close_read(); end

  # Closes the write end of a duplex I/O stream (i.e., one that contains both a
  # read and a write stream, such as a pipe). Will raise an `IOError` if the
  # stream is not duplexed.
  #
  # ```ruby
  # f = IO.popen("/bin/sh","r+")
  # f.close_write
  # f.print "nowhere"
  # ```
  #
  # *produces:*
  #
  # ```
  # prog.rb:3:in `write': not opened for writing (IOError)
  #  from prog.rb:3:in `print'
  #  from prog.rb:3
  # ```
  #
  # Calling this method on closed
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object is just ignored
  # since Ruby 2.3.
  sig {returns(NilClass)}
  def close_write(); end

  # Returns `true` if *ios* is completely closed (for duplex streams, both
  # reader and writer), `false` otherwise.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.close         #=> nil
  # f.closed?       #=> true
  # f = IO.popen("/bin/sh","r+")
  # f.close_write   #=> nil
  # f.closed?       #=> false
  # f.close_read    #=> nil
  # f.closed?       #=> true
  # ```
  sig {returns(T::Boolean)}
  def closed?(); end

  # Executes the block for every line in *ios*, where lines are separated by
  # *sep*. *ios* must be opened for reading or an `IOError` will be raised.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.each {|line| puts "#{f.lineno}: #{line}" }
  # ```
  #
  # *produces:*
  #
  # ```
  # 1: This is line one
  # 2: This is line two
  # 3: This is line three
  # 4: And so on...
  # ```
  #
  # See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-readlines)
  # for details about getline\_args.
  sig do
    params(
        sep: String,
        limit: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Enumerator[String])
  end
  def each(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  # Calls the given block once for each byte (0..255) in *ios*, passing the byte
  # as an argument. The stream must be opened for reading or an `IOError` will
  # be raised.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # f = File.new("testfile")
  # checksum = 0
  # f.each_byte {|x| checksum ^= x }   #=> #<File:testfile>
  # checksum                           #=> 12
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_byte(&blk); end

  # Calls the given block once for each character in *ios*, passing the
  # character as an argument. The stream must be opened for reading or an
  # `IOError` will be raised.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.each_char {|c| print c, ' ' }   #=> #<File:testfile>
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[String])}
  def each_char(&blk); end

  # Passes the `Integer` ordinal of each character in *ios*, passing the
  # codepoint as an argument. The stream must be opened for reading or an
  # `IOError` will be raised.
  #
  # If no block is given, an enumerator is returned instead.
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_codepoint(&blk); end

  # Returns true if *ios* is at end of file that means there are no more data to
  # read. The stream must be opened for reading or an `IOError` will be raised.
  #
  # ```ruby
  # f = File.new("testfile")
  # dummy = f.readlines
  # f.eof   #=> true
  # ```
  #
  # If *ios* is a stream such as pipe or socket, `IO#eof?` blocks until the
  # other end sends some data or closes it.
  #
  # ```ruby
  # r, w = IO.pipe
  # Thread.new { sleep 1; w.close }
  # r.eof?  #=> true after 1 second blocking
  #
  # r, w = IO.pipe
  # Thread.new { sleep 1; w.puts "a" }
  # r.eof?  #=> false after 1 second blocking
  #
  # r, w = IO.pipe
  # r.eof?  # blocks forever
  # ```
  #
  # Note that `IO#eof?` reads data to the input byte buffer. So `IO#sysread` may
  # not behave as you intend with `IO#eof?`, unless you call `IO#rewind` first
  # (which is not available for some streams).
  sig {returns(T::Boolean)}
  def eof(); end

  # Provides a mechanism for issuing low-level commands to control or query
  # file-oriented I/O streams. Arguments and results are platform dependent. If
  # *arg* is a number, its value is passed directly. If it is a string, it is
  # interpreted as a binary sequence of bytes (`Array#pack` might be a useful
  # way to build this string). On Unix platforms, see `fcntl(2)` for details.
  # Not implemented on all platforms.
  sig do
    params(
        integer_cmd: Integer,
        arg: T.any(String, Integer),
    )
    .returns(Integer)
  end
  def fcntl(integer_cmd, arg); end

  # Immediately writes all buffered data in *ios* to disk.
  #
  # If the underlying operating system does not support *fdatasync(2)*,
  # `IO#fsync` is called instead (which might raise a `NotImplementedError`).
  sig {returns(T.nilable(Integer))}
  def fdatasync(); end

  # Returns an integer representing the numeric file descriptor for *ios*.
  #
  # ```ruby
  # $stdin.fileno    #=> 0
  # $stdout.fileno   #=> 1
  # ```
  #
  #
  # Also aliased as:
  # [`to_i`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-to_i)
  sig {returns(Integer)}
  def fileno(); end

  # Flushes any buffered data within *ios* to the underlying operating system
  # (note that this is Ruby internal buffering only; the OS may buffer the data
  # as well).
  #
  # ```ruby
  # $stdout.print "no newline"
  # $stdout.flush
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # no newline
  # ```
  sig {returns(T.self_type)}
  def flush(); end

  # Immediately writes all buffered data in *ios* to disk. Note that `fsync`
  # differs from using `IO#sync=`. The latter ensures that data is flushed from
  # Ruby's buffers, but does not guarantee that the underlying operating system
  # actually writes it to disk.
  #
  # `NotImplementedError` is raised if the underlying operating system does not
  # support *fsync(2)*.
  sig {returns(T.nilable(Integer))}
  def fsync(); end

  # Gets the next 8-bit byte (0..255) from *ios*. Returns `nil` if called at end
  # of file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.getbyte   #=> 84
  # f.getbyte   #=> 104
  # ```
  sig {returns(T.nilable(Integer))}
  def getbyte(); end

  # Reads a one-character string from *ios*. Returns `nil` if called at end of
  # file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.getc   #=> "h"
  # f.getc   #=> "e"
  # ```
  sig {returns(T.nilable(String))}
  def getc(); end

  # Reads the next "line" from the I/O stream; lines are separated by *sep*. A
  # separator of `nil` reads the entire contents, and a zero-length separator
  # reads the input a paragraph at a time (two successive newlines in the input
  # separate paragraphs). The stream must be opened for reading or an `IOError`
  # will be raised. The line read in will be returned and also assigned to `$_`.
  # Returns `nil` if called at end of file. If the first argument is an integer,
  # or optional second argument is given, the returning string would not be
  # longer than the given value in bytes.
  #
  # ```ruby
  # File.new("testfile").gets   #=> "This is line one\n"
  # $_                          #=> "This is line one\n"
  #
  # File.new("testfile").gets(4)#=> "This"
  # ```
  #
  # If [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) contains multibyte
  # characters byte then `gets(1)` returns character entirely:
  #
  # ```ruby
  # # Russian characters take 2 bytes
  # File.write("testfile", "\u{442 435 441 442}")
  # File.open("testfile") {|f|f.gets(1)} #=> "\u0442"
  # File.open("testfile") {|f|f.gets(2)} #=> "\u0442"
  # File.open("testfile") {|f|f.gets(3)} #=> "\u0442\u0435"
  # File.open("testfile") {|f|f.gets(4)} #=> "\u0442\u0435"
  # ```
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T.nilable(String))
  end
  def gets(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  sig do
    params(
        fd: Integer,
        mode: Integer,
        opt: Integer,
    )
    .void
  end
  def initialize(fd, mode=T.unsafe(nil), opt=T.unsafe(nil)); end

  # Return a string describing this
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object.
  sig {returns(String)}
  def inspect(); end

  # Returns the [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html)
  # of the internal string if conversion is specified. Otherwise returns `nil`.
  sig {returns(Encoding)}
  def internal_encoding(); end

  # Provides a mechanism for issuing low-level commands to control or query I/O
  # devices. Arguments and results are platform dependent. If *arg* is a number,
  # its value is passed directly. If it is a string, it is interpreted as a
  # binary sequence of bytes. On Unix platforms, see `ioctl(2)` for details. Not
  # implemented on all platforms.
  sig do
    params(
        integer_cmd: Integer,
        arg: T.any(String, Integer),
    )
    .returns(Integer)
  end
  def ioctl(integer_cmd, arg); end

  # Returns `true` if *ios* is associated with a terminal device (tty), `false`
  # otherwise.
  #
  # ```ruby
  # File.new("testfile").isatty   #=> false
  # File.new("/dev/tty").isatty   #=> true
  # ```
  sig {returns(T::Boolean)}
  def isatty(); end

  # Returns the current line number in *ios*. The stream must be opened for
  # reading. `lineno` counts the number of times
  # [`gets`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-gets) is
  # called rather than the number of newlines encountered. The two values will
  # differ if
  # [`gets`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-gets) is
  # called with a separator other than newline.
  #
  # Methods that use `$/` like
  # [`each`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-each),
  # [`lines`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-lines) and
  # [`readline`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-readline)
  # will also increment `lineno`.
  #
  # See also the `$.` variable.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.lineno   #=> 0
  # f.gets     #=> "This is line one\n"
  # f.lineno   #=> 1
  # f.gets     #=> "This is line two\n"
  # f.lineno   #=> 2
  # ```
  sig {returns(Integer)}
  def lineno(); end

  # Manually sets the current line number to the given value. `$.` is updated
  # only on the next read.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.gets                     #=> "This is line one\n"
  # $.                         #=> 1
  # f.lineno = 1000
  # f.lineno                   #=> 1000
  # $.                         #=> 1         # lineno of last read
  # f.gets                     #=> "This is line two\n"
  # $.                         #=> 1001      # lineno of last read
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def lineno=(arg0); end

  # Returns the process ID of a child process associated with *ios*. This will
  # be set by `IO.popen`.
  #
  # ```ruby
  # pipe = IO.popen("-")
  # if pipe
  #   $stderr.puts "In parent, child pid is #{pipe.pid}"
  # else
  #   $stderr.puts "In child, pid is #{$$}"
  # end
  # ```
  #
  # *produces:*
  #
  # ```
  # In child, pid is 26209
  # In parent, child pid is 26209
  # ```
  sig {returns(Integer)}
  def pid(); end

  # Returns the current offset (in bytes) of *ios*.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.pos    #=> 0
  # f.gets   #=> "This is line one\n"
  # f.pos    #=> 17
  # ```
  sig {returns(Integer)}
  def pos(); end

  # Seeks to the given position (in bytes) in *ios*. It is not guaranteed that
  # seeking to the right position when *ios* is textmode.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.pos = 17
  # f.gets   #=> "This is line two\n"
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def pos=(arg0); end

  # Writes the given object(s) to *ios*. Returns `nil`.
  #
  # The stream must be opened for writing. Each given object that isn't a string
  # will be converted by calling its `to_s` method. When called without
  # arguments, prints the contents of `$_`.
  #
  # If the output field separator (`$,`) is not `nil`, it is inserted between
  # objects. If the output record separator (`$\`) is not `nil`, it is appended
  # to the output.
  #
  # ```ruby
  # $stdout.print("This is ", 100, " percent.\n")
  # ```
  #
  # *produces:*
  #
  # ```
  # This is 100 percent.
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def print(*arg0); end

  # Formats and writes to *ios*, converting parameters under control of the
  # format string. See `Kernel#sprintf` for details.
  sig do
    params(
        format_string: String,
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def printf(format_string, *arg0); end

  # If *obj* is `Numeric`, write the character whose code is the
  # least-significant byte of *obj*. If *obj* is `String`, write the first
  # character of *obj* to *ios*. Otherwise, raise `TypeError`.
  #
  # ```ruby
  # $stdout.putc "A"
  # $stdout.putc 65
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # AA
  # ```
  sig do
    params(
        arg0: T.any(Numeric, String),
    )
    .returns(T.untyped)
  end
  def putc(arg0); end

  # Writes the given object(s) to *ios*. Writes a newline after any that do not
  # already end with a newline sequence. Returns `nil`.
  #
  # The stream must be opened for writing. If called with an array argument,
  # writes each element on a new line. Each given object that isn't a string or
  # array will be converted by calling its `to_s` method. If called without
  # arguments, outputs a single newline.
  #
  # ```ruby
  # $stdout.puts("this", "is", ["a", "test"])
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # this
  # is
  # a
  # test
  # ```
  #
  # Note that `puts` always uses newlines and is not affected by the output
  # record separator (`$\`).
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def puts(*arg0); end

  # Reads *length* bytes from the I/O stream.
  #
  # *length* must be a non-negative integer or `nil`.
  #
  # If *length* is a positive integer, `read` tries to read *length* bytes
  # without any conversion (binary mode). It returns `nil` if an EOF is
  # encountered before anything can be read. Fewer than *length* bytes are
  # returned if an EOF is encountered during the read. In the case of an integer
  # *length*, the resulting string is always in ASCII-8BIT encoding.
  #
  # If *length* is omitted or is `nil`, it reads until EOF and the encoding
  # conversion is applied, if applicable. A string is returned even if EOF is
  # encountered before any data is read.
  #
  # If *length* is zero, it returns an empty string (`""`).
  #
  # If the optional *outbuf* argument is present, it must reference a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), which will
  # receive the data. The *outbuf* will contain only the received data after the
  # method call even if it is not empty at the beginning.
  #
  # When this method is called at end of file, it returns `nil` or `""`,
  # depending on *length*: `read`, `read(nil)`, and `read(0)` return `""`,
  # `read(positive_integer)` returns `nil`.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.read(16)   #=> "This is line one"
  #
  # # read whole file
  # open("file") do |f|
  #   data = f.read   # This returns a string even if the file is empty.
  #   # ...
  # end
  #
  # # iterate over fixed length records
  # open("fixed-record-file") do |f|
  #   while record = f.read(256)
  #     # ...
  #   end
  # end
  #
  # # iterate over variable length records,
  # # each record is prefixed by its 32-bit length
  # open("variable-record-file") do |f|
  #   while len = f.read(4)
  #     len = len.unpack("N")[0]   # 32-bit length
  #     record = f.read(len)       # This returns a string even if len is 0.
  #   end
  # end
  # ```
  #
  # Note that this method behaves like the fread() function in C. This means it
  # retries to invoke read(2) system calls to read data with the specified
  # length (or until EOF). This behavior is preserved even if *ios* is in
  # non-blocking mode. (This method is non-blocking flag insensitive as other
  # methods.) If you need the behavior like a single read(2) system call,
  # consider
  # [`readpartial`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-readpartial),
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-read_nonblock),
  # and
  # [`sysread`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-sysread).
  sig do
    params(
        length: Integer,
        outbuf: String,
    )
    .returns(T.nilable(String))
  end
  def read(length=T.unsafe(nil), outbuf=T.unsafe(nil)); end

  # Reads at most *maxlen* bytes from *ios* using the read(2) system call after
  # O\_NONBLOCK is set for the underlying file descriptor.
  #
  # If the optional *outbuf* argument is present, it must reference a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), which will
  # receive the data. The *outbuf* will contain only the received data after the
  # method call even if it is not empty at the beginning.
  #
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-read_nonblock)
  # just calls the read(2) system call. It causes all errors the read(2) system
  # call causes: Errno::EWOULDBLOCK, Errno::EINTR, etc. The caller should care
  # such errors.
  #
  # If the exception is Errno::EWOULDBLOCK or Errno::EAGAIN, it is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitReadable.html).
  # So
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitReadable.html)
  # can be used to rescue the exceptions for retrying read\_nonblock.
  #
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-read_nonblock)
  # causes [`EOFError`](https://docs.ruby-lang.org/en/2.6.0/EOFError.html) on
  # EOF.
  #
  # If the read byte buffer is not empty,
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-read_nonblock)
  # reads from the buffer like readpartial. In this case, the read(2) system
  # call is not called.
  #
  # When
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-read_nonblock)
  # raises an exception kind of
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitReadable.html),
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-read_nonblock)
  # should not be called until io is readable for avoiding busy loop. This can
  # be done as follows.
  #
  # ```ruby
  # # emulates blocking read (readpartial).
  # begin
  #   result = io.read_nonblock(maxlen)
  # rescue IO::WaitReadable
  #   IO.select([io])
  #   retry
  # end
  # ```
  #
  # Although
  # [`IO#read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-read_nonblock)
  # doesn't raise
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitWritable.html).
  # [`OpenSSL::Buffering#read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # can raise
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitWritable.html).
  # If [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) and SSL should be
  # used polymorphically,
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitWritable.html)
  # should be rescued too. See the document of
  # [`OpenSSL::Buffering#read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # for sample code.
  #
  # Note that this method is identical to readpartial except the non-blocking
  # flag is set.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-read_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead. At EOF, it will
  # return nil instead of raising
  # [`EOFError`](https://docs.ruby-lang.org/en/2.6.0/EOFError.html).
  sig do
    params(
        len: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        len: Integer,
        buf: String,
    )
    .returns(String)
  end
  def read_nonblock(len, buf=T.unsafe(nil)); end

  # Reads a byte as with `IO#getbyte`, but raises an `EOFError` on end of file.
  sig {returns(Integer)}
  def readbyte(); end

  # Reads a one-character string from *ios*. Raises an `EOFError` on end of
  # file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.readchar   #=> "h"
  # f.readchar   #=> "e"
  # ```
  sig {returns(String)}
  def readchar(); end

  # Reads a line as with `IO#gets`, but raises an `EOFError` on end of file.
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(String)
  end
  def readline(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  # Reads all of the lines in *ios*, and returns them in an array. Lines are
  # separated by the optional *sep*. If *sep* is `nil`, the rest of the stream
  # is returned as a single record. If the first argument is an integer, or an
  # optional second argument is given, the returning string would not be longer
  # than the given value in bytes. The stream must be opened for reading or an
  # `IOError` will be raised.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.readlines[0]   #=> "This is line one\n"
  #
  # f = File.new("testfile", chomp: true)
  # f.readlines[0]   #=> "This is line one"
  # ```
  #
  # See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-readlines)
  # for details about getline\_args.
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Array[String])
  end
  def readlines(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  # Reads at most *maxlen* bytes from the I/O stream. It blocks only if *ios*
  # has no data immediately available. It doesn't block if some data available.
  #
  # If the optional *outbuf* argument is present, it must reference a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), which will
  # receive the data. The *outbuf* will contain only the received data after the
  # method call even if it is not empty at the beginning.
  #
  # It raises `EOFError` on end of file.
  #
  # readpartial is designed for streams such as pipe, socket, tty, etc. It
  # blocks only when no data immediately available. This means that it blocks
  # only when following all conditions hold.
  # *   the byte buffer in the
  #     [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object is empty.
  # *   the content of the stream is empty.
  # *   the stream is not reached to EOF.
  #
  #
  # When readpartial blocks, it waits data or EOF on the stream. If some data is
  # reached, readpartial returns with the data. If EOF is reached, readpartial
  # raises [`EOFError`](https://docs.ruby-lang.org/en/2.6.0/EOFError.html).
  #
  # When readpartial doesn't blocks, it returns or raises immediately. If the
  # byte buffer is not empty, it returns the data in the buffer. Otherwise if
  # the stream has some content, it returns the data in the stream. Otherwise if
  # the stream is reached to EOF, it raises
  # [`EOFError`](https://docs.ruby-lang.org/en/2.6.0/EOFError.html).
  #
  # ```ruby
  # r, w = IO.pipe           #               buffer          pipe content
  # w << "abc"               #               ""              "abc".
  # r.readpartial(4096)      #=> "abc"       ""              ""
  # r.readpartial(4096)      # blocks because buffer and pipe is empty.
  #
  # r, w = IO.pipe           #               buffer          pipe content
  # w << "abc"               #               ""              "abc"
  # w.close                  #               ""              "abc" EOF
  # r.readpartial(4096)      #=> "abc"       ""              EOF
  # r.readpartial(4096)      # raises EOFError
  #
  # r, w = IO.pipe           #               buffer          pipe content
  # w << "abc\ndef\n"        #               ""              "abc\ndef\n"
  # r.gets                   #=> "abc\n"     "def\n"         ""
  # w << "ghi\n"             #               "def\n"         "ghi\n"
  # r.readpartial(4096)      #=> "def\n"     ""              "ghi\n"
  # r.readpartial(4096)      #=> "ghi\n"     ""              ""
  # ```
  #
  # Note that readpartial behaves similar to sysread. The differences are:
  # *   If the byte buffer is not empty, read from the byte buffer instead of
  #     "sysread for buffered
  #     [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) (IOError)".
  # *   It doesn't cause Errno::EWOULDBLOCK and Errno::EINTR. When readpartial
  #     meets EWOULDBLOCK and EINTR by read system call, readpartial retry the
  #     system call.
  #
  #
  # The latter means that readpartial is nonblocking-flag insensitive. It blocks
  # on the situation
  # [`IO#sysread`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-sysread)
  # causes Errno::EWOULDBLOCK as if the fd is blocking mode.
  sig do
    params(
        maxlen: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        maxlen: Integer,
        outbuf: String,
    )
    .returns(String)
  end
  def readpartial(maxlen, outbuf=T.unsafe(nil)); end

  # Reassociates *ios* with the I/O stream given in *other\_IO* or to a new
  # stream opened on *path*. This may dynamically change the actual class of
  # this stream. The `mode` and `opt` parameters accept the same values as
  # [`IO.open`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-open).
  #
  # ```ruby
  # f1 = File.new("testfile")
  # f2 = File.new("testfile")
  # f2.readlines[0]   #=> "This is line one\n"
  # f2.reopen(f1)     #=> #<File:testfile>
  # f2.readlines[0]   #=> "This is line one\n"
  # ```
  sig do
    params(
        other_IO_or_path: IO,
    )
    .returns(IO)
  end
  sig do
    params(
        other_IO_or_path: String,
        mode_str: String,
    )
    .returns(IO)
  end
  def reopen(other_IO_or_path, mode_str=T.unsafe(nil)); end

  # Positions *ios* to the beginning of input, resetting `lineno` to zero.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.readline   #=> "This is line one\n"
  # f.rewind     #=> 0
  # f.lineno     #=> 0
  # f.readline   #=> "This is line one\n"
  # ```
  #
  # Note that it cannot be used with streams such as pipes, ttys, and sockets.
  sig {returns(Integer)}
  def rewind(); end

  # Seeks to a given offset *anInteger* in the stream according to the value of
  # *whence*:
  #
  # ```
  # :CUR or IO::SEEK_CUR  | Seeks to _amount_ plus current position
  # ----------------------+--------------------------------------------------
  # :END or IO::SEEK_END  | Seeks to _amount_ plus end of stream (you
  #                       | probably want a negative value for _amount_)
  # ----------------------+--------------------------------------------------
  # :SET or IO::SEEK_SET  | Seeks to the absolute location given by _amount_
  # ```
  #
  # Example:
  #
  # ```ruby
  # f = File.new("testfile")
  # f.seek(-13, IO::SEEK_END)   #=> 0
  # f.readline                  #=> "And so on...\n"
  # ```
  sig do
    params(
        amount: Integer,
        whence: Integer,
    )
    .returns(Integer)
  end
  def seek(amount, whence=T.unsafe(nil)); end

  # If single argument is specified, read string from io is tagged with the
  # encoding specified. If encoding is a colon separated two encoding names
  # "A:B", the read string is converted from encoding A (external encoding) to
  # encoding B (internal encoding), then tagged with B. If two arguments are
  # specified, those must be encoding objects or encoding names, and the first
  # one is the external encoding, and the second one is the internal encoding.
  # If the external encoding and the internal encoding is specified, optional
  # hash argument specify the conversion option.
  sig do
    params(
        ext_or_ext_int_enc: T.any(String, Encoding),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        ext_or_ext_int_enc: T.any(String, Encoding),
        int_enc: T.any(String, Encoding),
    )
    .returns(T.self_type)
  end
  def set_encoding(ext_or_ext_int_enc=T.unsafe(nil), int_enc=T.unsafe(nil)); end

  # Returns status information for *ios* as an object of type `File::Stat`.
  #
  # ```ruby
  # f = File.new("testfile")
  # s = f.stat
  # "%o" % s.mode   #=> "100644"
  # s.blksize       #=> 4096
  # s.atime         #=> Wed Apr 09 08:53:54 CDT 2003
  # ```
  sig {returns(File::Stat)}
  def stat(); end

  # Returns the current "sync mode" of *ios*. When sync mode is true, all output
  # is immediately flushed to the underlying operating system and is not
  # buffered by Ruby internally. See also `IO#fsync`.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.sync   #=> false
  # ```
  sig {returns(T::Boolean)}
  def sync(); end

  # Sets the "sync mode" to `true` or `false`. When sync mode is true, all
  # output is immediately flushed to the underlying operating system and is not
  # buffered internally. Returns the new state. See also `IO#fsync`.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.sync = true
  # ```
  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def sync=(arg0); end

  # Reads *maxlen* bytes from *ios* using a low-level read and returns them as a
  # string. Do not mix with other methods that read from *ios* or you may get
  # unpredictable results.
  #
  # If the optional *outbuf* argument is present, it must reference a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), which will
  # receive the data. The *outbuf* will contain only the received data after the
  # method call even if it is not empty at the beginning.
  #
  # Raises `SystemCallError` on error and `EOFError` at end of file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.sysread(16)   #=> "This is line one"
  # ```
  sig do
    params(
        maxlen: Integer,
        outbuf: String,
    )
    .returns(String)
  end
  def sysread(maxlen, outbuf); end

  # Seeks to a given *offset* in the stream according to the value of *whence*
  # (see `IO#seek` for values of *whence*). Returns the new offset into the
  # file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.sysseek(-13, IO::SEEK_END)   #=> 53
  # f.sysread(10)                  #=> "And so on."
  # ```
  sig do
    params(
        amount: Integer,
        whence: Integer,
    )
    .returns(Integer)
  end
  def sysseek(amount, whence=T.unsafe(nil)); end

  # Writes the given string to *ios* using a low-level write. Returns the number
  # of bytes written. Do not mix with other methods that write to *ios* or you
  # may get unpredictable results. Raises `SystemCallError` on error.
  #
  # ```ruby
  # f = File.new("out", "w")
  # f.syswrite("ABCDEF")   #=> 6
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def syswrite(arg0); end

  # Returns the current offset (in bytes) of *ios*.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.pos    #=> 0
  # f.gets   #=> "This is line one\n"
  # f.pos    #=> 17
  # ```
  sig {returns(Integer)}
  def tell(); end

  # Returns *ios*.
  sig {returns(T.self_type)}
  def to_io(); end

  # Returns `true` if *ios* is associated with a terminal device (tty), `false`
  # otherwise.
  #
  # ```ruby
  # File.new("testfile").isatty   #=> false
  # File.new("/dev/tty").isatty   #=> true
  # ```
  sig {returns(T::Boolean)}
  def tty?(); end

  # Pushes back bytes (passed as a parameter) onto *ios*, such that a subsequent
  # buffered read will return it. Only one byte may be pushed back before a
  # subsequent read operation (that is, you will be able to read only the last
  # of several bytes that have been pushed back). Has no effect with unbuffered
  # reads (such as `IO#sysread`).
  #
  # ```ruby
  # f = File.new("testfile")   #=> #<File:testfile>
  # b = f.getbyte              #=> 0x38
  # f.ungetbyte(b)             #=> nil
  # f.getbyte                  #=> 0x38
  # ```
  sig do
    params(
        arg0: T.any(String, Integer),
    )
    .returns(NilClass)
  end
  def ungetbyte(arg0); end

  # Pushes back one character (passed as a parameter) onto *ios*, such that a
  # subsequent buffered character read will return it. Only one character may be
  # pushed back before a subsequent read operation (that is, you will be able to
  # read only the last of several characters that have been pushed back). Has no
  # effect with unbuffered reads (such as `IO#sysread`).
  #
  # ```ruby
  # f = File.new("testfile")   #=> #<File:testfile>
  # c = f.getc                 #=> "8"
  # f.ungetc(c)                #=> nil
  # f.getc                     #=> "8"
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(NilClass)
  end
  def ungetc(arg0); end

  # Writes the given strings to *ios*. The stream must be opened for writing.
  # Arguments that are not a string will be converted to a string using `to_s`.
  # Returns the number of bytes written in total.
  #
  # ```ruby
  # count = $stdout.write("This is", " a test\n")
  # puts "That was #{count} bytes of data"
  # ```
  #
  # *produces:*
  #
  # ```
  # This is a test
  # That was 15 bytes of data
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def write(arg0); end

  # Opens the file, optionally seeks to the given *offset*, then returns
  # *length* bytes (defaulting to the rest of the file). `binread` ensures the
  # file is closed before returning. The open mode would be "rb:ASCII-8BIT".
  #
  # ```ruby
  # IO.binread("testfile")           #=> "This is line one\nThis is line two\nThis is line three\nAnd so on...\n"
  # IO.binread("testfile", 20)       #=> "This is line one\nThi"
  # IO.binread("testfile", 20, 10)   #=> "ne one\nThis is line "
  # ```
  sig do
    params(
        name: String,
        length: Integer,
        offset: Integer,
    )
    .returns(String)
  end
  def self.binread(name, length=T.unsafe(nil), offset=T.unsafe(nil)); end

  # Same as `IO.write` except opening the file in binary mode and ASCII-8BIT
  # encoding ("wb:ASCII-8BIT").
  sig do
    params(
        name: String,
        arg0: String,
        offset: Integer,
        external_encoding: String,
        internal_encoding: String,
        encoding: String,
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(Integer)
  end
  def self.binwrite(name, arg0, offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  # [`IO.copy_stream`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-copy_stream)
  # copies *src* to *dst*. *src* and *dst* is either a filename or an IO-like
  # object. IO-like object for *src* should have `readpartial` or `read` method.
  # IO-like object for *dst* should have `write` method. (Specialized
  # mechanisms, such as sendfile system call, may be used on appropriate
  # situation.)
  #
  # This method returns the number of bytes copied.
  #
  # If optional arguments are not given, the start position of the copy is the
  # beginning of the filename or the current file offset of the
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html). The end position of the
  # copy is the end of file.
  #
  # If *copy\_length* is given, No more than *copy\_length* bytes are copied.
  #
  # If *src\_offset* is given, it specifies the start position of the copy.
  #
  # When *src\_offset* is specified and *src* is an
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html),
  # [`IO.copy_stream`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-copy_stream)
  # doesn't move the current file offset.
  sig do
    params(
        src: T.any(String, IO),
        dst: T.any(String, IO),
        copy_length: Integer,
        src_offset: Integer,
    )
    .returns(Integer)
  end
  def self.copy_stream(src, dst, copy_length=T.unsafe(nil), src_offset=T.unsafe(nil)); end

  ### https://ruby-doc.org/core-2.3.0/IO.html#method-c-popen
  ### This signature is very hard to type. I'm giving up and making it untyped.
  ### As far as I can tell, at least one arg is required, and it must be an array,
  ### but sometimes it's the first arg and sometimes it's the second arg, so
  ### let's just make everything untyped.
  ### TODO(jez) Have to declare this as a rest arg, because pay-server runtime
  ### reflection sees it this way. Once it's out of the missing method file, we
  ### can add a better sig here.
  # Runs the specified command as a subprocess; the subprocess's standard input
  # and output will be connected to the returned `IO` object.
  #
  # The PID of the started process can be obtained by
  # [`IO#pid`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-pid) method.
  #
  # *cmd* is a string or an array as follows.
  #
  # ```
  # cmd:
  #   "-"                                      : fork
  #   commandline                              : command line string which is passed to a shell
  #   [env, cmdname, arg1, ..., opts]          : command name and zero or more arguments (no shell)
  #   [env, [cmdname, argv0], arg1, ..., opts] : command name, argv[0] and zero or more arguments (no shell)
  # (env and opts are optional.)
  # ```
  #
  # If *cmd* is a `String` "`-`", then a new instance of Ruby is started as the
  # subprocess.
  #
  # If *cmd* is an `Array` of `String`, then it will be used as the subprocess's
  # `argv` bypassing a shell. The array can contain a hash at first for
  # environments and a hash at last for options similar to `spawn`.
  #
  # The default mode for the new file object is "r", but *mode* may be set to
  # any of the modes listed in the description for class
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html). The last argument *opt*
  # qualifies *mode*.
  #
  # ```ruby
  # # set IO encoding
  # IO.popen("nkf -e filename", :external_encoding=>"EUC-JP") {|nkf_io|
  #   euc_jp_string = nkf_io.read
  # }
  #
  # # merge standard output and standard error using
  # # spawn option.  See the document of Kernel.spawn.
  # IO.popen(["ls", "/", :err=>[:child, :out]]) {|ls_io|
  #   ls_result_with_error = ls_io.read
  # }
  #
  # # spawn options can be mixed with IO options
  # IO.popen(["ls", "/"], :err=>[:child, :out]) {|ls_io|
  #   ls_result_with_error = ls_io.read
  # }
  # ```
  #
  # Raises exceptions which `IO.pipe` and `Kernel.spawn` raise.
  #
  # If a block is given, Ruby will run the command as a child connected to Ruby
  # with a pipe. Ruby's end of the pipe will be passed as a parameter to the
  # block. At the end of block, Ruby closes the pipe and sets `$?`. In this case
  # `IO.popen` returns the value of the block.
  #
  # If a block is given with a *cmd* of "`-`", the block will be run in two
  # separate processes: once in the parent, and once in a child. The parent
  # process will be passed the pipe object as a parameter to the block, the
  # child version of the block will be passed `nil`, and the child's standard in
  # and standard out will be connected to the parent through the pipe. Not
  # available on all platforms.
  #
  # ```ruby
  # f = IO.popen("uname")
  # p f.readlines
  # f.close
  # puts "Parent is #{Process.pid}"
  # IO.popen("date") {|f| puts f.gets }
  # IO.popen("-") {|f| $stderr.puts "#{Process.pid} is here, f is #{f.inspect}"}
  # p $?
  # IO.popen(%w"sed -e s|^|<foo>| -e s&$&;zot;&", "r+") {|f|
  #   f.puts "bar"; f.close_write; puts f.gets
  # }
  # ```
  #
  # *produces:*
  #
  # ```
  # ["Linux\n"]
  # Parent is 21346
  # Thu Jan 15 22:41:19 JST 2009
  # 21346 is here, f is #<IO:fd 3>
  # 21352 is here, f is nil
  # #<Process::Status: pid 21352 exit 0>
  # <foo>bar;zot;
  # ```
  sig do
    params(
        args: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.popen(*args); end

  # Opens the file, optionally seeks to the given `offset`, then returns
  # `length` bytes (defaulting to the rest of the file). `read` ensures the file
  # is closed before returning.
  #
  # If `name` starts with a pipe character (`"|"`), a subprocess is created in
  # the same way as
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open),
  # and its output is returned.
  #
  # ### Options
  #
  # The options hash accepts the following keys:
  #
  # :encoding
  # :   string or encoding
  #
  #     Specifies the encoding of the read string. `:encoding` will be ignored
  #     if `length` is specified. See
  #     [`Encoding.aliases`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-aliases)
  #     for possible encodings.
  #
  # :mode
  # :   string or integer
  #
  #     Specifies the *mode* argument for open(). It must start with an "r",
  #     otherwise it will cause an error. See
  #     [`IO.new`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-new) for
  #     the list of possible modes.
  #
  # :open\_args
  # :   array
  #
  #     Specifies arguments for open() as an array. This key can not be used in
  #     combination with either `:encoding` or `:mode`.
  #
  #
  # Examples:
  #
  # ```ruby
  # IO.read("testfile")              #=> "This is line one\nThis is line two\nThis is line three\nAnd so on...\n"
  # IO.read("testfile", 20)          #=> "This is line one\nThi"
  # IO.read("testfile", 20, 10)      #=> "ne one\nThis is line "
  # IO.read("binfile", mode: "rb")   #=> "\xF7\x00\x00\x0E\x12"
  # ```
  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        length: Integer,
        offset: Integer,
        external_encoding: String,
        internal_encoding: String,
        encoding: String,
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(String)
  end
  def self.read(name, length=T.unsafe(nil), offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  # Reads the entire file specified by *name* as individual lines, and returns
  # those lines in an array. Lines are separated by *sep*.
  #
  # ```ruby
  # a = IO.readlines("testfile")
  # a[0]   #=> "This is line one\n"
  #
  # b = IO.readlines("testfile", chomp: true)
  # b[0]   #=> "This is line one"
  # ```
  #
  # If the last argument is a hash, it's the keyword argument to open.
  #
  # ### Options for getline
  #
  # The options hash accepts the following keys:
  #
  # :chomp
  # :   When the optional `chomp` keyword argument has a true value, `\n`, `\r`,
  #     and `\r\n` will be removed from the end of each line.
  #
  #
  # See also
  # [`IO.read`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-read) for
  # details about open\_args.
  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        sep: String,
        limit: Integer,
        external_encoding: String,
        internal_encoding: String,
        encoding: String,
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(T::Array[String])
  end
  def self.readlines(name, sep=T.unsafe(nil), limit=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  # Calls select(2) system call. It monitors given arrays of `IO` objects, waits
  # until one or more of `IO` objects are ready for reading, are ready for
  # writing, and have pending exceptions respectively, and returns an array that
  # contains arrays of those [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html)
  # objects. It will return `nil` if optional *timeout* value is given and no
  # `IO` object is ready in *timeout* seconds.
  #
  # `IO.select` peeks the buffer of `IO` objects for testing readability. If the
  # `IO` buffer is not empty, `IO.select` immediately notifies readability. This
  # "peek" only happens for `IO` objects. It does not happen for IO-like objects
  # such as
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.6.0/OpenSSL/SSL/SSLSocket.html).
  #
  # The best way to use `IO.select` is invoking it after nonblocking methods
  # such as `read_nonblock`, `write_nonblock`, etc. The methods raise an
  # exception which is extended by `IO::WaitReadable` or `IO::WaitWritable`. The
  # modules notify how the caller should wait with `IO.select`. If
  # `IO::WaitReadable` is raised, the caller should wait for reading. If
  # `IO::WaitWritable` is raised, the caller should wait for writing.
  #
  # So, blocking read (`readpartial`) can be emulated using `read_nonblock` and
  # `IO.select` as follows:
  #
  # ```ruby
  # begin
  #   result = io_like.read_nonblock(maxlen)
  # rescue IO::WaitReadable
  #   IO.select([io_like])
  #   retry
  # rescue IO::WaitWritable
  #   IO.select(nil, [io_like])
  #   retry
  # end
  # ```
  #
  # Especially, the combination of nonblocking methods and `IO.select` is
  # preferred for `IO` like objects such as `OpenSSL::SSL::SSLSocket`. It has
  # `to_io` method to return underlying `IO` object. `IO.select` calls `to_io`
  # to obtain the file descriptor to wait.
  #
  # This means that readability notified by `IO.select` doesn't mean readability
  # from `OpenSSL::SSL::SSLSocket` object.
  #
  # The most likely situation is that `OpenSSL::SSL::SSLSocket` buffers some
  # data. `IO.select` doesn't see the buffer. So `IO.select` can block when
  # `OpenSSL::SSL::SSLSocket#readpartial` doesn't block.
  #
  # However, several more complicated situations exist.
  #
  # SSL is a protocol which is sequence of records. The record consists of
  # multiple bytes. So, the remote side of SSL sends a partial record,
  # `IO.select` notifies readability but `OpenSSL::SSL::SSLSocket` cannot
  # decrypt a byte and `OpenSSL::SSL::SSLSocket#readpartial` will block.
  #
  # Also, the remote side can request SSL renegotiation which forces the local
  # SSL engine to write some data. This means
  # `OpenSSL::SSL::SSLSocket#readpartial` may invoke `write` system call and it
  # can block. In such a situation, `OpenSSL::SSL::SSLSocket#read_nonblock`
  # raises
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitWritable.html)
  # instead of blocking. So, the caller should wait for ready for writability as
  # above example.
  #
  # The combination of nonblocking methods and `IO.select` is also useful for
  # streams such as tty, pipe socket socket when multiple processes read from a
  # stream.
  #
  # Finally, Linux kernel developers don't guarantee that readability of
  # select(2) means readability of following read(2) even for a single process.
  # See select(2) manual on GNU/Linux system.
  #
  # Invoking `IO.select` before `IO#readpartial` works well as usual. However it
  # is not the best way to use `IO.select`.
  #
  # The writability notified by select(2) doesn't show how many bytes are
  # writable. `IO#write` method blocks until given whole string is written. So,
  # `IO#write(two or more bytes)` can block after writability is notified by
  # `IO.select`. `IO#write_nonblock` is required to avoid the blocking.
  #
  # Blocking write (`write`) can be emulated using `write_nonblock` and
  # `IO.select` as follows:
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.6.0/IO/WaitReadable.html)
  # should also be rescued for SSL renegotiation in `OpenSSL::SSL::SSLSocket`.
  #
  # ```ruby
  # while 0 < string.bytesize
  #   begin
  #     written = io_like.write_nonblock(string)
  #   rescue IO::WaitReadable
  #     IO.select([io_like])
  #     retry
  #   rescue IO::WaitWritable
  #     IO.select(nil, [io_like])
  #     retry
  #   end
  #   string = string.byteslice(written..-1)
  # end
  # ```
  #
  # ### Parameters
  # read\_array
  # :   an array of `IO` objects that wait until ready for read
  # write\_array
  # :   an array of `IO` objects that wait until ready for write
  # error\_array
  # :   an array of `IO` objects that wait for exceptions
  # timeout
  # :   a numeric value in second
  #
  #
  # ### Example
  #
  # ```ruby
  # rp, wp = IO.pipe
  # mesg = "ping "
  # 100.times {
  #   # IO.select follows IO#read.  Not the best way to use IO.select.
  #   rs, ws, = IO.select([rp], [wp])
  #   if r = rs[0]
  #     ret = r.read(5)
  #     print ret
  #     case ret
  #     when /ping/
  #       mesg = "pong\n"
  #     when /pong/
  #       mesg = "ping "
  #     end
  #   end
  #   if w = ws[0]
  #     w.write(mesg)
  #   end
  # }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # ping pong
  # ping pong
  # ping pong
  # (snipped)
  # ping
  # ```
  sig do
    params(
        read_array: T.nilable(T::Array[IO]),
        write_array: T.nilable(T::Array[IO]),
        error_array: T.nilable(T::Array[IO]),
        timeout: T.nilable(Integer),
    )
    .returns(T.nilable(T::Array[T::Array[IO]]))
  end
  def self.select(read_array, write_array=nil, error_array=nil, timeout=nil); end

  # Opens the given path, returning the underlying file descriptor as a
  # `Integer`.
  #
  # ```ruby
  # IO.sysopen("testfile")   #=> 3
  # ```
  sig do
    params(
        path: T.any(String, Tempfile, File, Pathname),
        mode: String,
        perm: String,
    )
    .returns(Integer)
  end
  def self.sysopen(path, mode=T.unsafe(nil), perm=T.unsafe(nil)); end

  # Try to convert *obj* into an
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html), using
  # [`to_io`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-to_io)
  # method. Returns converted
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) or `nil` if *obj* cannot
  # be converted for any reason.
  #
  # ```ruby
  # IO.try_convert(STDOUT)     #=> STDOUT
  # IO.try_convert("STDOUT")   #=> nil
  #
  # require 'zlib'
  # f = open("/tmp/zz.gz")       #=> #<File:/tmp/zz.gz>
  # z = Zlib::GzipReader.open(f) #=> #<Zlib::GzipReader:0x81d8744>
  # IO.try_convert(z)            #=> #<File:/tmp/zz.gz>
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.nilable(IO))
  end
  def self.try_convert(arg0); end

  # Opens the file, optionally seeks to the given *offset*, writes *string*,
  # then returns the length written. `write` ensures the file is closed before
  # returning. If *offset* is not given in write mode, the file is truncated.
  # Otherwise, it is not truncated.
  #
  # ```ruby
  # IO.write("testfile", "0123456789", 20)  #=> 10
  # # File could contain:  "This is line one\nThi0123456789two\nThis is line three\nAnd so on...\n"
  # IO.write("testfile", "0123456789")      #=> 10
  # # File would now read: "0123456789"
  # ```
  #
  # If the last argument is a hash, it specifies options for the internal
  # open(). It accepts the following keys:
  #
  # :encoding
  # :   string or encoding
  #
  #     Specifies the encoding of the read string. See
  #     [`Encoding.aliases`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-aliases)
  #     for possible encodings.
  #
  # :mode
  # :   string or integer
  #
  #     Specifies the *mode* argument for open(). It must start with "w", "a",
  #     or "r+", otherwise it will cause an error. See
  #     [`IO.new`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-new) for
  #     the list of possible modes.
  #
  # :perm
  # :   integer
  #
  #     Specifies the *perm* argument for open().
  #
  # :open\_args
  # :   array
  #
  #     Specifies arguments for open() as an array. This key can not be used in
  #     combination with other keys.
  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        arg0: String,
        offset: Integer,
        external_encoding: String,
        internal_encoding: String,
        encoding: String,
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(Integer)
  end
  def self.write(name, arg0, offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  # Synonym for `IO.new`.
  sig do
    params(
        fd: Integer,
        mode: Integer,
        opt: Integer,
    )
    .returns(T.self_type)
  end
  def self.for_fd(fd, mode=T.unsafe(nil), opt=T.unsafe(nil)); end

  # This is a deprecated alias for `each_byte`.
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def bytes(&blk); end

  # This is a deprecated alias for `each_char`.
  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[String])}
  def chars(&blk); end

  # This is a deprecated alias for `each_codepoint`.
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def codepoints(&blk); end

  # Executes the block for every line in *ios*, where lines are separated by
  # *sep*. *ios* must be opened for reading or an `IOError` will be raised.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.each {|line| puts "#{f.lineno}: #{line}" }
  # ```
  #
  # *produces:*
  #
  # ```
  # 1: This is line one
  # 2: This is line two
  # 3: This is line three
  # 4: And so on...
  # ```
  #
  # See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-readlines)
  # for details about getline\_args.
  sig do
    params(
        sep: String,
        limit: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Enumerator[String])
  end
  def each_line(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  # Returns true if *ios* is at end of file that means there are no more data to
  # read. The stream must be opened for reading or an `IOError` will be raised.
  #
  # ```ruby
  # f = File.new("testfile")
  # dummy = f.readlines
  # f.eof   #=> true
  # ```
  #
  # If *ios* is a stream such as pipe or socket, `IO#eof?` blocks until the
  # other end sends some data or closes it.
  #
  # ```ruby
  # r, w = IO.pipe
  # Thread.new { sleep 1; w.close }
  # r.eof?  #=> true after 1 second blocking
  #
  # r, w = IO.pipe
  # Thread.new { sleep 1; w.puts "a" }
  # r.eof?  #=> false after 1 second blocking
  #
  # r, w = IO.pipe
  # r.eof?  # blocks forever
  # ```
  #
  # Note that `IO#eof?` reads data to the input byte buffer. So `IO#sysread` may
  # not behave as you intend with `IO#eof?`, unless you call `IO#rewind` first
  # (which is not available for some streams).
  sig {returns(T::Boolean)}
  def eof?(); end

  # This is a deprecated alias for `each_line`.
  sig do
    params(
        sep: String,
        limit: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Enumerator[String])
  end
  def lines(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  # Alias for:
  # [`fileno`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-fileno)
  sig {returns(Integer)}
  def to_i(); end
end

# exception to wait for reading by EAGAIN. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-select).
class IO::EAGAINWaitReadable < Errno::EAGAIN
  include IO::WaitReadable
  Errno = T.let(nil, Integer)
end

# exception to wait for writing by EAGAIN. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-select).
class IO::EAGAINWaitWritable < Errno::EAGAIN
  include IO::WaitWritable
  Errno = T.let(nil, Integer)
end

# exception to wait for reading by EINPROGRESS. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-select).
class IO::EINPROGRESSWaitReadable < Errno::EINPROGRESS
  include IO::WaitReadable
  Errno = T.let(nil, Integer)
end

# exception to wait for writing by EINPROGRESS. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-select).
class IO::EINPROGRESSWaitWritable < Errno::EINPROGRESS
  include IO::WaitWritable
  Errno = T.let(nil, Integer)
end

# exception to wait for reading. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-select).
module IO::WaitReadable
end

# exception to wait for writing. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-select).
module IO::WaitWritable
end
