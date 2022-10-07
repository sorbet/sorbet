# typed: __STDLIB_INTERNAL

# A utility class for managing temporary files. When you create a
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object, it
# will create a temporary file with a unique filename. A
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) objects
# behaves just like a [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
# object, and you can perform all the usual file operations on it: reading data,
# writing data, changing its permissions, etc. So although this class does not
# explicitly document all instance methods supported by
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html), you can in fact call
# any [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) instance method on
# a [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object.
#
# ## Synopsis
#
# ```ruby
# require 'tempfile'
#
# file = Tempfile.new('foo')
# file.path      # => A unique filename in the OS's temp directory,
#                #    e.g.: "/tmp/foo.24722.0"
#                #    This filename contains 'foo' in its basename.
# file.write("hello world")
# file.rewind
# file.read      # => "hello world"
# file.close
# file.unlink    # deletes the temp file
# ```
#
# ## Good practices
#
# ### Explicit close
#
# When a [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object
# is garbage collected, or when the Ruby interpreter exits, its associated
# temporary file is automatically deleted. This means that's it's unnecessary to
# explicitly delete a
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) after use,
# though it's good practice to do so: not explicitly deleting unused Tempfiles
# can potentially leave behind large amounts of tempfiles on the filesystem
# until they're garbage collected. The existence of these temp files can make it
# harder to determine a new
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) filename.
#
# Therefore, one should always call
# [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
# or close in an ensure block, like this:
#
# ```ruby
# file = Tempfile.new('foo')
# begin
#    # ...do something with file...
# ensure
#    file.close
#    file.unlink   # deletes the temp file
# end
# ```
#
# ### Unlink after creation
#
# On POSIX systems, it's possible to unlink a file right after creating it, and
# before closing it. This removes the filesystem entry without closing the file
# handle, so it ensures that only the processes that already had the file handle
# open can access the file's contents. It's strongly recommended that you do
# this if you do not want any other processes to be able to read from or write
# to the [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html), and
# you do not need to know the Tempfile's filename either.
#
# For example, a practical use case for unlink-after-creation would be this: you
# need a large byte buffer that's too large to comfortably fit in RAM, e.g. when
# you're writing a web server and you want to buffer the client's file upload
# data.
#
# Please refer to
# [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
# for more information and a code example.
#
# ## Minor notes
#
# Tempfile's filename picking method is both thread-safe and inter-process-safe:
# it guarantees that no other threads or processes will pick the same filename.
#
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) itself however
# may not be entirely thread-safe. If you access the same
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object from
# multiple threads then you should protect it with a mutex.
class Tempfile
  extend T::Sig

  # Creates a temporary file as usual
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) object (not
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html)). It doesn't
  # use finalizer and delegation.
  #
  # If no block is given, this is similar to
  # [`Tempfile.new`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-c-new)
  # except creating [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
  # instead of [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html).
  # The created file is not removed automatically. You should use
  # [`File.unlink`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-unlink)
  # to remove it.
  #
  # If a block is given, then a
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) object will be
  # constructed, and the block is invoked with the object as the argument. The
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) object will be
  # automatically closed and the temporary file is removed after the block
  # terminates. The call returns the value of the block.
  #
  # In any case, all arguments (`basename`, `tmpdir`, `mode`, and `**options`)
  # will be treated as
  # [`Tempfile.new`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-c-new).
  #
  # ```ruby
  # Tempfile.create('foo', '/home/temp') do |f|
  #    # ... do something with f ...
  # end
  # ```
  sig do
    params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
    )
    .returns(File)
  end
  sig do
    type_parameters(:T)
    .params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
      blk: T.proc.params(arg0: File).returns(T.type_parameter(:T)),
    )
    .returns(T.type_parameter(:T))
  end
  def self.create(basename="", tmpdir=nil, mode: 0, **options, &blk); end

  # Creates a new
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html).
  #
  # If no block is given, this is a synonym for
  # [`Tempfile.new`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-c-new).
  #
  # If a block is given, then a
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object will
  # be constructed, and the block is run with said object as argument. The
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object will
  # be automatically closed after the block terminates. The call returns the
  # value of the block.
  #
  # In any case, all arguments (`*args`) will be passed to
  # [`Tempfile.new`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-c-new).
  #
  # ```ruby
  # Tempfile.open('foo', '/home/temp') do |f|
  #    # ... do something with f ...
  # end
  #
  # # Equivalent:
  # f = Tempfile.open('foo', '/home/temp')
  # begin
  #    # ... do something with f ...
  # ensure
  #    f.close
  # end
  # ```
  sig do
    params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
    )
    .returns(Tempfile)
  end
  sig do
    type_parameters(:T)
    .params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
      blk: T.nilable(T.proc.params(arg0: Tempfile).returns(T.type_parameter(:T))),
    )
    .returns(T.type_parameter(:T))
  end
  def self.open(basename='', tmpdir=nil, mode: 0, **options, &blk); end

  sig do
    params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
    )
    .void
  end
  def initialize(basename='', tmpdir=nil, mode: 0, **options); end

  # Closes the file. If `unlink_now` is true, then the file will be unlinked
  # (deleted) after closing. Of course, you can choose to later call
  # [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
  # if you do not unlink it now.
  #
  # If you don't explicitly unlink the temporary file, the removal will be
  # delayed until the object is finalized.
  sig {params(unlink_now: T::Boolean).void}
  def close(unlink_now=false); end

  # Closes and unlinks (deletes) the file. Has the same effect as called
  # `close(true)`.
  sig {void}
  def close!; end

  # Alias for:
  # [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
  sig {returns(T::Boolean)}
  def delete; end

  # Alias for:
  # [`size`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-size)
  sig {returns(Integer)}
  def length; end

  # Opens or reopens the file with mode "r+".
  sig {returns(Tempfile)}
  def open; end

  ### path returns nil if the Tempfile has been unlinked.
  # Returns the full path name of the temporary file. This will be nil if
  # [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
  # has been called.
  sig {returns(T.nilable(String))}
  def path; end

  # Returns the size of the temporary file. As a side effect, the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) buffer is flushed before
  # determining the size.
  #
  # Also aliased as:
  # [`length`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-length)
  sig {returns(Integer)}
  def size; end

  # Unlinks (deletes) the file from the filesystem. One should always unlink the
  # file after using it, as is explained in the "Explicit close" good practice
  # section in the
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) overview:
  #
  # ```ruby
  # file = Tempfile.new('foo')
  # begin
  #    # ...do something with file...
  # ensure
  #    file.close
  #    file.unlink   # deletes the temp file
  # end
  # ```
  #
  # ### Unlink-before-close
  #
  # On POSIX systems it's possible to unlink a file before closing it. This
  # practice is explained in detail in the
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) overview
  # (section "Unlink after creation"); please refer there for more information.
  #
  # However, unlink-before-close may not be supported on non-POSIX operating
  # systems. Microsoft Windows is the most notable case: unlinking a non-closed
  # file will result in an error, which this method will silently ignore. If you
  # want to practice unlink-before-close whenever possible, then you should
  # write code like this:
  #
  # ```ruby
  # file = Tempfile.new('foo')
  # file.unlink   # On Windows this silently fails.
  # begin
  #    # ... do something with file ...
  # ensure
  #    file.close!   # Closes the file handle. If the file wasn't unlinked
  #                  # because #unlink failed, then this method will attempt
  #                  # to do so again.
  # end
  # ```
  #
  #
  # Also aliased as:
  # [`delete`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-delete)
  sig {returns(T::Boolean)}
  def unlink; end

  # ---- ::IO methods ----

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.self_type)
  end
  def <<(arg0); end

  sig do
    params(
        arg0: Symbol,
        offset: Integer,
        len: Integer,
    )
    .returns(NilClass)
  end
  def advise(arg0, offset=T.unsafe(nil), len=T.unsafe(nil)); end

  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def autoclose=(arg0); end

  sig {returns(T::Boolean)}
  def autoclose?(); end

  sig {returns(T.self_type)}
  def binmode(); end

  sig {returns(T::Boolean)}
  def binmode?(); end

  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def close_on_exec=(arg0); end

  sig {returns(T::Boolean)}
  def close_on_exec?(); end

  sig {returns(NilClass)}
  def close_read(); end

  sig {returns(NilClass)}
  def close_write(); end

  sig {returns(T::Boolean)}
  def closed?(); end

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

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_byte(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[String])}
  def each_char(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_codepoint(&blk); end

  sig {returns(T::Boolean)}
  def eof(); end

  def external_encoding; end

  sig do
    params(
        integer_cmd: Integer,
        arg: T.any(String, Integer),
    )
    .returns(Integer)
  end
  def fcntl(integer_cmd, arg); end

  sig {returns(T.nilable(Integer))}
  def fdatasync(); end

  sig {returns(Integer)}
  def fileno(); end

  sig {returns(T.self_type)}
  def flush(); end

  sig {returns(T.nilable(Integer))}
  def fsync(); end

  sig {returns(T.nilable(Integer))}
  def getbyte(); end

  sig {returns(T.nilable(String))}
  def getc(); end

  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T.nilable(String))
  end
  def gets(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(Encoding)}
  def internal_encoding(); end

  sig do
    params(
        integer_cmd: Integer,
        arg: T.any(String, Integer),
    )
    .returns(Integer)
  end
  def ioctl(integer_cmd, arg); end

  sig {returns(T::Boolean)}
  def isatty(); end

  sig {returns(Integer)}
  def lineno(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def lineno=(arg0); end

  def pread(*_); end

  def pwrite(_, _); end

  sig {returns(Integer)}
  def pid(); end

  sig do
    params(
      ext_enc: T.nilable(T.any(String, Encoding)),
      int_enc: T.nilable(T.any(String, Encoding)),
      opt: T.nilable(T::Hash[Symbol, String]),
      blk: T.nilable(T.proc.params(read_io: IO, write_io: IO).void)
    ).returns([IO, IO])
  end
  def self.pipe(ext_enc = nil, int_enc = nil, opt = nil, &blk); end

  sig {returns(Integer)}
  def pos(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def pos=(arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def print(*arg0); end

  sig do
    params(
        format_string: String,
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def printf(format_string, *arg0); end

  sig do
    params(
        arg0: T.any(Numeric, String),
    )
    .returns(T.untyped)
  end
  def putc(arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def puts(*arg0); end

  sig do
    params(
        length: Integer,
        outbuf: String,
    )
    .returns(T.nilable(String))
  end
  def read(length=T.unsafe(nil), outbuf=T.unsafe(nil)); end

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

  sig {returns(Integer)}
  def readbyte(); end

  sig {returns(String)}
  def readchar(); end

  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(String)
  end
  def readline(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Array[String])
  end
  def readlines(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

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

  sig do
    params(
        other_IO_or_path: T.any(IO, Tempfile),
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

  sig {returns(Integer)}
  def rewind(); end

  sig do
    params(
        amount: Integer,
        whence: Integer,
    )
    .returns(Integer)
  end
  def seek(amount, whence=T.unsafe(nil)); end

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

  sig {returns(File::Stat)}
  def stat(); end

  sig {returns(T::Boolean)}
  def sync(); end

  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def sync=(arg0); end

  sig do
    params(
        maxlen: Integer,
        outbuf: String,
    )
    .returns(String)
  end
  def sysread(maxlen, outbuf=T.unsafe(nil)); end

  sig do
    params(
        amount: Integer,
        whence: Integer,
    )
    .returns(Integer)
  end
  def sysseek(amount, whence=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def syswrite(arg0); end

  sig {returns(Integer)}
  def tell(); end

  sig {returns(T.self_type)}
  def to_io(); end

  sig {returns(T::Boolean)}
  def tty?(); end

  sig do
    params(
        arg0: T.any(String, Integer),
    )
    .returns(NilClass)
  end
  def ungetbyte(arg0); end

  sig do
    params(
        arg0: String,
    )
    .returns(NilClass)
  end
  def ungetc(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(Integer)
  end
  def write(arg0); end

  sig do
    params(
        name: String,
        length: Integer,
        offset: Integer,
    )
    .returns(String)
  end
  def self.binread(name, length=T.unsafe(nil), offset=T.unsafe(nil)); end

  sig do
    params(
        name: String,
        arg0: String,
        offset: Integer,
        external_encoding: T.any(String, Encoding),
        internal_encoding: T.any(String, Encoding),
        encoding: T.any(String, Encoding),
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(Integer)
  end
  def self.binwrite(name, arg0, offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  sig do
    params(
        src: T.any(String, IO, Tempfile),
        dst: T.any(String, IO, Tempfile),
        copy_length: Integer,
        src_offset: Integer,
    )
    .returns(Integer)
  end
  def self.copy_stream(src, dst, copy_length=T.unsafe(nil), src_offset=T.unsafe(nil)); end

  def self.foreach(*_); end

  sig do
    params(
        args: T.untyped,
        blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.popen(*args, &blk); end

  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        length: Integer,
        offset: Integer,
        external_encoding: T.any(String, Encoding),
        internal_encoding: T.any(String, Encoding),
        encoding: T.any(String, Encoding),
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(String)
  end
  def self.read(name, length=T.unsafe(nil), offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        sep: String,
        limit: Integer,
        external_encoding: T.any(String, Encoding),
        internal_encoding: T.any(String, Encoding),
        encoding: T.any(String, Encoding),
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
        chomp: T::Boolean
    )
    .returns(T::Array[String])
  end
  def self.readlines(name, sep=T.unsafe(nil), limit=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil), chomp: T.unsafe(nil)); end

  sig do
    params(
        read_array: T.nilable(T::Array[IO]),
        write_array: T.nilable(T::Array[IO]),
        error_array: T.nilable(T::Array[IO]),
        timeout: T.any(NilClass, Integer, Float),
    )
    .returns(T.nilable(T::Array[T::Array[IO]]))
  end
  def self.select(read_array, write_array=nil, error_array=nil, timeout=nil); end

  sig do
    params(
        path: T.any(String, Tempfile, File, Pathname),
        mode: String,
        perm: String,
    )
    .returns(Integer)
  end
  def self.sysopen(path, mode=T.unsafe(nil), perm=T.unsafe(nil)); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.nilable(IO))
  end
  def self.try_convert(arg0); end

  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        string: Object,
        offset: Integer,
        external_encoding: T.any(String, Encoding),
        internal_encoding: T.any(String, Encoding),
        encoding: T.any(String, Encoding),
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
        perm: Integer
    )
    .returns(Integer)
  end
  def self.write(name, string, offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil), perm: T.unsafe(nil)); end

  sig do
    params(
        fd: Integer,
        mode: Integer,
        opt: Integer,
    )
    .returns(T.self_type)
  end
  def self.for_fd(fd, mode=T.unsafe(nil), opt=T.unsafe(nil)); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def bytes(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[String])}
  def chars(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def codepoints(&blk); end

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

  sig {returns(T::Boolean)}
  def eof?(); end

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

  sig {returns(Integer)}
  def to_i(); end

  sig { params(string: Object, exception: T::Boolean).returns(Integer) }
  def write_nonblock(string, exception: true); end

  # ---- ::File methods ----

  sig do
    params(
        file: T.any(String, Pathname),
        dir: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.absolute_path(file, dir=T.unsafe(nil)); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.atime(file); end

  sig do
    params(
        file: T.any(String, Pathname),
        suffix: String,
    )
    .returns(String)
  end
  def self.basename(file, suffix=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(String, Pathname),
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(String, Pathname),
        arg1: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(String, Pathname),
        arg1: Integer,
        arg2: Integer,
    )
    .returns(String)
  end
  def self.binread(arg0, arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.birthtime(file); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.blockdev?(file); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.chardev?(file); end

  sig do
    params(
        mode: Integer,
        files: String,
    )
    .returns(Integer)
  end
  def self.chmod(mode, *files); end

  sig do
    params(
        owner: T.nilable(Integer),
        group: T.nilable(Integer),
        files: String,
    )
    .returns(Integer)
  end
  def self.chown(owner, group, *files); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.ctime(file); end

  sig do
    params(
        files: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.delete(*files); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.directory?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.dirname(file); end

  def self.empty?(_); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.executable?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.executable_real?(file); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(T::Boolean)
  end
  def self.exist?(file); end

  sig do
    params(
        file: BasicObject,
        dir: BasicObject,
    )
    .returns(String)
  end
  def self.expand_path(file, dir=T.unsafe(nil)); end

  sig do
    params(
        path: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.extname(path); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.file?(file); end

  sig do
    params(
        pattern: String,
        path: T.any(String, Pathname),
        flags: Integer,
    )
    .returns(T::Boolean)
  end
  def self.fnmatch(pattern, path, flags=T.unsafe(nil)); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.ftype(file); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.grpowned?(file); end

  sig do
    params(
        file_1: T.any(String, Pathname, IO),
        file_2: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.identical?(file_1, file_2); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(String)
  end
  def self.join(*arg0); end

  sig do
    params(
        mode: Integer,
        files: String,
    )
    .returns(Integer)
  end
  def self.lchmod(mode, *files); end

  sig do
    params(
        owner: T.nilable(Integer),
        group: T.nilable(Integer),
        files: String,
    )
    .returns(Integer)
  end
  def self.lchown(owner, group, *files); end

  sig do
    params(
        old: T.any(String, Pathname),
        new: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.link(old, new); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(File::Stat)
  end
  def self.lstat(file); end

  def self.lutime(*_); end

  def self.mkfifo(*_); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.mtime(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.owned?(file); end

  sig do
    params(
        path: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.path(path); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.pipe?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.readable?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.readable_real?(file); end

  sig do
    params(
        link: String,
    )
    .returns(String)
  end
  def self.readlink(link); end

  sig do
    params(
        pathname: T.any(String, Pathname),
        dir: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.realdirpath(pathname, dir=T.unsafe(nil)); end

  sig do
    params(
        pathname: T.any(String, Pathname),
        dir: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.realpath(pathname, dir=T.unsafe(nil)); end

  sig do
    params(
        old: T.any(String, Pathname),
        new: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.rename(old, new); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.setgid?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.setuid?(file); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(Integer)
  end
  def self.size(file); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.size?(file); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.socket?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns([String, String])
  end
  def self.split(file); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(File::Stat)
  end
  def self.stat(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.sticky?(file); end

  sig do
    params(
        old: T.any(String, Pathname),
        new: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.symlink(old, new); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.symlink?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.truncate(file, arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.umask(arg0=T.unsafe(nil)); end

  sig do
    params(
        atime: Time,
        mtime: Time,
        files: String,
    )
    .returns(Integer)
  end
  def self.utime(atime, mtime, *files); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.world_readable?(file); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.world_writable?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T.nilable(Integer))
  end
  def self.writable?(file); end

  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T.nilable(Integer))
  end
  def self.writable_real?(file); end

  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.zero?(file); end

  sig {returns(Time)}
  def atime(); end

  sig {returns(Time)}
  def birthtime(); end

  sig do
    params(
        mode: Integer,
    )
    .returns(Integer)
  end
  def chmod(mode); end

  sig do
    params(
        owner: T.nilable(Integer),
        group: T.nilable(Integer),
    )
    .returns(Integer)
  end
  def chown(owner, group); end

  sig {returns(Time)}
  def ctime(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.any(Integer, TrueClass, FalseClass))
  end
  def flock(arg0); end

  sig {returns(File::Stat)}
  def lstat(); end

  sig {returns(Time)}
  def mtime(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def truncate(arg0); end

  sig do
    params(
        pattern: String,
        path: String,
        flags: Integer,
    )
    .returns(T::Boolean)
  end
  def self.fnmatch?(pattern, path, flags=T.unsafe(nil)); end

  sig do
    params(
        files: String,
    )
    .returns(Integer)
  end
  def self.unlink(*files); end

  sig {returns(String)}
  def to_path(); end

end
