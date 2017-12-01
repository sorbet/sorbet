class Date::Infinity
  standard_method({}, returns: Date::Infinity)
  def +@(); end

  standard_method({}, returns: Date::Infinity)
  def -@(); end

  standard_method({other: Opus::Types.untyped}, returns: Opus::Types.nilable(Integer))
  def <=>(other); end

  standard_method({}, returns: Float)
  def to_f(); end

  standard_method({other: Opus::Types.untyped}, returns: Numeric)
  def coerce(other); end

  standard_method({}, returns: Date::Infinity)
  def abs(); end

  standard_method({}, returns: Boolean)
  def zero?(); end

  standard_method({}, returns: Boolean)
  def finite?(); end

  standard_method({}, returns: Boolean)
  def infinite?(); end

  standard_method({}, returns: Boolean)
  def nan?(); end

  standard_method({}, returns: Boolean)
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

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def upto(_); end

  standard_method(
    {
      locale: Opus::Types.untyped,
      options: Opus::Types.untyped,
    },
    returns: Opus::Types.untyped,
  )
  def localize(locale=_, options=_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def <=>(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def <<(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def >>(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def ===(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def eql?(_); end

  standard_method({}, returns: Opus::Types.untyped)
  def start(); end

  standard_method({}, returns: Opus::Types.untyped)
  def marshal_dump(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def marshal_load(_); end

  standard_method({}, returns: Opus::Types.untyped)
  def ajd(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def +(_); end

  standard_method({}, returns: Opus::Types.untyped)
  def inspect(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def -(_); end

  standard_method({}, returns: Opus::Types.untyped)
  def mday(); end

  standard_method({}, returns: Opus::Types.untyped)
  def day(); end

  standard_method({}, returns: Opus::Types.untyped)
  def mon(); end

  standard_method({}, returns: Opus::Types.untyped)
  def month(); end

  standard_method({}, returns: Opus::Types.untyped)
  def year(); end

  standard_method({}, returns: Opus::Types.untyped)
  def wday(); end

  standard_method({}, returns: Opus::Types.untyped)
  def yday(); end

  standard_method({}, returns: Opus::Types.untyped)
  def ctime(); end

  standard_method({}, returns: Opus::Types.untyped)
  def pretty_date(); end

  standard_method({}, returns: Opus::Types.untyped)
  def succ(); end

  standard_method({}, returns: Opus::Types.untyped)
  def to_utc_time(); end

  standard_method({}, returns: Opus::Types.untyped)
  def jd(); end

  standard_method({}, returns: Opus::Types.untyped)
  def sunday?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def monday?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def tuesday?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def wednesday?(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def step(*_); end

  standard_method({}, returns: Opus::Types.untyped)
  def friday?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def saturday?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def blank?(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def downto(_); end

  standard_method({}, returns: Opus::Types.untyped)
  def asctime(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def strftime(*_); end

  standard_method({}, returns: Opus::Types.untyped)
  def thursday?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def to_s(); end

  standard_method({}, returns: Opus::Types.untyped)
  def leap?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def iso8601(); end

  standard_method({}, returns: Opus::Types.untyped)
  def rfc3339(); end

  standard_method({}, returns: Opus::Types.untyped)
  def xmlschema(); end

  standard_method({}, returns: Opus::Types.untyped)
  def rfc2822(); end

  standard_method({}, returns: Opus::Types.untyped)
  def rfc822(); end

  standard_method({}, returns: Opus::Types.untyped)
  def httpdate(); end

  standard_method({}, returns: Opus::Types.untyped)
  def jisx0301(); end

  standard_method({}, returns: Opus::Types.untyped)
  def amjd(); end

  standard_method({}, returns: Opus::Types.untyped)
  def mjd(); end

  standard_method({}, returns: Opus::Types.untyped)
  def day_fraction(); end

  standard_method({}, returns: Opus::Types.untyped)
  def cwyear(); end

  standard_method({}, returns: Opus::Types.untyped)
  def cweek(); end

  standard_method({}, returns: Opus::Types.untyped)
  def cwday(); end

  standard_method({}, returns: Opus::Types.untyped)
  def hash(); end

  standard_method({}, returns: Opus::Types.untyped)
  def julian?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def gregorian?(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def new_start(*_); end

  standard_method({}, returns: Opus::Types.untyped)
  def italy(); end

  standard_method({}, returns: Opus::Types.untyped)
  def england(); end

  standard_method({}, returns: Opus::Types.untyped)
  def julian(); end

  standard_method({}, returns: Opus::Types.untyped)
  def gregorian(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def next_day(*_); end

  standard_method({}, returns: Opus::Types.untyped)
  def ld(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def next_month(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def prev_day(*_); end

  standard_method({}, returns: Opus::Types.untyped)
  def next(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def prev_month(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def next_year(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def prev_year(*_); end

  standard_method({}, returns: Time)
  def to_time(); end

  standard_method({}, returns: Date)
  def to_date(); end

  standard_method({}, returns: DateTime)
  def to_datetime(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.new(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self._load(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.today(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.parse(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.jd(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.valid_jd?(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.valid_ordinal?(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.valid_civil?(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.valid_date?(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.valid_commercial?(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.julian_leap?(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.gregorian_leap?(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.leap?(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.ordinal(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.civil(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.commercial(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self._strptime(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.strptime(*_); end

  standard_method(
    {
      _: String,
      comp: Boolean
    },
    returns: Hash
  )
  def self._parse(_, comp=true); end

  standard_method({_: String}, returns: Hash)
  def self._iso8601(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.iso8601(*_); end

  standard_method({_: String}, returns: Hash)
  def self._rfc3339(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.rfc3339(*_); end

  standard_method({_: String}, returns: Hash)
  def self._xmlschema(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.xmlschema(*_); end

  standard_method({_: String}, returns: Hash)
  def self._rfc2822(_); end

  standard_method({_: String}, returns: Hash)
  def self._rfc822(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.rfc2822(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.rfc822(*_); end

  standard_method({_: String}, returns: Hash)
  def self._httpdate(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.httpdate(*_); end

  standard_method({_: String}, returns: Hash)
  def self._jisx0301(_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.jisx0301(*_); end
end

class DateTime
  standard_method({}, returns: Opus::Types.untyped)
  def min(); end

  standard_method({}, returns: Opus::Types.untyped)
  def to_s(); end

  standard_method({}, returns: Opus::Types.untyped)
  def offset(); end

  standard_method({}, returns: Opus::Types.untyped)
  def zone(); end

  standard_method({}, returns: Opus::Types.untyped)
  def sec(); end

  standard_method({}, returns: Opus::Types.untyped)
  def hour(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def strftime(*_); end

  standard_method({}, returns: Opus::Types.untyped)
  def second(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def iso8601(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def rfc3339(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def xmlschema(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def jisx0301(*_); end

  standard_method({}, returns: Opus::Types.untyped)
  def minute(); end

  standard_method({}, returns: Opus::Types.untyped)
  def sec_fraction(); end

  standard_method({}, returns: Opus::Types.untyped)
  def second_fraction(); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def new_offset(*_); end

  standard_method({}, returns: Time)
  def to_time(); end

  standard_method({}, returns: Date)
  def to_date(); end

  standard_method({}, returns: DateTime)
  def to_datetime(); end

  standard_method({}, returns: Opus::Types.untyped)
  def blank?(); end

  standard_method({}, returns: Opus::Types.untyped)
  def to_utc_time(); end

  standard_method(
    {
      locale: Opus::Types.untyped,
      options: Opus::Types.untyped,
    },
    returns: Opus::Types.untyped,
  )
  def localize(locale=_, options=_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.new(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.now(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.parse(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.jd(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.ordinal(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.civil(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.commercial(*_); end

  standard_method(
    {
      _: String,
      format: String
    },
    returns: Hash
  )
  def self._strptime(_, format="%F"); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.strptime(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.iso8601(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.rfc3339(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.xmlschema(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.rfc2822(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.rfc822(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.httpdate(*_); end

  standard_method({_: Opus::Types.untyped}, returns: Opus::Types.untyped)
  def self.jisx0301(*_); end
end

class Time
  standard_method({}, returns: Time)
  def to_time(); end

  standard_method({}, returns: Date)
  def to_date(); end

  standard_method({}, returns: DateTime)
  def to_datetime(); end
end
