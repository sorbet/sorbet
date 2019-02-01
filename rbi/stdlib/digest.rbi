# typed: strict

module Digest
  REQUIRE_MUTEX = ::T.let(nil, ::T.untyped)

  Sorbet.sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.const_missing(name); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexencode(_); end
end

class Digest::Base < Digest::Class
  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(_); end

  Sorbet.sig {returns(::T.untyped)}
  def block_length(); end

  Sorbet.sig {returns(::T.untyped)}
  def digest_length(); end

  Sorbet.sig {returns(::T.untyped)}
  def reset(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(_); end
end

class Digest::Class
  include ::Digest::Instance
  Sorbet.sig {returns(::T.untyped)}
  def initialize(); end

  Sorbet.sig do
    params(
      str: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.base64digest(str, *args); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(*_); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.file(name, *args); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(*_); end
end

module Digest::Instance
  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(_); end

  Sorbet.sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def base64digest(str=T.unsafe(nil)); end

  Sorbet.sig {returns(::T.untyped)}
  def base64digest!(); end

  Sorbet.sig {returns(::T.untyped)}
  def block_length(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def digest(*_); end

  Sorbet.sig {returns(::T.untyped)}
  def digest!(); end

  Sorbet.sig {returns(::T.untyped)}
  def digest_length(); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def file(name); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def hexdigest(*_); end

  Sorbet.sig {returns(::T.untyped)}
  def hexdigest!(); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def length(); end

  Sorbet.sig {returns(::T.untyped)}
  def new(); end

  Sorbet.sig {returns(::T.untyped)}
  def reset(); end

  Sorbet.sig {returns(::T.untyped)}
  def size(); end

  Sorbet.sig {returns(::T.untyped)}
  def to_s(); end

  Sorbet.sig do
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
  Sorbet.sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(str); end

  Sorbet.sig {returns(::T.untyped)}
  def block_length(); end

  Sorbet.sig {returns(::T.untyped)}
  def digest_length(); end

  Sorbet.sig do
    params(
      bitlen: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(bitlen=T.unsafe(nil)); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def reset(); end

  Sorbet.sig do
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
