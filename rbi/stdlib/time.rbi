# typed: strict
class Time
  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.httpdate(date); end

  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.iso8601(date); end

  sig do
    params(
        date: String,
        now: Time,
    )
    .returns(Time)
  end
  def self.parse(date, now=T.unsafe(nil)); end

  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.rfc2822(date); end

  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.rfc822(date); end

  sig do
    params(
        date: String,
        format: String,
        now: Time,
    )
    .returns(Time)
  end
  def self.strptime(date, format, now=T.unsafe(nil)); end

  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.xmlschema(date); end

  sig do
    params(
        zone: String,
    )
    .returns(Time)
  end
  def self.zone_offset(zone); end

  sig {returns(String)}
  def httpdate(); end

  sig do
    params(
        fraction_digits: T.any(Integer, String),
    )
    .returns(String)
  end
  def iso8601(fraction_digits=0); end

  sig {returns(String)}
  def rfc2822(); end

  sig {returns(String)}
  def rfc822(); end

  sig do
    params(
        fraction_digits: T.any(Integer, String),
    )
    .returns(String)
  end
  def xmlschema(fraction_digits=0); end
end
