# typed: true
class String < Object
  include Comparable

  sig(
      arg0: Object,
  )
  .returns(String)
  def %(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(String)
  def *(arg0); end

  sig(
      arg0: String,
  )
  .returns(String)
  def +(arg0); end

  sig(
      arg0: Object,
  )
  .returns(String)
  def <<(arg0); end

  sig(
      other: String,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.nilable(Integer))
  def =~(arg0); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(T::Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def [](arg0, arg1=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def ascii_only?(); end

  sig.returns(String)
  def b(); end

  sig.returns(Array)
  def bytes(); end

  sig.returns(Integer)
  def bytesize(); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T::Range[Integer],
  )
  .returns(T.nilable(String))
  def byteslice(arg0, arg1=_); end

  sig.returns(String)
  def capitalize(); end

  sig.returns(T.nilable(String))
  def capitalize!(); end

  sig(
      arg0: String,
  )
  .returns(T.nilable(Integer))
  def casecmp(arg0); end

  sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def center(arg0, arg1=_); end

  sig.returns(Array)
  def chars(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def chomp(arg0=_); end

  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def chomp!(arg0=_); end

  sig.returns(String)
  def chop(); end

  sig.returns(T.nilable(String))
  def chop!(); end

  sig.returns(String)
  def chr(); end

  sig.returns(String)
  def clear(); end

  sig.returns(T::Array[Integer])
  sig(
      blk: BasicObject,
  )
  .returns(T::Array[Integer])
  def codepoints(&blk); end

  sig(
      arg0: T.any(Integer, Object),
  )
  .returns(String)
  def concat(arg0); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(Integer)
  def count(arg0, *arg1); end

  sig(
      arg0: String,
  )
  .returns(String)
  def crypt(arg0); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def delete(arg0, *arg1); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def delete!(arg0, *arg1); end

  sig.returns(String)
  def downcase(); end

  sig.returns(T.nilable(String))
  def downcase!(); end

  sig.returns(String)
  def dump(); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(String)
  sig.returns(Enumerator[Integer])
  def each_byte(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  sig.returns(Enumerator[String])
  def each_char(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(String)
  sig.returns(Enumerator[Integer])
  def each_codepoint(&blk); end

  sig(
      arg0: String,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  sig(
      arg0: String,
  )
  .returns(Enumerator[String])
  def each_line(arg0=_, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig.returns(Encoding)
  def encoding(); end

  sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def end_with?(*arg0); end

  sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: T.any(String, Encoding),
  )
  .returns(String)
  def force_encoding(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.nilable(Integer))
  def getbyte(arg0); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
      arg1: Hash,
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(Enumerator[String])
  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(String)
  def gsub(arg0, arg1=_, &blk); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(Enumerator[String])
  def gsub!(arg0, arg1=_, &blk); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def hex(); end

  sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(T.nilable(Integer))
  def index(arg0, arg1=_); end

  sig(
      str: String,
  )
  .returns(Object)
  def initialize(str=_); end

  sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def insert(arg0, arg1); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Symbol)
  def intern(); end

  sig.returns(Integer)
  def length(); end

  sig(
      arg0: String,
  )
  .returns(T::Array[String])
  def lines(arg0=_); end

  sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def ljust(arg0, arg1=_); end

  sig.returns(String)
  def lstrip(); end

  sig.returns(T.nilable(String))
  def lstrip!(); end

  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(MatchData)
  sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(MatchData)
  def match(arg0, arg1=_); end

  sig.returns(String)
  def next(); end

  sig.returns(String)
  def next!(); end

  sig.returns(Integer)
  def oct(); end

  sig.returns(Integer)
  def ord(); end

  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(T::Array[String])
  def partition(arg0); end

  sig(
      arg0: String,
  )
  .returns(String)
  def prepend(arg0); end

  sig(
      arg0: String,
  )
  .returns(String)
  def replace(arg0); end

  sig.returns(String)
  def reverse(); end

  sig(
      arg0: T.any(String, Regexp),
      arg1: Integer,
  )
  .returns(T.nilable(Integer))
  def rindex(arg0, arg1=_); end

  sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def rjust(arg0, arg1=_); end

  sig(
      arg0: T.any(String, Regexp),
  )
  .returns(T::Array[String])
  def rpartition(arg0); end

  sig.returns(String)
  def rstrip(); end

  sig.returns(String)
  def rstrip!(); end

  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(T::Array[T.any(String, T::Array[String])])
  sig(
      arg0: T.any(Regexp, String),
      blk: BasicObject,
  )
  .returns(T::Array[T.any(String, T::Array[String])])
  def scan(arg0, &blk); end

  sig(
      arg0: String,
  )
  .returns(String)
  sig(
      arg0: String,
      blk: T.proc(arg0: T.untyped).returns(BasicObject),
  )
  .returns(String)
  def scrub(arg0=_, &blk); end

  sig(
      arg0: String,
  )
  .returns(String)
  sig(
      arg0: String,
      blk: T.proc(arg0: T.untyped).returns(BasicObject),
  )
  .returns(String)
  def scrub!(arg0=_, &blk); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(Integer)
  def setbyte(arg0, arg1); end

  sig.returns(Integer)
  def size(); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(T::Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def slice!(arg0, arg1=_); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(T::Array[String])
  sig(
      arg0: Integer,
  )
  .returns(T::Array[String])
  def split(arg0=_, arg1=_); end

  sig(
      arg0: String,
  )
  .returns(String)
  def squeeze(arg0=_); end

  sig(
      arg0: String,
  )
  .returns(String)
  def squeeze!(arg0=_); end

  sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def start_with?(*arg0); end

  sig.returns(String)
  def strip(); end

  sig.returns(String)
  def strip!(); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: T.any(String, Hash),
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  def sub(arg0, arg1=_, &blk); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  def sub!(arg0, arg1=_, &blk); end

  sig.returns(String)
  def succ(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def sum(arg0=_); end

  sig.returns(String)
  def swapcase(); end

  sig.returns(T.nilable(String))
  def swapcase!(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def to_i(arg0=_); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(String)
  def to_str(); end

  sig.returns(Symbol)
  def to_sym(); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def tr(arg0, arg1); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def tr!(arg0, arg1); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def tr_s(arg0, arg1); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def tr_s!(arg0, arg1); end

  sig(
      arg0: String,
  )
  .returns(T::Array[String])
  def unpack(arg0); end

  sig.returns(String)
  def upcase(); end

  sig.returns(T.nilable(String))
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

  sig.returns(T.any(TrueClass, FalseClass))
  def valid_encoding?(); end

  sig(
      obj: Object,
  )
  .returns(T.nilable(String))
  def self.try_convert(obj); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(T::Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def slice(arg0, arg1=_); end
end
