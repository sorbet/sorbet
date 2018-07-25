class StringIO
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: String)

  sig(
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

  sig(
      arg0: BasicObject,
  )
  .returns(T.self_type)
  def <<(arg0); end

  sig.returns(T.self_type)
  def binmode(); end

  sig.returns(NilClass)
  def close(); end

  sig.returns(NilClass)
  def close_read(); end

  sig.returns(NilClass)
  def close_write(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def closed?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def closed_read?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def closed_write?(); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each(sep=_, limit=_, &blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.self_type)
  sig.returns(Enumerator[Integer])
  def each_byte(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  sig.returns(Enumerator[String])
  def each_char(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.self_type)
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

  sig.returns(NilClass)
  def fileno(); end

  sig.returns(T.self_type)
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

  sig.returns(Encoding)
  def internal_encoding(); end

  sig.returns(Encoding)
  def external_encoding(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def isatty(); end

  sig.returns(Integer)
  def lineno(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def lineno=(arg0); end

  sig.returns(NilClass)
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
      other: StringIO,
  )
  .returns(T.self_type)
  sig(
      other: String,
      mode_str: String,
  )
  .returns(T.self_type)
  def reopen(other, mode_str=_); end

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
  .returns(T.self_type)
  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
      int_enc: T.any(String, Encoding),
  )
  .returns(T.self_type)
  def set_encoding(ext_or_ext_int_enc=_, int_enc=_); end

  sig.returns(String)
  def string(); end

  sig(str: String).returns(String)
  def string=(str); end

  sig.returns(Integer)
  def size(); end

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
      arg0: String,
  )
  .returns(Integer)
  def syswrite(arg0); end

  sig.returns(Integer)
  def tell(); end

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
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.self_type)
  sig.returns(Enumerator[Integer])
  def bytes(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  sig.returns(Enumerator[String])
  def chars(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.self_type)
  sig.returns(Enumerator[Integer])
  def codepoints(&blk); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
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
  .returns(T.self_type)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def lines(sep=_, limit=_, &blk); end
end
