# typed: true
module Base64
  sig do
    params(
        str: String,
    )
    .returns(String)
  end
  def self.decode64(str); end

  sig do
    params(
        bin: String,
    )
    .returns(String)
  end
  def self.encode64(bin); end

  sig do
    params(
        str: String,
    )
    .returns(String)
  end
  def self.strict_decode64(str); end

  sig do
    params(
        bin: String,
    )
    .returns(String)
  end
  def self.strict_encode64(bin); end

  sig do
    params(
        str: String,
    )
    .returns(String)
  end
  def self.urlsafe_decode64(str); end

  sig do
    params(
        bin: String,
        padding: T::Boolean,
    )
    .returns(String)
  end
  def self.urlsafe_encode64(bin, padding: true); end
end
