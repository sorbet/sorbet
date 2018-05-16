# typed: true
class Time < Object
  include Comparable

  RFC2822_DAY_NAME = T.let(T.unsafe(nil), Array)
  RFC2822_MONTH_NAME = T.let(T.unsafe(nil), Array)

  sig(
      seconds: Time,
  )
  .returns(Time)
  sig(
      seconds: Numeric,
  )
  .returns(Time)
  sig(
      seconds: Numeric,
      microseconds_with_frac: Numeric,
  )
  .returns(Time)
  def self.at(seconds, microseconds_with_frac=_); end

  sig(
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

  sig(
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

  sig.returns(Time)
  def self.now(); end

  sig(
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

  sig(
      arg0: Numeric,
  )
  .returns(Time)
  def +(arg0); end

  sig(
      arg0: Time,
  )
  .returns(Float)
  sig(
      arg0: Numeric,
  )
  .returns(Time)
  def -(arg0); end

  sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      other: Time,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig.returns(String)
  def asctime(); end

  sig.returns(String)
  def ctime(); end

  sig.returns(Integer)
  def day(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def dst?(); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def friday?(); end

  sig.returns(Time)
  def getgm(); end

  sig(
      utc_offset: Integer,
  )
  .returns(Time)
  def getlocal(utc_offset=_); end

  sig.returns(Time)
  def getutc(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def gmt?(); end

  sig.returns(Integer)
  def gmt_offset(); end

  sig.returns(Time)
  def gmtime(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def hour(); end

  sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Object)
  def initialize(year=_, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def isdst(); end

  sig(
      utc_offset: String,
  )
  .returns(Time)
  def localtime(utc_offset=_); end

  sig.returns(Integer)
  def mday(); end

  sig.returns(Integer)
  def min(); end

  sig.returns(Integer)
  def mon(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def monday?(); end

  sig.returns(Integer)
  def nsec(); end

  sig(
      arg0: Integer,
  )
  .returns(Time)
  def round(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def saturday?(); end

  sig.returns(Integer)
  def sec(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def strftime(arg0); end

  sig.returns(Numeric)
  def subsec(); end

  sig.returns(Time)
  def succ(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def sunday?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def thursday?(); end

  sig.returns([Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, T.any(TrueClass, FalseClass), String])
  def to_a(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Numeric)
  def to_i(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def tuesday?(); end

  sig.returns(Numeric)
  def tv_nsec(); end

  sig.returns(Numeric)
  def tv_sec(); end

  sig.returns(Numeric)
  def tv_usec(); end

  sig.returns(Numeric)
  def usec(); end

  sig.returns(Time)
  def utc(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def utc?(); end

  sig.returns(Integer)
  def utc_offset(); end

  sig.returns(Integer)
  def wday(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def wednesday?(); end

  sig.returns(Integer)
  def yday(); end

  sig.returns(Integer)
  def year(); end

  sig.returns(String)
  def zone(); end

  sig(
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

  sig.returns(Integer)
  def gmtoff(); end

  sig.returns(Integer)
  def month(); end
end
