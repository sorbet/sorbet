# typed: strict
class Time
  sig(
      date: String,
  )
  .returns(Time)
  def self.httpdate(date); end

  sig(
      date: String,
  )
  .returns(Time)
  def self.iso8601(date); end

  sig(
      date: String,
      now: Time,
  )
  .returns(Time)
  def self.parse(date, now=_); end

  sig(
      date: String,
  )
  .returns(Time)
  def self.rfc2822(date); end

  sig(
      date: String,
  )
  .returns(Time)
  def self.rfc822(date); end

  sig(
      date: String,
      format: String,
      now: Time,
  )
  .returns(Time)
  def self.strptime(date, format, now=_); end

  sig(
      date: String,
  )
  .returns(Time)
  def self.xmlschema(date); end

  sig(
      zone: String,
  )
  .returns(Time)
  def self.zone_offset(zone); end

  sig.returns(String)
  def httpdate(); end

  sig(
      fraction_digits: T.any(Integer, String),
  )
  .returns(String)
  def iso8601(fraction_digits=0); end

  sig.returns(String)
  def rfc2822(); end

  sig.returns(String)
  def rfc822(); end

  sig(
      fraction_digits: T.any(Integer, String),
  )
  .returns(String)
  def xmlschema(fraction_digits=0); end
end
