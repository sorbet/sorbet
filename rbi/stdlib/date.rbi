# typed: strict
class Date::Infinity
  Sorbet.sig.returns(Date::Infinity)
  def +@(); end

  Sorbet.sig.returns(Date::Infinity)
  def -@(); end

  Sorbet.sig(other: T.untyped).returns(T.nilable(Integer))
  def <=>(other); end

  Sorbet.sig.returns(Float)
  def to_f(); end

  Sorbet.sig(other: T.untyped).returns(Numeric)
  def coerce(other); end

  Sorbet.sig.returns(Date::Infinity)
  def abs(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def finite?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def infinite?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def nan?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def d(); end
end

class Date
  Sorbet.sig(
    year: Integer,
    month: Integer,
    mday: Integer,
    start: Integer,
  )
  .void
  def initialize(year=-4712, month=1, mday=1, start=Date::ITALY); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def upto(arg0); end

  Sorbet.sig(
    locale: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def localize(locale=_, options=_); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def <=>(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def <<(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def >>(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def ===(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def eql?(arg0); end

  Sorbet.sig.returns(T.untyped)
  def start(); end

  Sorbet.sig.returns(T.untyped)
  def marshal_dump(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def marshal_load(arg0); end

  Sorbet.sig.returns(T.untyped)
  def ajd(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def +(arg0); end

  Sorbet.sig.returns(T.untyped)
  def inspect(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def -(arg0); end

  Sorbet.sig.returns(T.untyped)
  def mday(); end

  Sorbet.sig.returns(T.untyped)
  def day(); end

  Sorbet.sig.returns(T.untyped)
  def mon(); end

  Sorbet.sig.returns(T.untyped)
  def month(); end

  Sorbet.sig.returns(T.untyped)
  def year(); end

  Sorbet.sig.returns(T.untyped)
  def wday(); end

  Sorbet.sig.returns(T.untyped)
  def yday(); end

  Sorbet.sig.returns(T.untyped)
  def ctime(); end

  Sorbet.sig.returns(T.untyped)
  def pretty_date(); end

  Sorbet.sig.returns(T.untyped)
  def succ(); end

  Sorbet.sig.returns(T.untyped)
  def to_utc_time(); end

  Sorbet.sig.returns(T.untyped)
  def jd(); end

  Sorbet.sig.returns(T.untyped)
  def sunday?(); end

  Sorbet.sig.returns(T.untyped)
  def monday?(); end

  Sorbet.sig.returns(T.untyped)
  def tuesday?(); end

  Sorbet.sig.returns(T.untyped)
  def wednesday?(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def step(*arg0); end

  Sorbet.sig.returns(T.untyped)
  def friday?(); end

  Sorbet.sig.returns(T.untyped)
  def saturday?(); end

  Sorbet.sig.returns(T.untyped)
  def blank?(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def downto(arg0); end

  Sorbet.sig.returns(T.untyped)
  def asctime(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def strftime(*arg0); end

  Sorbet.sig.returns(T.untyped)
  def thursday?(); end

  Sorbet.sig.returns(T.untyped)
  def to_s(); end

  Sorbet.sig.returns(T.untyped)
  def leap?(); end

  Sorbet.sig.returns(T.untyped)
  def iso8601(); end

  Sorbet.sig.returns(T.untyped)
  def rfc3339(); end

  Sorbet.sig.returns(T.untyped)
  def xmlschema(); end

  Sorbet.sig.returns(T.untyped)
  def rfc2822(); end

  Sorbet.sig.returns(T.untyped)
  def rfc822(); end

  Sorbet.sig.returns(T.untyped)
  def httpdate(); end

  Sorbet.sig.returns(T.untyped)
  def jisx0301(); end

  Sorbet.sig.returns(T.untyped)
  def amjd(); end

  Sorbet.sig.returns(T.untyped)
  def mjd(); end

  Sorbet.sig.returns(T.untyped)
  def day_fraction(); end

  Sorbet.sig.returns(T.untyped)
  def cwyear(); end

  Sorbet.sig.returns(T.untyped)
  def cweek(); end

  Sorbet.sig.returns(T.untyped)
  def cwday(); end

  Sorbet.sig.returns(T.untyped)
  def hash(); end

  Sorbet.sig.returns(T.untyped)
  def julian?(); end

  Sorbet.sig.returns(T.untyped)
  def gregorian?(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def new_start(*arg0); end

  Sorbet.sig.returns(T.untyped)
  def italy(); end

  Sorbet.sig.returns(T.untyped)
  def england(); end

  Sorbet.sig.returns(T.untyped)
  def julian(); end

  Sorbet.sig.returns(T.untyped)
  def gregorian(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def next_day(*arg0); end

  Sorbet.sig.returns(T.untyped)
  def ld(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def next_month(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def prev_day(*arg0); end

  Sorbet.sig.returns(T.untyped)
  def next(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def prev_month(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def next_year(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def prev_year(*arg0); end

  Sorbet.sig.returns(Time)
  def to_time(); end

  Sorbet.sig.returns(Date)
  def to_date(); end

  Sorbet.sig.returns(DateTime)
  def to_datetime(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.new(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self._load(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.today(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.parse(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.jd(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_jd?(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_ordinal?(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_civil?(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_date?(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.valid_commercial?(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.julian_leap?(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.gregorian_leap?(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.leap?(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.ordinal(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.civil(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.commercial(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self._strptime(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.strptime(*arg0); end

  Sorbet.sig(
    arg0: String,
    comp: T.any(TrueClass, FalseClass)
  )
  .returns(Hash)
  def self._parse(arg0, comp=true); end

  Sorbet.sig(arg0: String).returns(Hash)
  def self._iso8601(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.iso8601(*arg0); end

  Sorbet.sig(arg0: String).returns(Hash)
  def self._rfc3339(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc3339(*arg0); end

  Sorbet.sig(arg0: String).returns(Hash)
  def self._xmlschema(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.xmlschema(*arg0); end

  Sorbet.sig(arg0: String).returns(Hash)
  def self._rfc2822(arg0); end

  Sorbet.sig(arg0: String).returns(Hash)
  def self._rfc822(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc2822(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc822(*arg0); end

  Sorbet.sig(arg0: String).returns(Hash)
  def self._httpdate(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.httpdate(*arg0); end

  Sorbet.sig(arg0: String).returns(Hash)
  def self._jisx0301(arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.jisx0301(*arg0); end
end

class DateTime
  Sorbet.sig.returns(T.untyped)
  def min(); end

  Sorbet.sig.returns(T.untyped)
  def to_s(); end

  Sorbet.sig.returns(T.untyped)
  def offset(); end

  Sorbet.sig.returns(T.untyped)
  def zone(); end

  Sorbet.sig.returns(T.untyped)
  def sec(); end

  Sorbet.sig.returns(T.untyped)
  def hour(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def strftime(*arg0); end

  Sorbet.sig.returns(T.untyped)
  def second(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def iso8601(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def rfc3339(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def xmlschema(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def jisx0301(*arg0); end

  Sorbet.sig.returns(T.untyped)
  def minute(); end

  Sorbet.sig.returns(T.untyped)
  def sec_fraction(); end

  Sorbet.sig.returns(T.untyped)
  def second_fraction(); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def new_offset(*arg0); end

  Sorbet.sig.returns(Time)
  def to_time(); end

  Sorbet.sig.returns(Date)
  def to_date(); end

  Sorbet.sig.returns(DateTime)
  def to_datetime(); end

  Sorbet.sig.returns(T.untyped)
  def blank?(); end

  Sorbet.sig.returns(T.untyped)
  def to_utc_time(); end

  Sorbet.sig(
    locale: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def localize(locale=_, options=_); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.new(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.now(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.parse(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.jd(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.ordinal(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.civil(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.commercial(*arg0); end

  Sorbet.sig(
    arg0: String,
    format: String
  )
  .returns(Hash)
  def self._strptime(arg0, format="%F"); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.strptime(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.iso8601(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc3339(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.xmlschema(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc2822(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.rfc822(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.httpdate(*arg0); end

  Sorbet.sig(arg0: T.untyped).returns(T.untyped)
  def self.jisx0301(*arg0); end
end

class Time
  Sorbet.sig.returns(Time)
  def to_time(); end

  Sorbet.sig.returns(Date)
  def to_date(); end

  Sorbet.sig.returns(DateTime)
  def to_datetime(); end
end
