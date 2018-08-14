# typed: true
class Time < Object
  include Comparable

  RFC2822_DAY_NAME = T.let(T.unsafe(nil), Array)
  RFC2822_MONTH_NAME = T.let(T.unsafe(nil), Array)

  Sorbet.sig(
      seconds: T.any(Time, Numeric)
  )
  .returns(Time)
  Sorbet.sig(
      seconds: Numeric,
      microseconds_with_frac: Numeric,
  )
  .returns(Time)
  def self.at(seconds, microseconds_with_frac=_); end

  Sorbet.sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Time)
  def self.gm(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  Sorbet.sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Time)
  def self.local(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  Sorbet.sig.returns(Time)
  def self.now(); end

  Sorbet.sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Time)
  def self.utc(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Time)
  def +(arg0); end

  Sorbet.sig(
      arg0: Time,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Time)
  def -(arg0); end

  Sorbet.sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  Sorbet.sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  Sorbet.sig(
      other: Time,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  Sorbet.sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  Sorbet.sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  Sorbet.sig.returns(String)
  def asctime(); end

  Sorbet.sig.returns(String)
  def ctime(); end

  Sorbet.sig.returns(Integer)
  def day(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def dst?(); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def friday?(); end

  Sorbet.sig.returns(Time)
  def getgm(); end

  Sorbet.sig(
      utc_offset: Integer,
  )
  .returns(Time)
  def getlocal(utc_offset=_); end

  Sorbet.sig.returns(Time)
  def getutc(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def gmt?(); end

  Sorbet.sig.returns(Integer)
  def gmt_offset(); end

  Sorbet.sig.returns(T.self_type)
  def gmtime(); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(Integer)
  def hour(); end

  Sorbet.sig(
      year: T.any(Integer, String),
      month: T.any(Integer, String),
      day: T.any(Integer, String),
      hour: T.any(Integer, String),
      min: T.any(Integer, String),
      sec: T.any(Numeric, String),
      usec_with_frac: T.any(Numeric, String),
  )
  .void
  def initialize(year=_, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def isdst(); end

  Sorbet.sig(
      utc_offset: String,
  )
  .returns(T.self_type)
  def localtime(utc_offset=_); end

  Sorbet.sig.returns(Integer)
  def mday(); end

  Sorbet.sig.returns(Integer)
  def min(); end

  Sorbet.sig.returns(Integer)
  def mon(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def monday?(); end

  Sorbet.sig.returns(Integer)
  def nsec(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Time)
  def round(arg0); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def saturday?(); end

  Sorbet.sig.returns(Integer)
  def sec(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def strftime(arg0); end

  Sorbet.sig.returns(Numeric)
  def subsec(); end

  Sorbet.sig.returns(Time)
  def succ(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def sunday?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def thursday?(); end

  Sorbet.sig.returns([Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, T.any(TrueClass, FalseClass), String])
  def to_a(); end

  Sorbet.sig.returns(Float)
  def to_f(); end

  Sorbet.sig.returns(Integer)
  def to_i(); end

  Sorbet.sig.returns(Rational)
  def to_r(); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def tuesday?(); end

  Sorbet.sig.returns(Numeric)
  def tv_nsec(); end

  Sorbet.sig.returns(Numeric)
  def tv_sec(); end

  Sorbet.sig.returns(Numeric)
  def tv_usec(); end

  Sorbet.sig.returns(Numeric)
  def usec(); end

  Sorbet.sig.returns(T.self_type)
  def utc(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def utc?(); end

  Sorbet.sig.returns(Integer)
  def utc_offset(); end

  Sorbet.sig.returns(Integer)
  def wday(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def wednesday?(); end

  Sorbet.sig.returns(Integer)
  def yday(); end

  Sorbet.sig.returns(Integer)
  def year(); end

  Sorbet.sig.returns(String)
  def zone(); end

  Sorbet.sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Time)
  def self.mktime(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  Sorbet.sig.returns(Integer)
  def gmtoff(); end

  Sorbet.sig.returns(Integer)
  def month(); end
end
