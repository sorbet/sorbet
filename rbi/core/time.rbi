# typed: true
class Time < Object
  include Comparable

  RFC2822_DAY_NAME = T.let(T.unsafe(nil), Array)
  RFC2822_MONTH_NAME = T.let(T.unsafe(nil), Array)

  sig do
    params(
        seconds: T.any(Time, Numeric)
    )
    .returns(Time)
  end
  sig do
    params(
        seconds: Numeric,
        microseconds_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.at(seconds, microseconds_with_frac=T.unsafe(nil)); end

  sig do
    params(
        year: Integer,
        month: T.any(Integer, String),
        day: Integer,
        hour: Integer,
        min: Integer,
        sec: Numeric,
        usec_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.gm(year, month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  sig do
    params(
        year: Integer,
        month: T.any(Integer, String),
        day: Integer,
        hour: Integer,
        min: Integer,
        sec: Numeric,
        usec_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.local(year, month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  sig {returns(Time)}
  def self.now(); end

  sig do
    params(
        year: Integer,
        month: T.any(Integer, String),
        day: Integer,
        hour: Integer,
        min: Integer,
        sec: Numeric,
        usec_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.utc(year, month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Time)
  end
  def +(arg0); end

  sig do
    params(
        arg0: Time,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Time)
  end
  def -(arg0); end

  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def <=(arg0); end

  sig do
    params(
        other: Time,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def >=(arg0); end

  sig {returns(String)}
  def asctime(); end

  sig {returns(String)}
  def ctime(); end

  sig {returns(Integer)}
  def day(); end

  sig {returns(T::Boolean)}
  def dst?(); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  sig {returns(T::Boolean)}
  def friday?(); end

  sig {returns(Time)}
  def getgm(); end

  sig do
    params(
        utc_offset: Integer,
    )
    .returns(Time)
  end
  def getlocal(utc_offset=T.unsafe(nil)); end

  sig {returns(Time)}
  def getutc(); end

  sig {returns(T::Boolean)}
  def gmt?(); end

  sig {returns(Integer)}
  def gmt_offset(); end

  sig {returns(Time)}
  def gmtime(); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def hour(); end

  sig do
    params(
        year: T.any(Integer, String),
        month: T.any(Integer, String),
        day: T.any(Integer, String),
        hour: T.any(Integer, String),
        min: T.any(Integer, String),
        sec: T.any(Numeric, String),
        usec_with_frac: T.any(Numeric, String),
    )
    .void
  end
  def initialize(year=T.unsafe(nil), month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(T::Boolean)}
  def isdst(); end

  sig do
    params(
        utc_offset: String,
    )
    .returns(Time)
  end
  def localtime(utc_offset=T.unsafe(nil)); end

  sig {returns(Integer)}
  def mday(); end

  sig {returns(Integer)}
  def min(); end

  sig {returns(Integer)}
  def mon(); end

  sig {returns(T::Boolean)}
  def monday?(); end

  sig {returns(Integer)}
  def nsec(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Time)
  end
  def round(arg0); end

  sig {returns(T::Boolean)}
  def saturday?(); end

  sig {returns(Integer)}
  def sec(); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def strftime(arg0); end

  sig {returns(Numeric)}
  def subsec(); end

  sig {returns(Time)}
  def succ(); end

  sig {returns(T::Boolean)}
  def sunday?(); end

  sig {returns(T::Boolean)}
  def thursday?(); end

  sig {returns([Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, T::Boolean, String])}
  def to_a(); end

  sig {returns(Float)}
  def to_f(); end

  sig {returns(Integer)}
  def to_i(); end

  sig {returns(Rational)}
  def to_r(); end

  sig {returns(String)}
  def to_s(); end

  sig {returns(T::Boolean)}
  def tuesday?(); end

  sig {returns(Numeric)}
  def tv_nsec(); end

  sig {returns(Numeric)}
  def tv_sec(); end

  sig {returns(Numeric)}
  def tv_usec(); end

  sig {returns(Numeric)}
  def usec(); end

  sig {returns(Time)}
  def utc(); end

  sig {returns(T::Boolean)}
  def utc?(); end

  sig {returns(Integer)}
  def utc_offset(); end

  sig {returns(Integer)}
  def wday(); end

  sig {returns(T::Boolean)}
  def wednesday?(); end

  sig {returns(Integer)}
  def yday(); end

  sig {returns(Integer)}
  def year(); end

  sig {returns(String)}
  def zone(); end

  sig do
    params(
        year: Integer,
        month: T.any(Integer, String),
        day: Integer,
        hour: Integer,
        min: Integer,
        sec: Numeric,
        usec_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.mktime(year, month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  sig {returns(Integer)}
  def gmtoff(); end

  sig {returns(Integer)}
  def month(); end
end
