# typed: __STDLIB_INTERNAL

# The [`Base64`](https://docs.ruby-lang.org/en/2.6.0/Base64.html) module
# provides for the encoding (#encode64,
# [`strict_encode64`](https://docs.ruby-lang.org/en/2.6.0/Base64.html#method-i-strict_encode64),
# [`urlsafe_encode64`](https://docs.ruby-lang.org/en/2.6.0/Base64.html#method-i-urlsafe_encode64))
# and decoding (#decode64,
# [`strict_decode64`](https://docs.ruby-lang.org/en/2.6.0/Base64.html#method-i-strict_decode64),
# [`urlsafe_decode64`](https://docs.ruby-lang.org/en/2.6.0/Base64.html#method-i-urlsafe_decode64))
# of binary data using a
# [`Base64`](https://docs.ruby-lang.org/en/2.6.0/Base64.html) representation.
#
# ## Example
#
# A simple encoding and decoding.
#
# ```ruby
# require "base64"
#
# enc   = Base64.encode64('Send reinforcements')
#                     # -> "U2VuZCByZWluZm9yY2VtZW50cw==\n"
# plain = Base64.decode64(enc)
#                     # -> "Send reinforcements"
# ```
#
# The purpose of using base64 to encode data is that it translates any binary
# data into purely printable characters.
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
