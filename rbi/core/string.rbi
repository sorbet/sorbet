# typed: core
class String < Object
  include Comparable

  sig do
    params(
        arg0: Object,
    )
    .returns(String)
  end
  def %(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(String)
  end
  def *(arg0); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def +(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(String)
  end
  def <<(arg0); end

  sig do
    params(
        other: String,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T.nilable(Integer))
  end
  def =~(arg0); end

  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(T::Range[Integer], Regexp),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def [](arg0, arg1=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def ascii_only?(); end

  sig {returns(String)}
  def b(); end

  sig {returns(Array)}
  def bytes(); end

  sig {returns(Integer)}
  def bytesize(); end

  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T.nilable(String))
  end
  def byteslice(arg0, arg1=T.unsafe(nil)); end

  sig {returns(String)}
  def capitalize(); end

  sig {returns(T.nilable(String))}
  def capitalize!(); end

  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(Integer))
  end
  def casecmp(arg0); end

  sig do
    params(
        arg0: Integer,
        arg1: String,
    )
    .returns(String)
  end
  def center(arg0, arg1=T.unsafe(nil)); end

  sig {returns(Array)}
  def chars(); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def chomp(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def chomp!(arg0=T.unsafe(nil)); end

  sig {returns(String)}
  def chop(); end

  sig {returns(T.nilable(String))}
  def chop!(); end

  sig {returns(String)}
  def chr(); end

  sig {returns(String)}
  def clear(); end

  sig {returns(T::Array[Integer])}
  sig do
    params(
        blk: BasicObject,
    )
    .returns(T::Array[Integer])
  end
  def codepoints(&blk); end

  sig do
    params(
        arg0: T.any(Integer, Object),
    )
    .returns(String)
  end
  def concat(arg0); end

  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(Integer)
  end
  def count(arg0, *arg1); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def crypt(arg0); end

  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(String)
  end
  def delete(arg0, *arg1); end

  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  def delete!(arg0, *arg1); end

  sig {returns(String)}
  def downcase(); end

  sig {returns(T.nilable(String))}
  def downcase!(); end

  sig {returns(String)}
  def dump(); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(String)
  end
  sig {returns(Enumerator[Integer])}
  def each_byte(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  sig {returns(Enumerator[String])}
  def each_char(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(String)
  end
  sig {returns(Enumerator[Integer])}
  def each_codepoint(&blk); end

  sig do
    params(
        arg0: String,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  sig do
    params(
        arg0: String,
    )
    .returns(Enumerator[String])
  end
  def each_line(arg0=T.unsafe(nil), &blk); end

  sig {returns(T::Boolean)}
  def empty?(); end

  sig {returns(Encoding)}
  def encoding(); end

  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def end_with?(*arg0); end

  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  sig do
    params(
        arg0: T.any(String, Encoding),
    )
    .returns(String)
  end
  def force_encoding(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.nilable(Integer))
  end
  def getbyte(arg0); end

  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Hash,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(Enumerator[String])
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(String)
  end
  def gsub(arg0, arg1=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(Enumerator[String])
  end
  def gsub!(arg0, arg1=T.unsafe(nil), &blk); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def hex(); end

  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end

  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Integer,
    )
    .returns(T.nilable(Integer))
  end
  def index(arg0, arg1=T.unsafe(nil)); end

  sig do
    params(
        str: String,
    )
    .void
  end
  def initialize(str=T.unsafe(nil)); end

  sig do
    params(
        arg0: Integer,
        arg1: String,
    )
    .returns(String)
  end
  def insert(arg0, arg1); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(Symbol)}
  def intern(); end

  sig {returns(Integer)}
  def length(); end

  sig do
    params(
        arg0: String,
    )
    .returns(T::Array[String])
  end
  def lines(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: Integer,
        arg1: String,
    )
    .returns(String)
  end
  def ljust(arg0, arg1=T.unsafe(nil)); end

  sig {returns(String)}
  def lstrip(); end

  sig {returns(T.nilable(String))}
  def lstrip!(); end

  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(T.nilable(MatchData))
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Integer,
    )
    .returns(T.nilable(MatchData))
  end
  def match(arg0, arg1=T.unsafe(nil)); end

  sig {returns(String)}
  def next(); end

  sig {returns(String)}
  def next!(); end

  sig {returns(Integer)}
  def oct(); end

  sig {returns(Integer)}
  def ord(); end

  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns([String, String, String])
  end
  def partition(arg0); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def prepend(arg0); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def replace(arg0); end

  sig {returns(String)}
  def reverse(); end

  sig do
    params(
        arg0: T.any(String, Regexp),
        arg1: Integer,
    )
    .returns(T.nilable(Integer))
  end
  def rindex(arg0, arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: Integer,
        arg1: String,
    )
    .returns(String)
  end
  def rjust(arg0, arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(String, Regexp),
    )
    .returns([String, String, String])
  end
  def rpartition(arg0); end

  sig {returns(String)}
  def rstrip(); end

  sig {returns(String)}
  def rstrip!(); end

  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(T::Array[T.any(String, T::Array[String])])
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: BasicObject,
    )
    .returns(T::Array[T.any(String, T::Array[String])])
  end
  def scan(arg0, &blk); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: String,
        blk: T.proc.params(arg0: T.untyped).returns(BasicObject),
    )
    .returns(String)
  end
  def scrub(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: String,
        blk: T.proc.params(arg0: T.untyped).returns(BasicObject),
    )
    .returns(String)
  end
  def scrub!(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(Integer)
  end
  def setbyte(arg0, arg1); end

  sig {returns(Integer)}
  def size(); end

  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(T::Range[Integer], Regexp),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def slice!(arg0, arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Integer,
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[String])
  end
  def split(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def squeeze(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def squeeze!(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def start_with?(*arg0); end

  sig {returns(String)}
  def strip(); end

  sig {returns(String)}
  def strip!(); end

  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: T.any(String, Hash),
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  def sub(arg0, arg1=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  def sub!(arg0, arg1=T.unsafe(nil), &blk); end

  sig {returns(String)}
  def succ(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def sum(arg0=T.unsafe(nil)); end

  sig {returns(String)}
  def swapcase(); end

  sig {returns(T.nilable(String))}
  def swapcase!(); end

  sig {returns(Complex)}
  def to_c(); end

  sig {returns(Float)}
  def to_f(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def to_i(arg0=T.unsafe(nil)); end

  sig {returns(Rational)}
  def to_r(); end

  sig {returns(String)}
  def to_s(); end

  sig {returns(String)}
  def to_str(); end

  sig {returns(Symbol)}
  def to_sym(); end

  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(String)
  end
  def tr(arg0, arg1); end

  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  def tr!(arg0, arg1); end

  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(String)
  end
  def tr_s(arg0, arg1); end

  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  def tr_s!(arg0, arg1); end

  sig do
    params(
        arg0: String,
    )
    .returns(T::Array[String])
  end
  def unpack(arg0); end

  sig {returns(String)}
  def upcase(); end

  sig {returns(T.nilable(String))}
  def upcase!(); end

  sig do
    type_parameters(:Bool).params(
        arg0: String,
        arg1: T.type_parameter(:Bool),
    )
    .returns(Enumerator[String])
  end
  sig do
    type_parameters(:Bool).params(
        arg0: String,
        arg1: T.type_parameter(:Bool),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  def upto(arg0, arg1=T.unsafe(nil), &blk); end

  sig {returns(T::Boolean)}
  def valid_encoding?(); end

  sig do
    params(
        obj: Object,
    )
    .returns(T.nilable(String))
  end
  def self.try_convert(obj); end

  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(T::Range[Integer], Regexp),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def slice(arg0, arg1=T.unsafe(nil)); end
end
