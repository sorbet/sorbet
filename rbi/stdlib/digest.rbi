# typed: strict

module Digest
  REQUIRE_MUTEX = ::T.let(nil, ::T.untyped)

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.const_missing(name); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexencode(_); end
end

class Digest::Base < Digest::Class
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(_); end

  sig {returns(::T.untyped)}
  def block_length(); end

  sig {returns(::T.untyped)}
  def digest_length(); end

  sig {returns(::T.untyped)}
  def reset(); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(_); end
end

class Digest::Class
  include ::Digest::Instance
  sig {returns(::T.untyped)}
  def initialize(); end

  sig do
    params(
      str: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.base64digest(str, *args); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(*_); end

  sig do
    params(
      name: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.file(name, *args); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(*_); end
end

module Digest::Instance
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(_); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(_); end

  sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def base64digest(str=T.unsafe(nil)); end

  sig {returns(::T.untyped)}
  def base64digest!(); end

  sig {returns(::T.untyped)}
  def block_length(); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def digest(*_); end

  sig {returns(::T.untyped)}
  def digest!(); end

  sig {returns(::T.untyped)}
  def digest_length(); end

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def file(name); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def hexdigest(*_); end

  sig {returns(::T.untyped)}
  def hexdigest!(); end

  sig {returns(::T.untyped)}
  def inspect(); end

  sig {returns(::T.untyped)}
  def length(); end

  sig {returns(::T.untyped)}
  def new(); end

  sig {returns(::T.untyped)}
  def reset(); end

  sig {returns(::T.untyped)}
  def size(); end

  sig {returns(::T.untyped)}
  def to_s(); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(_); end
end

class Digest::MD5 < Digest::Base
end

class Digest::SHA1 < Digest::Base
end

class Digest::SHA2 < Digest::Class
  sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(str); end

  sig {returns(::T.untyped)}
  def block_length(); end

  sig {returns(::T.untyped)}
  def digest_length(); end

  sig do
    params(
      bitlen: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(bitlen=T.unsafe(nil)); end

  sig {returns(::T.untyped)}
  def inspect(); end

  sig {returns(::T.untyped)}
  def reset(); end

  sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(str); end
end

class Digest::SHA256 < Digest::Base
end

class Digest::SHA384 < Digest::Base
end

class Digest::SHA512 < Digest::Base
end
