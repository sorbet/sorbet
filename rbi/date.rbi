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
  ITALY = _
  MONTHNAMES = _
  ABBR_MONTHNAMES = _
  DAYNAMES = _
  ABBR_DAYNAMES = _
  ENGLAND = _
  JULIAN = _
  GREGORIAN = _

  sig(_: T.untyped).returns(T.untyped)
  def upto(_); end

  sig(
    locale: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def localize(locale=_, options=_); end

  sig(_: T.untyped).returns(T.untyped)
  def <=>(_); end

  sig(_: T.untyped).returns(T.untyped)
  def <<(_); end

  sig(_: T.untyped).returns(T.untyped)
  def >>(_); end

  sig(_: T.untyped).returns(T.untyped)
  def ===(_); end

  sig(_: T.untyped).returns(T.untyped)
  def eql?(_); end

  sig.returns(T.untyped)
  def start(); end

  sig.returns(T.untyped)
  def marshal_dump(); end

  sig(_: T.untyped).returns(T.untyped)
  def marshal_load(_); end

  sig.returns(T.untyped)
  def ajd(); end

  sig(_: T.untyped).returns(T.untyped)
  def +(_); end

  sig.returns(T.untyped)
  def inspect(); end

  sig(_: T.untyped).returns(T.untyped)
  def -(_); end

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

  sig(_: T.untyped).returns(T.untyped)
  def step(*_); end

  sig.returns(T.untyped)
  def friday?(); end

  sig.returns(T.untyped)
  def saturday?(); end

  sig.returns(T.untyped)
  def blank?(); end

  sig(_: T.untyped).returns(T.untyped)
  def downto(_); end

  sig.returns(T.untyped)
  def asctime(); end

  sig(_: T.untyped).returns(T.untyped)
  def strftime(*_); end

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

  sig(_: T.untyped).returns(T.untyped)
  def new_start(*_); end

  sig.returns(T.untyped)
  def italy(); end

  sig.returns(T.untyped)
  def england(); end

  sig.returns(T.untyped)
  def julian(); end

  sig.returns(T.untyped)
  def gregorian(); end

  sig(_: T.untyped).returns(T.untyped)
  def next_day(*_); end

  sig.returns(T.untyped)
  def ld(); end

  sig(_: T.untyped).returns(T.untyped)
  def next_month(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def prev_day(*_); end

  sig.returns(T.untyped)
  def next(); end

  sig(_: T.untyped).returns(T.untyped)
  def prev_month(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def next_year(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def prev_year(*_); end

  sig.returns(Time)
  def to_time(); end

  sig.returns(Date)
  def to_date(); end

  sig.returns(DateTime)
  def to_datetime(); end

  sig(_: T.untyped).returns(T.untyped)
  def self.new(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self._load(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.today(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.parse(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.jd(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.valid_jd?(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.valid_ordinal?(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.valid_civil?(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.valid_date?(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.valid_commercial?(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.julian_leap?(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.gregorian_leap?(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.leap?(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.ordinal(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.civil(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.commercial(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self._strptime(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.strptime(*_); end

  sig(
    _: String,
    comp: T.any(TrueClass, FalseClass)
  )
  .returns(Hash)
  def self._parse(_, comp=true); end

  sig(_: String).returns(Hash)
  def self._iso8601(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.iso8601(*_); end

  sig(_: String).returns(Hash)
  def self._rfc3339(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.rfc3339(*_); end

  sig(_: String).returns(Hash)
  def self._xmlschema(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.xmlschema(*_); end

  sig(_: String).returns(Hash)
  def self._rfc2822(_); end

  sig(_: String).returns(Hash)
  def self._rfc822(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.rfc2822(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.rfc822(*_); end

  sig(_: String).returns(Hash)
  def self._httpdate(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.httpdate(*_); end

  sig(_: String).returns(Hash)
  def self._jisx0301(_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.jisx0301(*_); end
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

  sig(_: T.untyped).returns(T.untyped)
  def strftime(*_); end

  sig.returns(T.untyped)
  def second(); end

  sig(_: T.untyped).returns(T.untyped)
  def iso8601(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def rfc3339(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def xmlschema(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def jisx0301(*_); end

  sig.returns(T.untyped)
  def minute(); end

  sig.returns(T.untyped)
  def sec_fraction(); end

  sig.returns(T.untyped)
  def second_fraction(); end

  sig(_: T.untyped).returns(T.untyped)
  def new_offset(*_); end

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

  sig(_: T.untyped).returns(T.untyped)
  def self.new(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.now(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.parse(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.jd(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.ordinal(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.civil(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.commercial(*_); end

  sig(
    _: String,
    format: String
  )
  .returns(Hash)
  def self._strptime(_, format="%F"); end

  sig(_: T.untyped).returns(T.untyped)
  def self.strptime(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.iso8601(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.rfc3339(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.xmlschema(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.rfc2822(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.rfc822(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.httpdate(*_); end

  sig(_: T.untyped).returns(T.untyped)
  def self.jisx0301(*_); end
end

class Time
  sig.returns(Time)
  def to_time(); end

  sig.returns(Date)
  def to_date(); end

  sig.returns(DateTime)
  def to_datetime(); end
end
