# typed: true
module Base64
  Sorbet.sig(
      str: String,
  )
  .returns(String)
  def self.decode64(str); end

  Sorbet.sig(
      bin: String,
  )
  .returns(String)
  def self.encode64(bin); end

  Sorbet.sig(
      str: String,
  )
  .returns(String)
  def self.strict_decode64(str); end

  Sorbet.sig(
      bin: String,
  )
  .returns(String)
  def self.strict_encode64(bin); end

  Sorbet.sig(
      str: String,
  )
  .returns(String)
  def self.urlsafe_decode64(str); end

  Sorbet.sig(
      bin: String,
      padding: T.any(TrueClass, FalseClass),
  )
  .returns(String)
  def self.urlsafe_encode64(bin, padding: true); end
end
