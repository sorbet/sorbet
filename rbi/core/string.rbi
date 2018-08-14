# typed: true
class String < Object
  include Comparable

  Sorbet.sig(
      arg0: Object,
  )
  .returns(String)
  def %(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(String)
  def *(arg0); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def +(arg0); end

  Sorbet.sig(
      arg0: Object,
  )
  .returns(String)
  def <<(arg0); end

  Sorbet.sig(
      other: String,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(arg0); end

  Sorbet.sig(
      arg0: Object,
  )
  .returns(T.nilable(Integer))
  def =~(arg0); end

  Sorbet.sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: T.any(T::Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def [](arg0, arg1=_); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def ascii_only?(); end

  Sorbet.sig.returns(String)
  def b(); end

  Sorbet.sig.returns(Array)
  def bytes(); end

  Sorbet.sig.returns(Integer)
  def bytesize(); end

  Sorbet.sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: T::Range[Integer],
  )
  .returns(T.nilable(String))
  def byteslice(arg0, arg1=_); end

  Sorbet.sig.returns(String)
  def capitalize(); end

  Sorbet.sig.returns(T.nilable(String))
  def capitalize!(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(T.nilable(Integer))
  def casecmp(arg0); end

  Sorbet.sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def center(arg0, arg1=_); end

  Sorbet.sig.returns(Array)
  def chars(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def chomp(arg0=_); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def chomp!(arg0=_); end

  Sorbet.sig.returns(String)
  def chop(); end

  Sorbet.sig.returns(T.nilable(String))
  def chop!(); end

  Sorbet.sig.returns(String)
  def chr(); end

  Sorbet.sig.returns(String)
  def clear(); end

  Sorbet.sig.returns(T::Array[Integer])
  Sorbet.sig(
      blk: BasicObject,
  )
  .returns(T::Array[Integer])
  def codepoints(&blk); end

  Sorbet.sig(
      arg0: T.any(Integer, Object),
  )
  .returns(String)
  def concat(arg0); end

  Sorbet.sig(
      arg0: String,
      arg1: String,
  )
  .returns(Integer)
  def count(arg0, *arg1); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def crypt(arg0); end

  Sorbet.sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def delete(arg0, *arg1); end

  Sorbet.sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def delete!(arg0, *arg1); end

  Sorbet.sig.returns(String)
  def downcase(); end

  Sorbet.sig.returns(T.nilable(String))
  def downcase!(); end

  Sorbet.sig.returns(String)
  def dump(); end

  Sorbet.sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(String)
  Sorbet.sig.returns(Enumerator[Integer])
  def each_byte(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  Sorbet.sig.returns(Enumerator[String])
  def each_char(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(String)
  Sorbet.sig.returns(Enumerator[Integer])
  def each_codepoint(&blk); end

  Sorbet.sig(
      arg0: String,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  Sorbet.sig(
      arg0: String,
  )
  .returns(Enumerator[String])
  def each_line(arg0=_, &blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  Sorbet.sig.returns(Encoding)
  def encoding(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def end_with?(*arg0); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  Sorbet.sig(
      arg0: T.any(String, Encoding),
  )
  .returns(String)
  def force_encoding(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T.nilable(Integer))
  def getbyte(arg0); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(String)
  Sorbet.sig(
      arg0: T.any(Regexp, String),
      arg1: Hash,
  )
  .returns(String)
  Sorbet.sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  Sorbet.sig(
      arg0: T.any(Regexp, String),
  )
  .returns(Enumerator[String])
  Sorbet.sig(
      arg0: T.any(Regexp, String),
  )
  .returns(String)
  def gsub(arg0, arg1=_, &blk); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: T.any(Regexp, String),
  )
  .returns(Enumerator[String])
  def gsub!(arg0, arg1=_, &blk); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(Integer)
  def hex(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(T.nilable(Integer))
  def index(arg0, arg1=_); end

  Sorbet.sig(
      str: String,
  )
  .void
  def initialize(str=_); end

  Sorbet.sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def insert(arg0, arg1); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(Symbol)
  def intern(); end

  Sorbet.sig.returns(Integer)
  def length(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(T::Array[String])
  def lines(arg0=_); end

  Sorbet.sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def ljust(arg0, arg1=_); end

  Sorbet.sig.returns(String)
  def lstrip(); end

  Sorbet.sig.returns(T.nilable(String))
  def lstrip!(); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
  )
  .returns(MatchData)
  Sorbet.sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(MatchData)
  def match(arg0, arg1=_); end

  Sorbet.sig.returns(String)
  def next(); end

  Sorbet.sig.returns(String)
  def next!(); end

  Sorbet.sig.returns(Integer)
  def oct(); end

  Sorbet.sig.returns(Integer)
  def ord(); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
  )
  .returns(T::Array[String])
  def partition(arg0); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def prepend(arg0); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def replace(arg0); end

  Sorbet.sig.returns(String)
  def reverse(); end

  Sorbet.sig(
      arg0: T.any(String, Regexp),
      arg1: Integer,
  )
  .returns(T.nilable(Integer))
  def rindex(arg0, arg1=_); end

  Sorbet.sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def rjust(arg0, arg1=_); end

  Sorbet.sig(
      arg0: T.any(String, Regexp),
  )
  .returns(T::Array[String])
  def rpartition(arg0); end

  Sorbet.sig.returns(String)
  def rstrip(); end

  Sorbet.sig.returns(String)
  def rstrip!(); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
  )
  .returns(T::Array[T.any(String, T::Array[String])])
  Sorbet.sig(
      arg0: T.any(Regexp, String),
      blk: BasicObject,
  )
  .returns(T::Array[T.any(String, T::Array[String])])
  def scan(arg0, &blk); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  Sorbet.sig(
      arg0: String,
      blk: T.proc(arg0: T.untyped).returns(BasicObject),
  )
  .returns(String)
  def scrub(arg0=_, &blk); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  Sorbet.sig(
      arg0: String,
      blk: T.proc(arg0: T.untyped).returns(BasicObject),
  )
  .returns(String)
  def scrub!(arg0=_, &blk); end

  Sorbet.sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(Integer)
  def setbyte(arg0, arg1); end

  Sorbet.sig.returns(Integer)
  def size(); end

  Sorbet.sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: T.any(T::Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def slice!(arg0, arg1=_); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(T::Array[String])
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T::Array[String])
  def split(arg0=_, arg1=_); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def squeeze(arg0=_); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def squeeze!(arg0=_); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def start_with?(*arg0); end

  Sorbet.sig.returns(String)
  def strip(); end

  Sorbet.sig.returns(String)
  def strip!(); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
      arg1: T.any(String, Hash),
  )
  .returns(String)
  Sorbet.sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  def sub(arg0, arg1=_, &blk); end

  Sorbet.sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(String)
  Sorbet.sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  def sub!(arg0, arg1=_, &blk); end

  Sorbet.sig.returns(String)
  def succ(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def sum(arg0=_); end

  Sorbet.sig.returns(String)
  def swapcase(); end

  Sorbet.sig.returns(T.nilable(String))
  def swapcase!(); end

  Sorbet.sig.returns(Complex)
  def to_c(); end

  Sorbet.sig.returns(Float)
  def to_f(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def to_i(arg0=_); end

  Sorbet.sig.returns(Rational)
  def to_r(); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig.returns(String)
  def to_str(); end

  Sorbet.sig.returns(Symbol)
  def to_sym(); end

  Sorbet.sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def tr(arg0, arg1); end

  Sorbet.sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def tr!(arg0, arg1); end

  Sorbet.sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def tr_s(arg0, arg1); end

  Sorbet.sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def tr_s!(arg0, arg1); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(T::Array[String])
  def unpack(arg0); end

  Sorbet.sig.returns(String)
  def upcase(); end

  Sorbet.sig.returns(T.nilable(String))
  def upcase!(); end

  type_parameters(:Bool).sig(
      arg0: String,
      arg1: T.type_parameter(:Bool),
  )
  .returns(Enumerator[String])
  type_parameters(:Bool).sig(
      arg0: String,
      arg1: T.type_parameter(:Bool),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  def upto(arg0, arg1=_, &blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def valid_encoding?(); end

  Sorbet.sig(
      obj: Object,
  )
  .returns(T.nilable(String))
  def self.try_convert(obj); end

  Sorbet.sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: T.any(T::Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def slice(arg0, arg1=_); end
end
