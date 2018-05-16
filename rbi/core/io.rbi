# typed: true
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
  SEEK_CUR = T.let(T.unsafe(nil), Integer)
  SEEK_DATA = T.let(T.unsafe(nil), Integer)
  SEEK_END = T.let(T.unsafe(nil), Integer)
  SEEK_HOLE = T.let(T.unsafe(nil), Integer)
  SEEK_SET = T.let(T.unsafe(nil), Integer)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)

  sig(
      arg0: BasicObject,
  )
  .returns(IO)
  def <<(arg0); end

  sig(
      arg0: Symbol,
      offset: Integer,
      len: Integer,
  )
  .returns(NilClass)
  def advise(arg0, offset=_, len=_); end

  sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def autoclose=(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def autoclose?(); end

  sig.returns(IO)
  def binmode(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def binmode?(); end

  sig.returns(NilClass)
  def close(); end

  sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def close_on_exec=(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def close_on_exec?(); end

  sig.returns(NilClass)
  def close_read(); end

  sig.returns(NilClass)
  def close_write(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def closed?(); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each(sep=_, limit=_, &blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[Integer])
  def each_byte(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[String])
  def each_char(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[Integer])
  def each_codepoint(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def eof(); end

  sig(
      integer_cmd: Integer,
      arg: T.any(String, Integer),
  )
  .returns(Integer)
  def fcntl(integer_cmd, arg); end

  sig.returns(T.nilable(Integer))
  def fdatasync(); end

  sig.returns(Integer)
  def fileno(); end

  sig.returns(IO)
  def flush(); end

  sig.returns(T.nilable(Integer))
  def fsync(); end

  sig.returns(T.nilable(Integer))
  def getbyte(); end

  sig.returns(T.nilable(String))
  def getc(); end

  sig(
      sep: String,
      limit: Integer,
  )
  .returns(T.nilable(String))
  def gets(sep=_, limit=_); end

  sig(
      fd: Integer,
      mode: Integer,
      opt: Integer,
  )
  .returns(Object)
  def initialize(fd, mode=_, opt=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Encoding)
  def internal_encoding(); end

  sig(
      integer_cmd: Integer,
      arg: T.any(String, Integer),
  )
  .returns(Integer)
  def ioctl(integer_cmd, arg); end

  sig.returns(T.any(TrueClass, FalseClass))
  def isatty(); end

  sig.returns(Integer)
  def lineno(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def lineno=(arg0); end

  sig.returns(Integer)
  def pid(); end

  sig.returns(Integer)
  def pos(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def pos=(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def print(*arg0); end

  sig(
      format_string: String,
      arg0: BasicObject,
  )
  .returns(NilClass)
  def printf(format_string, *arg0); end

  sig(
      arg0: T.any(Numeric, String),
  )
  .returns(T.untyped)
  def putc(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def puts(*arg0); end

  sig(
      length: Integer,
      outbuf: String,
  )
  .returns(T.nilable(String))
  def read(length=_, outbuf=_); end

  sig(
      len: Integer,
  )
  .returns(String)
  sig(
      len: Integer,
      buf: String,
  )
  .returns(String)
  def read_nonblock(len, buf=_); end

  sig.returns(Integer)
  def readbyte(); end

  sig.returns(String)
  def readchar(); end

  sig(
      sep: String,
      limit: Integer,
  )
  .returns(String)
  def readline(sep=_, limit=_); end

  sig(
      sep: String,
      limit: Integer,
  )
  .returns(T::Array[String])
  def readlines(sep=_, limit=_); end

  sig(
      maxlen: Integer,
  )
  .returns(String)
  sig(
      maxlen: Integer,
      outbuf: String,
  )
  .returns(String)
  def readpartial(maxlen, outbuf=_); end

  sig(
      other_IO_or_path: IO,
  )
  .returns(IO)
  sig(
      other_IO_or_path: String,
      mode_str: String,
  )
  .returns(IO)
  def reopen(other_IO_or_path, mode_str=_); end

  sig.returns(Integer)
  def rewind(); end

  sig(
      amount: Integer,
      whence: Integer,
  )
  .returns(Integer)
  def seek(amount, whence=_); end

  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
  )
  .returns(IO)
  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
      int_enc: T.any(String, Encoding),
  )
  .returns(IO)
  def set_encoding(ext_or_ext_int_enc=_, int_enc=_); end

  sig.returns(File::Stat)
  def stat(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def sync(); end

  sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def sync=(arg0); end

  sig(
      maxlen: Integer,
      outbuf: String,
  )
  .returns(String)
  def sysread(maxlen, outbuf); end

  sig(
      amount: Integer,
      whence: Integer,
  )
  .returns(Integer)
  def sysseek(amount, whence=_); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def syswrite(arg0); end

  sig.returns(Integer)
  def tell(); end

  sig.returns(IO)
  def to_io(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def tty?(); end

  sig(
      arg0: T.any(String, Integer),
  )
  .returns(NilClass)
  def ungetbyte(arg0); end

  sig(
      arg0: String,
  )
  .returns(NilClass)
  def ungetc(arg0); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def write(arg0); end

  sig(
      name: String,
      length: Integer,
      offset: Integer,
  )
  .returns(String)
  def self.binread(name, length=_, offset=_); end

  sig(
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
  def self.binwrite(name, arg0, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  sig(
      src: T.any(String, IO),
      dst: T.any(String, IO),
      copy_length: Integer,
      src_offset: Integer,
  )
  .returns(Integer)
  def self.copy_stream(src, dst, copy_length=_, src_offset=_); end

  sig(
      name: String,
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
  def self.read(name, length=_, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  sig(
      name: String,
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
  def self.readlines(name, sep=_, limit=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  sig(
      read_array: T::Array[IO],
      write_array: T::Array[IO],
      error_array: T::Array[IO],
      timeout: Integer,
  )
  .returns(T.nilable(T::Array[IO]))
  def self.select(read_array, write_array=_, error_array=_, timeout=_); end

  sig(
      path: String,
      mode: String,
      perm: String,
  )
  .returns(Integer)
  def self.sysopen(path, mode=_, perm=_); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.nilable(IO))
  def self.try_convert(arg0); end

  sig(
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
  def self.write(name, arg0, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  sig(
      fd: Integer,
      mode: Integer,
      opt: Integer,
  )
  .returns(IO)
  def self.for_fd(fd, mode=_, opt=_); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[Integer])
  def bytes(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[String])
  def chars(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[Integer])
  def codepoints(&blk); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each_line(sep=_, limit=_, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def eof?(); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def lines(sep=_, limit=_, &blk); end

  sig.returns(Integer)
  def to_i(); end
end

class IO::EAGAINWaitReadable < Errno::EAGAIN
  include IO::WaitReadable
end

class IO::EAGAINWaitWritable < Errno::EAGAIN
  include IO::WaitWritable
end

class IO::EINPROGRESSWaitReadable < Errno::EINPROGRESS
  include IO::WaitReadable
end

class IO::EINPROGRESSWaitWritable < Errno::EINPROGRESS
  include IO::WaitWritable
end
