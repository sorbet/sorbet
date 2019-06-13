# typed: __STDLIB_INTERNAL

class DateTime < Date
  sig {returns(T.untyped)}
  def min(); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def offset(); end

  sig {returns(T.untyped)}
  def zone(); end

  sig {returns(T.untyped)}
  def sec(); end

  sig {returns(T.untyped)}
  def hour(); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def strftime(*arg0); end

  sig {returns(T.untyped)}
  def second(); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def iso8601(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def rfc3339(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def xmlschema(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def jisx0301(*arg0); end

  sig {returns(T.untyped)}
  def minute(); end

  sig {returns(T.untyped)}
  def sec_fraction(); end

  sig {returns(T.untyped)}
  def second_fraction(); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def new_offset(*arg0); end

  sig {returns(Time)}
  def to_time(); end

  sig {returns(Date)}
  def to_date(); end

  sig {returns(DateTime)}
  def to_datetime(); end

  sig {returns(T.untyped)}
  def blank?(); end

  sig {returns(T.untyped)}
  def to_utc_time(); end

  sig do
    params(
      locale: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def localize(locale=T.unsafe(nil), options=T.unsafe(nil)); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.new(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.now(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.parse(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.jd(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.ordinal(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.civil(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.commercial(*arg0); end

  sig do
    params(
      arg0: String,
      format: String
    )
    .returns(Hash)
  end
  def self._strptime(arg0, format="%F"); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.strptime(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.iso8601(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.rfc3339(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.xmlschema(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.rfc2822(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.rfc822(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.httpdate(*arg0); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.jisx0301(*arg0); end
end

