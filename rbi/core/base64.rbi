# typed: true
module Base64
  sig(
      str: String,
  )
  .returns(String)
  def self.decode64(str); end

  sig(
      bin: String,
  )
  .returns(String)
  def self.encode64(bin); end

  sig(
      str: String,
  )
  .returns(String)
  def self.strict_decode64(str); end

  sig(
      bin: String,
  )
  .returns(String)
  def self.strict_encode64(bin); end

  sig(
      str: String,
  )
  .returns(String)
  def self.urlsafe_decode64(str); end

  sig(
      bin: String,
  )
  .returns(String)
  def self.urlsafe_encode64(bin); end
end
