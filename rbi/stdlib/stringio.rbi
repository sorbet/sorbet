# typed: strict
class StringIO
  include Enumerable
  extend T::Generic

  Sorbet.sig(
    string: String,
    mode: T.nilable(String)
  ).void
  def initialize(string="", mode="rw"); end

  type_variables(:U).sig(
    string: String,
    mode: T.nilable(String),
    blk: T.proc(arg: StringIO).returns(T.type_variable(:U)),
  ).returns(T.type_variable(:U))
  def self.open(string="", mode="rw", &blk); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.self_type)
  def <<(arg0); end

  Sorbet.sig.returns(T.self_type)
  def binmode(); end

  Sorbet.sig.returns(NilClass)
  def close(); end

  Sorbet.sig.returns(NilClass)
  def close_read(); end

  Sorbet.sig.returns(NilClass)
  def close_write(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def closed?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def closed_read?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def closed_write?(); end

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

  Sorbet.sig.returns(NilClass)
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

  Sorbet.sig.returns(Encoding)
  def internal_encoding(); end

  Sorbet.sig.returns(Encoding)
  def external_encoding(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def isatty(); end

  Sorbet.sig.returns(Integer)
  def lineno(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def lineno=(arg0); end

  Sorbet.sig.returns(NilClass)
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
      other: StringIO,
  )
  .returns(T.self_type)
  Sorbet.sig(
      other: String,
      mode_str: String,
  )
  .returns(T.self_type)
  def reopen(other, mode_str=_); end

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

  Sorbet.sig.returns(String)
  def string(); end

  Sorbet.sig(str: String).returns(String)
  def string=(str); end

  Sorbet.sig.returns(Integer)
  def size(); end

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
      arg0: String,
  )
  .returns(Integer)
  def syswrite(arg0); end

  Sorbet.sig.returns(Integer)
  def tell(); end

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
end
