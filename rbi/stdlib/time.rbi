# typed: strict
class Time
  Sorbet.sig(
      date: String,
  )
  .returns(Time)
  def self.httpdate(date); end

  Sorbet.sig(
      date: String,
  )
  .returns(Time)
  def self.iso8601(date); end

  Sorbet.sig(
      date: String,
      now: Time,
  )
  .returns(Time)
  def self.parse(date, now=_); end

  Sorbet.sig(
      date: String,
  )
  .returns(Time)
  def self.rfc2822(date); end

  Sorbet.sig(
      date: String,
  )
  .returns(Time)
  def self.rfc822(date); end

  Sorbet.sig(
      date: String,
      format: String,
      now: Time,
  )
  .returns(Time)
  def self.strptime(date, format, now=_); end

  Sorbet.sig(
      date: String,
  )
  .returns(Time)
  def self.xmlschema(date); end

  Sorbet.sig(
      zone: String,
  )
  .returns(Time)
  def self.zone_offset(zone); end

  Sorbet.sig.returns(String)
  def httpdate(); end

  Sorbet.sig(
      fraction_digits: T.any(Integer, String),
  )
  .returns(String)
  def iso8601(fraction_digits=0); end

  Sorbet.sig.returns(String)
  def rfc2822(); end

  Sorbet.sig.returns(String)
  def rfc822(); end

  Sorbet.sig(
      fraction_digits: T.any(Integer, String),
  )
  .returns(String)
  def xmlschema(fraction_digits=0); end
end
