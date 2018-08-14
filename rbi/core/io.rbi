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

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.self_type)
  def <<(arg0); end

  Sorbet.sig(
      arg0: Symbol,
      offset: Integer,
      len: Integer,
  )
  .returns(NilClass)
  def advise(arg0, offset=_, len=_); end

  Sorbet.sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def autoclose=(arg0); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def autoclose?(); end

  Sorbet.sig.returns(T.self_type)
  def binmode(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def binmode?(); end

  Sorbet.sig.returns(NilClass)
  def close(); end

  Sorbet.sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def close_on_exec=(arg0); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def close_on_exec?(); end

  Sorbet.sig.returns(NilClass)
  def close_read(); end

  Sorbet.sig.returns(NilClass)
  def close_write(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def closed?(); end

  Sorbet.sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each(sep=_, limit=_, &blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[Integer])
  def each_byte(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[String])
  def each_char(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[Integer])
  def each_codepoint(&blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def eof(); end

  Sorbet.sig(
      integer_cmd: Integer,
      arg: T.any(String, Integer),
  )
  .returns(Integer)
  def fcntl(integer_cmd, arg); end

  Sorbet.sig.returns(T.nilable(Integer))
  def fdatasync(); end

  Sorbet.sig.returns(Integer)
  def fileno(); end

  Sorbet.sig.returns(T.self_type)
  def flush(); end

  Sorbet.sig.returns(T.nilable(Integer))
  def fsync(); end

  Sorbet.sig.returns(T.nilable(Integer))
  def getbyte(); end

  Sorbet.sig.returns(T.nilable(String))
  def getc(); end

  Sorbet.sig(
      sep: String,
      limit: Integer,
  )
  .returns(T.nilable(String))
  def gets(sep=_, limit=_); end

  Sorbet.sig(
      fd: Integer,
      mode: Integer,
      opt: Integer,
  )
  .void
  def initialize(fd, mode=_, opt=_); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(Encoding)
  def internal_encoding(); end

  Sorbet.sig(
      integer_cmd: Integer,
      arg: T.any(String, Integer),
  )
  .returns(Integer)
  def ioctl(integer_cmd, arg); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def isatty(); end

  Sorbet.sig.returns(Integer)
  def lineno(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def lineno=(arg0); end

  Sorbet.sig.returns(Integer)
  def pid(); end

  Sorbet.sig.returns(Integer)
  def pos(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def pos=(arg0); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def print(*arg0); end

  Sorbet.sig(
      format_string: String,
      arg0: BasicObject,
  )
  .returns(NilClass)
  def printf(format_string, *arg0); end

  Sorbet.sig(
      arg0: T.any(Numeric, String),
  )
  .returns(T.untyped)
  def putc(arg0); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def puts(*arg0); end

  Sorbet.sig(
      length: Integer,
      outbuf: String,
  )
  .returns(T.nilable(String))
  def read(length=_, outbuf=_); end

  Sorbet.sig(
      len: Integer,
  )
  .returns(String)
  Sorbet.sig(
      len: Integer,
      buf: String,
  )
  .returns(String)
  def read_nonblock(len, buf=_); end

  Sorbet.sig.returns(Integer)
  def readbyte(); end

  Sorbet.sig.returns(String)
  def readchar(); end

  Sorbet.sig(
      sep: String,
      limit: Integer,
  )
  .returns(String)
  def readline(sep=_, limit=_); end

  Sorbet.sig(
      sep: String,
      limit: Integer,
  )
  .returns(T::Array[String])
  def readlines(sep=_, limit=_); end

  Sorbet.sig(
      maxlen: Integer,
  )
  .returns(String)
  Sorbet.sig(
      maxlen: Integer,
      outbuf: String,
  )
  .returns(String)
  def readpartial(maxlen, outbuf=_); end

  Sorbet.sig(
      other_IO_or_path: IO,
  )
  .returns(IO)
  Sorbet.sig(
      other_IO_or_path: String,
      mode_str: String,
  )
  .returns(IO)
  def reopen(other_IO_or_path, mode_str=_); end

  Sorbet.sig.returns(Integer)
  def rewind(); end

  Sorbet.sig(
      amount: Integer,
      whence: Integer,
  )
  .returns(Integer)
  def seek(amount, whence=_); end

  Sorbet.sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
  )
  .returns(T.self_type)
  Sorbet.sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
      int_enc: T.any(String, Encoding),
  )
  .returns(T.self_type)
  def set_encoding(ext_or_ext_int_enc=_, int_enc=_); end

  Sorbet.sig.returns(File::Stat)
  def stat(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def sync(); end

  Sorbet.sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def sync=(arg0); end

  Sorbet.sig(
      maxlen: Integer,
      outbuf: String,
  )
  .returns(String)
  def sysread(maxlen, outbuf); end

  Sorbet.sig(
      amount: Integer,
      whence: Integer,
  )
  .returns(Integer)
  def sysseek(amount, whence=_); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(Integer)
  def syswrite(arg0); end

  Sorbet.sig.returns(Integer)
  def tell(); end

  Sorbet.sig.returns(T.self_type)
  def to_io(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def tty?(); end

  Sorbet.sig(
      arg0: T.any(String, Integer),
  )
  .returns(NilClass)
  def ungetbyte(arg0); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(NilClass)
  def ungetc(arg0); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(Integer)
  def write(arg0); end

  Sorbet.sig(
      name: String,
      length: Integer,
      offset: Integer,
  )
  .returns(String)
  def self.binread(name, length=_, offset=_); end

  Sorbet.sig(
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

  Sorbet.sig(
      src: T.any(String, IO),
      dst: T.any(String, IO),
      copy_length: Integer,
      src_offset: Integer,
  )
  .returns(Integer)
  def self.copy_stream(src, dst, copy_length=_, src_offset=_); end

  Sorbet.sig(
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

  Sorbet.sig(
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

  Sorbet.sig(
      read_array: T::Array[IO],
      write_array: T::Array[IO],
      error_array: T::Array[IO],
      timeout: Integer,
  )
  .returns(T.nilable(T::Array[IO]))
  def self.select(read_array, write_array=_, error_array=_, timeout=_); end

  Sorbet.sig(
      path: String,
      mode: String,
      perm: String,
  )
  .returns(Integer)
  def self.sysopen(path, mode=_, perm=_); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.nilable(IO))
  def self.try_convert(arg0); end

  Sorbet.sig(
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

  Sorbet.sig(
      fd: Integer,
      mode: Integer,
      opt: Integer,
  )
  .returns(T.self_type)
  def self.for_fd(fd, mode=_, opt=_); end

  Sorbet.sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[Integer])
  def bytes(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[String])
  def chars(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[Integer])
  def codepoints(&blk); end

  Sorbet.sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each_line(sep=_, limit=_, &blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def eof?(); end

  Sorbet.sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def lines(sep=_, limit=_, &blk); end

  Sorbet.sig.returns(Integer)
  def to_i(); end
end
