# typed: strict
class Date::Infinity
  sig.returns(Date::Infinity)
  def +@(); end

  sig.returns(Date::Infinity)
  def -@(); end

  sig(other: T.untyped).returns(T.nilable(Integer))
  def <=>(other); end

  sig.returns(Float)
  def to_f(); end

  sig(other: T.untyped).returns(Numeric)
  def coerce(other); end

  sig.returns(Date::Infinity)
  def abs(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def finite?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def infinite?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def nan?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def d(); end
end

class Date
  sig(
    year: Integer,
    month: Integer,
    mday: Integer,
    start: Integer,
  )
  .void
  def initialize(year=-4712, month=1, mday=1, start=Date::ITALY); end

  sig(arg0: T.untyped).returns(T.untyped)
  def upto(arg0); end

  sig(
    locale: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def localize(locale=_, options=_); end

  sig(arg0: T.untyped).returns(T.untyped)
  def <=>(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def <<(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def >>(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def ===(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def eql?(arg0); end

  sig.returns(T.untyped)
  def start(); end

  sig.returns(T.untyped)
  def marshal_dump(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def marshal_load(arg0); end

  sig.returns(T.untyped)
  def ajd(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def +(arg0); end

  sig.returns(T.untyped)
  def inspect(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def -(arg0); end

  sig.returns(T.untyped)
  def mday(); end

  sig.returns(T.untyped)
  def day(); end

  sig.returns(T.untyped)
  def mon(); end

  sig.returns(T.untyped)
  def month(); end

  sig.returns(T.untyped)
  def year(); end

  sig.returns(T.untyped)
  def wday(); end

  sig.returns(T.untyped)
  def yday(); end

  sig.returns(T.untyped)
  def ctime(); end

  sig.returns(T.untyped)
  def pretty_date(); end

  sig.returns(T.untyped)
  def succ(); end

  sig.returns(T.untyped)
  def to_utc_time(); end

  sig.returns(T.untyped)
  def jd(); end

  sig.returns(T.untyped)
  def sunday?(); end

  sig.returns(T.untyped)
  def monday?(); end

  sig.returns(T.untyped)
  def tuesday?(); end

  sig.returns(T.untyped)
  def wednesday?(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def step(*arg0); end

  sig.returns(T.untyped)
  def friday?(); end

  sig.returns(T.untyped)
  def saturday?(); end

  sig.returns(T.untyped)
  def blank?(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def downto(arg0); end

  sig.returns(T.untyped)
  def asctime(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def strftime(*arg0); end

  sig.returns(T.untyped)
  def thursday?(); end

  sig.returns(T.untyped)
  def to_s(); end

  sig.returns(T.untyped)
  def leap?(); end

  sig.returns(T.untyped)
  def iso8601(); end

  sig.returns(T.untyped)
  def rfc3339(); end

  sig.returns(T.untyped)
  def xmlschema(); end

  sig.returns(T.untyped)
  def rfc2822(); end

  sig.returns(T.untyped)
  def rfc822(); end

  sig.returns(T.untyped)
  def httpdate(); end

  sig.returns(T.untyped)
  def jisx0301(); end

  sig.returns(T.untyped)
  def amjd(); end

  sig.returns(T.untyped)
  def mjd(); end

  sig.returns(T.untyped)
  def day_fraction(); end

  sig.returns(T.untyped)
  def cwyear(); end

  sig.returns(T.untyped)
  def cweek(); end

  sig.returns(T.untyped)
  def cwday(); end

  sig.returns(T.untyped)
  def hash(); end

  sig.returns(T.untyped)
  def julian?(); end

  sig.returns(T.untyped)
  def gregorian?(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def new_start(*arg0); end

  sig.returns(T.untyped)
  def italy(); end

  sig.returns(T.untyped)
  def england(); end

  sig.returns(T.untyped)
  def julian(); end

  sig.returns(T.untyped)
  def gregorian(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def next_day(*arg0); end

  sig.returns(T.untyped)
  def ld(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def next_month(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def prev_day(*arg0); end

  sig.returns(T.untyped)
  def next(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def prev_month(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def next_year(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def prev_year(*arg0); end

  sig.returns(Time)
  def to_time(); end

  sig.returns(Date)
  def to_date(); end

  sig.returns(DateTime)
  def to_datetime(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.new(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self._load(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.today(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.parse(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.jd(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_jd?(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_ordinal?(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_civil?(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_date?(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_commercial?(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.julian_leap?(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.gregorian_leap?(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.leap?(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.ordinal(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.civil(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.commercial(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self._strptime(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.strptime(*arg0); end

  sig(
    arg0: String,
    comp: T.any(TrueClass, FalseClass)
  )
  .returns(Hash)
  def self._parse(arg0, comp=true); end

  sig(arg0: String).returns(Hash)
  def self._iso8601(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.iso8601(*arg0); end

  sig(arg0: String).returns(Hash)
  def self._rfc3339(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc3339(*arg0); end

  sig(arg0: String).returns(Hash)
  def self._xmlschema(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.xmlschema(*arg0); end

  sig(arg0: String).returns(Hash)
  def self._rfc2822(arg0); end

  sig(arg0: String).returns(Hash)
  def self._rfc822(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc2822(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc822(*arg0); end

  sig(arg0: String).returns(Hash)
  def self._httpdate(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.httpdate(*arg0); end

  sig(arg0: String).returns(Hash)
  def self._jisx0301(arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.jisx0301(*arg0); end
end

class DateTime
  sig.returns(T.untyped)
  def min(); end

  sig.returns(T.untyped)
  def to_s(); end

  sig.returns(T.untyped)
  def offset(); end

  sig.returns(T.untyped)
  def zone(); end

  sig.returns(T.untyped)
  def sec(); end

  sig.returns(T.untyped)
  def hour(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def strftime(*arg0); end

  sig.returns(T.untyped)
  def second(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def iso8601(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def rfc3339(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def xmlschema(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def jisx0301(*arg0); end

  sig.returns(T.untyped)
  def minute(); end

  sig.returns(T.untyped)
  def sec_fraction(); end

  sig.returns(T.untyped)
  def second_fraction(); end

  sig(arg0: T.untyped).returns(T.untyped)
  def new_offset(*arg0); end

  sig.returns(Time)
  def to_time(); end

  sig.returns(Date)
  def to_date(); end

  sig.returns(DateTime)
  def to_datetime(); end

  sig.returns(T.untyped)
  def blank?(); end

  sig.returns(T.untyped)
  def to_utc_time(); end

  sig(
    locale: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def localize(locale=_, options=_); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.new(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.now(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.parse(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.jd(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.ordinal(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.civil(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.commercial(*arg0); end

  sig(
    arg0: String,
    format: String
  )
  .returns(Hash)
  def self._strptime(arg0, format="%F"); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.strptime(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.iso8601(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc3339(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.xmlschema(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc2822(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc822(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.httpdate(*arg0); end

  sig(arg0: T.untyped).returns(T.untyped)
  def self.jisx0301(*arg0); end
end

class Time
  sig.returns(Time)
  def to_time(); end

  sig.returns(Date)
  def to_date(); end

  sig.returns(DateTime)
  def to_datetime(); end
end
