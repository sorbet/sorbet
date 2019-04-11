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

  sig {returns(NilClass)}
  def close(); end

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
    .returns(Enumerator[String])
  end
  def each(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[Integer])}
  def each_byte(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[String])}
  def each_char(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[Integer])}
  def each_codepoint(&blk); end

  sig {returns(T::Boolean)}
  def eof(); end

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

  sig do
    params(
        fd: Integer,
        mode: Integer,
        opt: Integer,
    )
    .void
  end
  def initialize(fd, mode=T.unsafe(nil), opt=T.unsafe(nil)); end

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

  sig {returns(Integer)}
  def pid(); end

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
  def sysread(maxlen, outbuf); end

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
        arg0: String,
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

  # https://ruby-doc.org/core-2.3.0/IO.html#method-c-popen
  # This signature is very hard to type. I'm giving up and making it untyped.
  # As far as I can tell, at least one arg is required, and it must be an array,
  # but sometimes it's the first arg and sometimes it's the second arg, so
  # let's just make everything untyped.
  #
  # TODO(jez) Have to declare this as a rest arg, because pay-server runtime
  # reflection sees it this way. Once it's out of the missing method file, we
  # can add a better sig here.
  sig do
    params(
        args: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.popen(*args); end

  sig do
    params(
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
  end
  def self.read(name, length=T.unsafe(nil), offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  sig do
    params(
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
  end
  def self.readlines(name, sep=T.unsafe(nil), limit=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  sig do
    params(
        read_array: T::Array[IO],
        write_array: T::Array[IO],
        error_array: T::Array[IO],
        timeout: Integer,
    )
    .returns(T.nilable(T::Array[IO]))
  end
  def self.select(read_array, write_array=T.unsafe(nil), error_array=T.unsafe(nil), timeout=T.unsafe(nil)); end

  sig do
    params(
        path: String,
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
  def self.write(name, arg0, offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

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
  sig {returns(Enumerator[Integer])}
  def bytes(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[String])}
  def chars(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[Integer])}
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
    .returns(Enumerator[String])
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
    .returns(Enumerator[String])
  end
  def lines(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  sig {returns(Integer)}
  def to_i(); end
end
