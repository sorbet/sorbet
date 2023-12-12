# typed: __STDLIB_INTERNAL

# This module provides a framework for message digest libraries.
#
# You may want to look at
# [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html)
# as it supports more algorithms.
#
# A cryptographic hash function is a procedure that takes data and returns a
# fixed bit string: the hash value, also known as *digest*.
# [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) functions are also
# called one-way functions, it is easy to compute a digest from a message, but
# it is infeasible to generate a message from a digest.
#
# ## Examples
#
# ```ruby
# require 'digest'
#
# # Compute a complete digest
# Digest::SHA256.digest 'message'       #=> "\xABS\n\x13\xE4Y..."
#
# sha256 = Digest::SHA256.new
# sha256.digest 'message'               #=> "\xABS\n\x13\xE4Y..."
#
# # Other encoding formats
# Digest::SHA256.hexdigest 'message'    #=> "ab530a13e459..."
# Digest::SHA256.base64digest 'message' #=> "q1MKE+RZFJgr..."
#
# # Compute digest by chunks
# md5 = Digest::MD5.new
# md5.update 'message1'
# md5 << 'message2'                     # << is an alias for update
#
# md5.hexdigest                         #=> "94af09c09bb9..."
#
# # Compute digest for a file
# sha256 = Digest::SHA256.file 'testfile'
# sha256.hexdigest
# ```
#
# Additionally digests can be encoded in "bubble babble" format as a sequence of
# consonants and vowels which is more recognizable and comparable than a
# hexadecimal digest.
#
# ```ruby
# require 'digest/bubblebabble'
#
# Digest::SHA256.bubblebabble 'message' #=> "xopoh-fedac-fenyh-..."
# ```
#
# See the bubble babble specification at
# http://web.mit.edu/kenta/www/one/bubblebabble/spec/jrtrjwzi/draft-huima-01.txt.
#
# ## [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) algorithms
#
# Different digest algorithms (or hash functions) are available:
#
# MD5
# :   See RFC 1321 The MD5 Message-Digest Algorithm
# RIPEMD-160
# :   As
#     [`Digest::RMD160`](https://docs.ruby-lang.org/en/2.7.0/Digest/RMD160.html).
#     See http://homes.esat.kuleuven.be/~bosselae/ripemd160.html.
# SHA1
# :   See FIPS 180 Secure
#     [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) Standard.
# SHA2 family
# :   See FIPS 180 Secure
#     [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) Standard which
#     defines the following algorithms:
#     *   SHA512
#     *   SHA384
#     *   SHA256
#
#
#
# The latest versions of the FIPS publications can be found here:
# http://csrc.nist.gov/publications/PubsFIPS.html.
module Digest
  # A mutex for Digest().
  REQUIRE_MUTEX = ::T.let(nil, ::T.untyped)

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.const_missing(name); end

  # Generates a hex-encoded version of a given *string*.
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexencode(_); end
end

# This abstract class provides a common interface to message digest
# implementation classes written in C.
#
# ## Write a [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) subclass in C
# [`Digest::Base`](https://docs.ruby-lang.org/en/2.7.0/Digest/Base.html)
# provides a common interface to message digest classes written in C. These
# classes must provide a struct of type rb\_digest\_metadata\_t:
#
# ```
# typedef int (*rb_digest_hash_init_func_t)(void *);
# typedef void (*rb_digest_hash_update_func_t)(void *, unsigned char *, size_t);
# typedef int (*rb_digest_hash_finish_func_t)(void *, unsigned char *);
#
# typedef struct {
#   int api_version;
#   size_t digest_len;
#   size_t block_len;
#   size_t ctx_size;
#   rb_digest_hash_init_func_t init_func;
#   rb_digest_hash_update_func_t update_func;
#   rb_digest_hash_finish_func_t finish_func;
# } rb_digest_metadata_t;
# ```
#
# This structure must be set as an instance variable named `metadata` (without
# the +@+ in front of the name). By example:
#
# ```
#  static const rb_digest_metadata_t sha1 = {
#     RUBY_DIGEST_API_VERSION,
#     SHA1_DIGEST_LENGTH,
#     SHA1_BLOCK_LENGTH,
#     sizeof(SHA1_CTX),
#     (rb_digest_hash_init_func_t)SHA1_Init,
#     (rb_digest_hash_update_func_t)SHA1_Update,
#     (rb_digest_hash_finish_func_t)SHA1_Finish,
# };
#
# rb_ivar_set(cDigest_SHA1, rb_intern("metadata"),
#             Data_Wrap_Struct(0, 0, 0, (void *)&sha1));
# ```
class Digest::Base < Digest::Class
  # Update the digest using given *string* and return `self`.
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(_); end

  # Return the block length of the digest in bytes.
  sig {returns(::T.untyped)}
  def block_length(); end

  # Return the length of the hash value in bytes.
  sig {returns(::T.untyped)}
  def digest_length(); end

  # Reset the digest to its initial state and return `self`.
  sig {returns(::T.untyped)}
  def reset(); end

  # Update the digest using given *string* and return `self`.
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(_); end
end

# This module stands as a base class for digest implementation classes.
class Digest::Class
  include ::Digest::Instance
  sig {void}
  def initialize(); end

  # Returns the base64 encoded hash value of a given *string*. The return value
  # is properly padded with '=' and contains no line feeds.
  sig do
    params(
      str: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.base64digest(str, *args); end

  # Returns the hash value of a given *string*. This is equivalent to
  # [`Digest::Class.new(*parameters)`](https://docs.ruby-lang.org/en/2.7.0/Digest/Instance.html#method-i-new).digest(string),
  # where extra *parameters*, if any, are passed through to the constructor and
  # the *string* is passed to digest().
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(*_); end

  # Creates a digest object and reads a given file, *name*. Optional arguments
  # are passed to the constructor of the digest class.
  #
  # ```ruby
  # p Digest::SHA256.file("X11R6.8.2-src.tar.bz2").hexdigest
  # # => "f02e3c85572dc9ad7cb77c2a638e3be24cc1b5bea9fdbb0b0299c9668475c534"
  # ```
  sig do
    params(
      name: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.file(name, *args); end

  # Returns the hex-encoded hash value of a given *string*. This is almost
  # equivalent to
  # [`Digest.hexencode`](https://docs.ruby-lang.org/en/2.7.0/Digest.html#method-c-hexencode)([`Digest::Class.new(*parameters)`](https://docs.ruby-lang.org/en/2.7.0/Digest/Instance.html#method-i-new).digest(string)).
  sig do
    params(
      _: String,
    )
    .returns(String)
  end
  def self.hexdigest(*_); end
end

# This module provides instance methods for a digest implementation object to
# calculate message digest values.
module Digest::Instance
  # Updates the digest using a given *string* and returns self.
  #
  # The update() method and the left-shift operator are overridden by each
  # implementation subclass. (One should be an alias for the other)
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(_); end

  # If a string is given, checks whether it is equal to the hex-encoded hash
  # value of the digest object. If another digest instance is given, checks
  # whether they have the same hash value. Otherwise returns false.
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(_); end

  # If none is given, returns the resulting hash value of the digest in a base64
  # encoded form, keeping the digest's state.
  #
  # If a `string` is given, returns the hash value for the given `string` in a
  # base64 encoded form, resetting the digest to the initial state before and
  # after the process.
  #
  # In either case, the return value is properly padded with '=' and contains no
  # line feeds.
  sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def base64digest(str=T.unsafe(nil)); end

  # Returns the resulting hash value and resets the digest to the initial state.
  sig {returns(::T.untyped)}
  def base64digest!(); end

  # Returns the block length of the digest.
  #
  # This method is overridden by each implementation subclass.
  sig {returns(::T.untyped)}
  def block_length(); end

  # If none is given, returns the resulting hash value of the digest, keeping
  # the digest's state.
  #
  # If a *string* is given, returns the hash value for the given *string*,
  # resetting the digest to the initial state before and after the process.
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def digest(*_); end

  # Returns the resulting hash value and resets the digest to the initial state.
  sig {returns(::T.untyped)}
  def digest!(); end

  # Returns the length of the hash value of the digest.
  #
  # This method should be overridden by each implementation subclass. If not,
  # digest\_obj.digest().length() is returned.
  sig {returns(::T.untyped)}
  def digest_length(); end

  # Updates the digest with the contents of a given file *name* and returns
  # self.
  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def file(name); end

  # If none is given, returns the resulting hash value of the digest in a
  # hex-encoded form, keeping the digest's state.
  #
  # If a *string* is given, returns the hash value for the given *string* in a
  # hex-encoded form, resetting the digest to the initial state before and after
  # the process.
  sig do
    params(
      _: String,
    )
    .returns(String)
  end
  def hexdigest(*_); end

  # Returns the resulting hash value in a hex-encoded form and resets the digest
  # to the initial state.
  sig {returns(::T.untyped)}
  def hexdigest!(); end

  # Creates a printable version of the digest object.
  sig {returns(::T.untyped)}
  def inspect(); end

  # Returns digest\_obj.digest\_length().
  sig {returns(::T.untyped)}
  def length(); end

  # Returns a new, initialized copy of the digest object. Equivalent to
  # digest\_obj.clone().reset().
  sig {returns(::T.untyped)}
  def new(); end

  # Resets the digest to the initial state and returns self.
  #
  # This method is overridden by each implementation subclass.
  sig {returns(::T.untyped)}
  def reset(); end

  # Returns digest\_obj.digest\_length().
  sig {returns(::T.untyped)}
  def size(); end

  # Returns digest\_obj.hexdigest().
  sig {returns(::T.untyped)}
  def to_s(); end

  # Updates the digest using a given *string* and returns self.
  #
  # The update() method and the left-shift operator are overridden by each
  # implementation subclass. (One should be an alias for the other)
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(_); end
end

# A class for calculating message digests using the
# [`MD5`](https://docs.ruby-lang.org/en/2.7.0/Digest/MD5.html) Message-Digest
# Algorithm by RSA [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html)
# Security, Inc., described in RFC1321.
#
# [`MD5`](https://docs.ruby-lang.org/en/2.7.0/Digest/MD5.html) calculates a
# digest of 128 bits (16 bytes).
#
# ## Examples
#
# ```ruby
# require 'digest'
#
# # Compute a complete digest
# Digest::MD5.hexdigest 'abc'      #=> "90015098..."
#
# # Compute digest by chunks
# md5 = Digest::MD5.new               # =>#<Digest::MD5>
# md5.update "ab"
# md5 << "c"                           # alias for #update
# md5.hexdigest                        # => "90015098..."
#
# # Use the same object to compute another digest
# md5.reset
# md5 << "message"
# md5.hexdigest                        # => "78e73102..."
# ```
class Digest::MD5 < Digest::Base
end

# A class for calculating message digests using the SHA-1 Secure
# [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) Algorithm by NIST (the
# US' National Institute of Standards and Technology), described in FIPS PUB
# 180-1.
#
# See
# [`Digest::Instance`](https://docs.ruby-lang.org/en/2.7.0/Digest/Instance.html)
# for digest API.
#
# SHA-1 calculates a digest of 160 bits (20 bytes).
#
# ## Examples
#
# ```ruby
# require 'digest'
#
# # Compute a complete digest
# Digest::SHA1.hexdigest 'abc'      #=> "a9993e36..."
#
# # Compute digest by chunks
# sha1 = Digest::SHA1.new               # =>#<Digest::SHA1>
# sha1.update "ab"
# sha1 << "c"                           # alias for #update
# sha1.hexdigest                        # => "a9993e36..."
#
# # Use the same object to compute another digest
# sha1.reset
# sha1 << "message"
# sha1.hexdigest                        # => "6f9b9af3..."
# ```
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
    .void
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
