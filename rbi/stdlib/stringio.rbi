# typed: strict
class StringIO
  include Enumerable
  extend T::Generic

  sig do
    params(
      string: String,
      mode: T.nilable(String)
    ).void
  end
  def initialize(string="", mode="rw"); end

  sig do
    type_parameters(:U)
    .params(
      string: String,
      mode: T.nilable(String),
      blk: T.proc.params(arg: StringIO).returns(T.type_parameter(:U)),
    ).returns(T.type_parameter(:U))
  end
  def self.open(string="", mode="rw", &blk); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.self_type)
  end
  def <<(arg0); end

  sig {returns(T.self_type)}
  def binmode(); end

  sig {returns(NilClass)}
  def close(); end

  sig {returns(NilClass)}
  def close_read(); end

  sig {returns(NilClass)}
  def close_write(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def closed?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def closed_read?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def closed_write?(); end

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

  sig {returns(T.any(TrueClass, FalseClass))}
  def eof(); end

  sig do
    params(
        integer_cmd: Integer,
        arg: T.any(String, Integer),
    )
    .returns(Integer)
  end
  def fcntl(integer_cmd, arg); end

  sig {returns(NilClass)}
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

  sig {returns(Encoding)}
  def internal_encoding(); end

  sig {returns(Encoding)}
  def external_encoding(); end

  sig {returns(T.any(TrueClass, FalseClass))}
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

  sig {returns(NilClass)}
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
        other: StringIO,
    )
    .returns(T.self_type)
  end
  sig do
    params(
        other: String,
        mode_str: String,
    )
    .returns(T.self_type)
  end
  def reopen(other, mode_str=T.unsafe(nil)); end

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

  sig {returns(String)}
  def string(); end

  sig {params(str: String).returns(String)}
  def string=(str); end

  sig {returns(Integer)}
  def size(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def sync(); end

  sig do
    params(
        arg0: T.any(TrueClass, FalseClass),
    )
    .returns(T.any(TrueClass, FalseClass))
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
        arg0: String,
    )
    .returns(Integer)
  end
  def syswrite(arg0); end

  sig {returns(Integer)}
  def tell(); end

  sig {returns(T.any(TrueClass, FalseClass))}
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

  sig {returns(T.any(TrueClass, FalseClass))}
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
end
