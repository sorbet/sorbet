# typed: __STDLIB_INTERNAL

module Kernel
  # Returns `uri` converted to an
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) object.
  sig {params(uri: T.any(URI::Generic, String)).returns(URI::Generic)}
  def URI(uri); end
end

# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) is a module providing
# classes to handle Uniform Resource Identifiers
# ([RFC2396](http://tools.ietf.org/html/rfc2396)).
#
# ## Features
#
# *   Uniform way of handling URIs.
# *   Flexibility to introduce custom
#     [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) schemes.
# *   Flexibility to have an alternate
#     [`URI::Parser`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_Parser.html)
#     (or just different patterns and regexp's).
#
#
# ## Basic example
#
# ```ruby
# require 'uri'
#
# uri = URI("http://foo.com/posts?id=30&limit=5#time=1305298413")
# #=> #<URI::HTTP http://foo.com/posts?id=30&limit=5#time=1305298413>
#
# uri.scheme    #=> "http"
# uri.host      #=> "foo.com"
# uri.path      #=> "/posts"
# uri.query     #=> "id=30&limit=5"
# uri.fragment  #=> "time=1305298413"
#
# uri.to_s      #=> "http://foo.com/posts?id=30&limit=5#time=1305298413"
# ```
#
# ## Adding custom URIs
#
# ```ruby
# module URI
#   class RSYNC < Generic
#     DEFAULT_PORT = 873
#   end
#   @@schemes['RSYNC'] = RSYNC
# end
# #=> URI::RSYNC
#
# URI.scheme_list
# #=> {"FILE"=>URI::File, "FTP"=>URI::FTP, "HTTP"=>URI::HTTP,
# #    "HTTPS"=>URI::HTTPS, "LDAP"=>URI::LDAP, "LDAPS"=>URI::LDAPS,
# #    "MAILTO"=>URI::MailTo, "RSYNC"=>URI::RSYNC}
#
# uri = URI("rsync://rsync.foo.com")
# #=> #<URI::RSYNC rsync://rsync.foo.com>
# ```
#
# ## RFC References
#
# A good place to view an RFC spec is http://www.ietf.org/rfc.html.
#
# Here is a list of all related RFC's:
# *   [RFC822](http://tools.ietf.org/html/rfc822)
# *   [RFC1738](http://tools.ietf.org/html/rfc1738)
# *   [RFC2255](http://tools.ietf.org/html/rfc2255)
# *   [RFC2368](http://tools.ietf.org/html/rfc2368)
# *   [RFC2373](http://tools.ietf.org/html/rfc2373)
# *   [RFC2396](http://tools.ietf.org/html/rfc2396)
# *   [RFC2732](http://tools.ietf.org/html/rfc2732)
# *   [RFC3986](http://tools.ietf.org/html/rfc3986)
#
#
# ## [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) tree
#
# *   [`URI::Generic`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html) (in
#     uri/generic.rb)
#     *   [`URI::File`](https://docs.ruby-lang.org/en/2.7.0/URI/File.html) - (in
#         uri/file.rb)
#     *   [`URI::FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html) - (in
#         uri/ftp.rb)
#     *   [`URI::HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html) - (in
#         uri/http.rb)
#         *   [`URI::HTTPS`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTPS.html)
#             - (in uri/https.rb)
#
#     *   [`URI::LDAP`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAP.html) - (in
#         uri/ldap.rb)
#         *   [`URI::LDAPS`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAPS.html)
#             - (in uri/ldaps.rb)
#
#     *   [`URI::MailTo`](https://docs.ruby-lang.org/en/2.7.0/URI/MailTo.html) -
#         (in uri/mailto.rb)
#
# *   [`URI::Parser`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_Parser.html)
#     - (in uri/common.rb)
# *   [`URI::REGEXP`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_REGEXP.html)
#     - (in uri/common.rb)
#     *   URI::REGEXP::PATTERN - (in uri/common.rb)
#
# *   URI::Util - (in uri/common.rb)
# *   [`URI::Escape`](https://docs.ruby-lang.org/en/2.7.0/URI/Escape.html) - (in
#     uri/common.rb)
# *   [`URI::Error`](https://docs.ruby-lang.org/en/2.7.0/URI/Error.html) - (in
#     uri/common.rb)
#     *   [`URI::InvalidURIError`](https://docs.ruby-lang.org/en/2.7.0/URI/InvalidURIError.html)
#         - (in uri/common.rb)
#     *   [`URI::InvalidComponentError`](https://docs.ruby-lang.org/en/2.7.0/URI/InvalidComponentError.html)
#         - (in uri/common.rb)
#     *   [`URI::BadURIError`](https://docs.ruby-lang.org/en/2.7.0/URI/BadURIError.html)
#         - (in uri/common.rb)
#
#
#
# ## Copyright Info
#
# Author
# :   Akira Yamada <akira@ruby-lang.org>
# Documentation
# :   Akira Yamada <akira@ruby-lang.org> Dmitry V. Sabanin <sdmitry@lrn.ru>
#     Vincent Batts <vbatts@hashbangbash.com>
# License
# :   Copyright (c) 2001 akira yamada <akira@ruby-lang.org> You can redistribute
#     it and/or modify it under the same term as Ruby.
# Revision
# :   $Id$
module URI
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  # [`URI::Parser.new`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_Parser.html#method-c-new)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
  ESCAPED = T.let(T.unsafe(nil), Regexp)
  FRAGMENT = T.let(T.unsafe(nil), Regexp)
  HOST = T.let(T.unsafe(nil), Regexp)
  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
  OPAQUE = T.let(T.unsafe(nil), Regexp)
  Parser = URI::RFC2396_Parser
  PORT = T.let(T.unsafe(nil), Regexp)
  QUERY = T.let(T.unsafe(nil), Regexp)
  REGEXP = URI::RFC2396_REGEXP
  REGISTRY = T.let(T.unsafe(nil), Regexp)
  REL_PATH = T.let(T.unsafe(nil), Regexp)
  REL_URI = T.let(T.unsafe(nil), Regexp)
  REL_URI_REF = T.let(T.unsafe(nil), Regexp)
  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
  SCHEME = T.let(T.unsafe(nil), Regexp)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Decodes URL-encoded form data from given `str`.
  #
  # This decodes application/x-www-form-urlencoded data and returns an array of
  # key-value arrays.
  #
  # This refers http://url.spec.whatwg.org/#concept-urlencoded-parser, so this
  # supports only &-separator, and doesn't support ;-separator.
  #
  # ```ruby
  # ary = URI.decode_www_form("a=1&a=2&b=3")
  # ary                   #=> [['a', '1'], ['a', '2'], ['b', '3']]
  # ary.assoc('a').last   #=> '1'
  # ary.assoc('b').last   #=> '3'
  # ary.rassoc('a').last  #=> '2'
  # Hash[ary]             #=> {"a"=>"2", "b"=>"3"}
  # ```
  #
  # See
  # [`URI.decode_www_form_component`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-decode_www_form_component),
  # [`URI.encode_www_form`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-encode_www_form).
  sig do
    params(
      str: String,
      enc: Encoding,
      separator: String,
      use__charset_: T::Boolean,
      isindex: T::Boolean
    ).returns(T::Array[[String, String]])
  end
  def self.decode_www_form(str, enc = Encoding::UTF_8, separator: '&', use__charset_: false, isindex: false); end

  # Decodes given `str` of URL-encoded form data.
  #
  # This decodes + to SP.
  #
  # See
  # [`URI.encode_www_form_component`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-encode_www_form_component),
  # [`URI.decode_www_form`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-decode_www_form).
  sig do
    params(
        str: String,
        enc: Encoding,
    )
    .returns(String)
  end
  def self.decode_www_form_component(str, enc=Encoding::UTF_8); end

  # Generates URL-encoded form data from given `enum`.
  #
  # This generates application/x-www-form-urlencoded data defined in HTML5 from
  # given an [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html)
  # object.
  #
  # This internally uses
  # [`URI.encode_www_form_component(str)`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-encode_www_form_component).
  #
  # This method doesn't convert the encoding of given items, so convert them
  # before calling this method if you want to send data as other than original
  # encoding or mixed encoding data. (Strings which are encoded in an HTML5
  # ASCII incompatible encoding are converted to UTF-8.)
  #
  # This method doesn't handle files. When you send a file, use
  # multipart/form-data.
  #
  # This refers http://url.spec.whatwg.org/#concept-urlencoded-serializer
  #
  # ```ruby
  # URI.encode_www_form([["q", "ruby"], ["lang", "en"]])
  # #=> "q=ruby&lang=en"
  # URI.encode_www_form("q" => "ruby", "lang" => "en")
  # #=> "q=ruby&lang=en"
  # URI.encode_www_form("q" => ["ruby", "perl"], "lang" => "en")
  # #=> "q=ruby&q=perl&lang=en"
  # URI.encode_www_form([["q", "ruby"], ["q", "perl"], ["lang", "en"]])
  # #=> "q=ruby&q=perl&lang=en"
  # ```
  #
  # See
  # [`URI.encode_www_form_component`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-encode_www_form_component),
  # [`URI.decode_www_form`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-decode_www_form).
  sig do
    params(
      enum: T::Enumerable[Object],
      enc: T.nilable(Encoding)
    ).returns(String)
  end
  def self.encode_www_form(enum, enc=nil); end

  # Encodes given `str` to URL-encoded form data.
  #
  # This method doesn't convert \*, -, ., 0-9, A-Z, \_, a-z, but does convert SP
  # (ASCII space) to + and converts others to %XX.
  #
  # If `enc` is given, convert `str` to the encoding before percent encoding.
  #
  # This is an implementation of
  # http://www.w3.org/TR/2013/CR-html5-20130806/forms.html#url-encoded-form-data.
  #
  # See
  # [`URI.decode_www_form_component`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-decode_www_form_component),
  # [`URI.encode_www_form`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-encode_www_form).
  sig do
    params(
      str: Object,
      enc: T.nilable(Encoding)
    ).returns(String)
  end
  def self.encode_www_form_component(str, enc=nil); end

  sig do
    params(
        arg: String,
        arg0: Regexp,
    )
    .returns(String)
  end
  sig do
    params(
        arg: String,
        arg0: String,
    )
    .returns(String)
  end
  def self.escape(arg, *arg0); end

  # ## Synopsis
  #
  # ```
  # URI::extract(str[, schemes][,&blk])
  # ```
  #
  # ## Args
  #
  # `str`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to extract
  #     URIs from.
  # `schemes`
  # :   Limit [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) matching to
  #     specific schemes.
  #
  #
  # ## Description
  #
  # Extracts URIs from a string. If block given, iterates through all matched
  # URIs. Returns nil if block given or array with matches.
  #
  # ## Usage
  #
  # ```ruby
  # require "uri"
  #
  # URI.extract("text here http://foo.example.org/bla and here mailto:test@example.com and here also.")
  # # => ["http://foo.example.com/bla", "mailto:test@example.com"]
  # ```
  sig do
    params(
        str: String,
        schemes: T::Array[T.untyped],
        blk: BasicObject,
    )
    .returns(T::Array[String])
  end
  def self.extract(str, schemes=T.unsafe(nil), &blk); end

  # ## Synopsis
  #
  # ```
  # URI::join(str[, str, ...])
  # ```
  #
  # ## Args
  #
  # `str`
  # :   String(s) to work with, will be converted to RFC3986 URIs before
  #     merging.
  #
  #
  # ## Description
  #
  # Joins URIs.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # URI.join("http://example.com/","main.rbx")
  # # => #<URI::HTTP http://example.com/main.rbx>
  #
  # URI.join('http://example.com', 'foo')
  # # => #<URI::HTTP http://example.com/foo>
  #
  # URI.join('http://example.com', '/foo', '/bar')
  # # => #<URI::HTTP http://example.com/bar>
  #
  # URI.join('http://example.com', '/foo', 'bar')
  # # => #<URI::HTTP http://example.com/bar>
  #
  # URI.join('http://example.com', '/foo/', 'bar')
  # # => #<URI::HTTP http://example.com/foo/bar>
  # ```
  sig do
    params(
        str: T.any(URI::Generic, String),
    )
    .returns(URI::Generic)
  end
  def self.join(*str); end

  # ## Synopsis
  #
  # ```ruby
  # URI::parse(uri_str)
  # ```
  #
  # ## Args
  #
  # `uri_str`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) with
  #     [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  #
  # ## Description
  #
  # Creates one of the URI's subclasses instance from the string.
  #
  # ## Raises
  #
  # [`URI::InvalidURIError`](https://docs.ruby-lang.org/en/2.7.0/URI/InvalidURIError.html)
  # :   Raised if [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) given is
  #     not a correct one.
  #
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://www.ruby-lang.org/")
  # # => #<URI::HTTP http://www.ruby-lang.org/>
  # uri.scheme
  # # => "http"
  # uri.host
  # # => "www.ruby-lang.org"
  # ```
  #
  # It's recommended to first ::escape the provided `uri_str` if there are any
  # invalid [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) characters.
  sig do
    params(
        uri: String,
    )
    .returns(URI::Generic)
  end
  def self.parse(uri); end

  # ## Synopsis
  #
  # ```ruby
  # URI::regexp([match_schemes])
  # ```
  #
  # ## Args
  #
  # `match_schemes`
  # :   [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of schemes. If
  #     given, resulting regexp matches to URIs whose scheme is one of the
  #     match\_schemes.
  #
  #
  # ## Description
  #
  # Returns a [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) object
  # which matches to URI-like strings. The
  # [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) object returned
  # by this method includes arbitrary number of capture group (parentheses).
  # Never rely on it's number.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # # extract first URI from html_string
  # html_string.slice(URI.regexp)
  #
  # # remove ftp URIs
  # html_string.sub(URI.regexp(['ftp']), '')
  #
  # # You should not rely on the number of parentheses
  # html_string.scan(URI.regexp) do |*matches|
  #   p $&
  # end
  # ```
  sig do
    params(
        schemes: T::Array[T.untyped],
    )
    .returns(Regexp)
  end
  def self.regexp(schemes=T.unsafe(nil)); end

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of the
  # defined schemes.
  sig {returns(T::Hash[String, Class])}
  def self.scheme_list(); end

  # ## Synopsis
  #
  # ```ruby
  # URI::split(uri)
  # ```
  #
  # ## Args
  #
  # `uri`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) with
  #     [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  #
  # ## Description
  #
  # Splits the string on following parts and returns array with result:
  #
  # *   Scheme
  # *   Userinfo
  # *   Host
  # *   Port
  # *   Registry
  # *   Path
  # *   Opaque
  # *   Query
  # *   Fragment
  #
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # URI.split("http://www.ruby-lang.org/")
  # # => ["http", nil, "www.ruby-lang.org", nil, nil, "/", nil, nil, nil]
  # ```
  sig do
    params(
        uri: String,
    )
    .returns(T::Array[T.nilable(String)])
  end
  def self.split(uri); end

  sig do
    params(
        arg: String,
    )
    .returns(String)
  end
  def self.unescape(*arg); end

  sig do
    params(
        arg: String,
        arg0: Regexp,
    )
    .returns(String)
  end
  sig do
    params(
        arg: String,
        arg0: String,
    )
    .returns(String)
  end
  def self.encode(arg, *arg0); end

  sig do
    params(
        arg: String,
    )
    .returns(String)
  end
  def self.decode(*arg); end
end

# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) is valid, bad usage is
# not.
class URI::BadURIError < URI::Error
end

# Base class for all [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
# exceptions.
class URI::Error < StandardError
end

# [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) for escaping
# unsafe characters with codes.
module URI::Escape
  # Alias for:
  # [`unescape`](https://docs.ruby-lang.org/en/2.7.0/URI/Escape.html#method-i-unescape)
  def decode(*arg); end

  # Alias for:
  # [`escape`](https://docs.ruby-lang.org/en/2.7.0/URI/Escape.html#method-i-escape)
  def encode(*arg); end

  # ## Synopsis
  #
  # ```
  # URI.escape(str [, unsafe])
  # ```
  #
  # ## Args
  #
  # `str`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to replaces
  #     in.
  # `unsafe`
  # :   [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) that matches
  #     all symbols that must be replaced with codes. By default uses `UNSAFE`.
  #     When this argument is a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), it
  #     represents a character set.
  #
  #
  # ## Description
  #
  # Escapes the string, replacing all unsafe characters with codes.
  #
  # This method is obsolete and should not be used. Instead, use
  # [`CGI.escape`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escape),
  # [`URI.encode_www_form`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-encode_www_form)
  # or
  # [`URI.encode_www_form_component`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-encode_www_form_component)
  # depending on your specific use case.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # enc_uri = URI.escape("http://example.com/?a=\11\15")
  # # => "http://example.com/?a=%09%0D"
  #
  # URI.unescape(enc_uri)
  # # => "http://example.com/?a=\t\r"
  #
  # URI.escape("@?@!", "!?")
  # # => "@%3F@%21"
  # ```
  #
  #
  # Also aliased as:
  # [`encode`](https://docs.ruby-lang.org/en/2.7.0/URI/Escape.html#method-i-encode)
  def escape(*arg); end

  # ## Synopsis
  #
  # ```ruby
  # URI.unescape(str)
  # ```
  #
  # ## Args
  #
  # `str`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to unescape.
  #
  #
  # ## Description
  #
  # This method is obsolete and should not be used. Instead, use
  # [`CGI.unescape`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-unescape),
  # [`URI.decode_www_form`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-decode_www_form)
  # or
  # [`URI.decode_www_form_component`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-decode_www_form_component)
  # depending on your specific use case.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # enc_uri = URI.escape("http://example.com/?a=\11\15")
  # # => "http://example.com/?a=%09%0D"
  #
  # URI.unescape(enc_uri)
  # # => "http://example.com/?a=\t\r"
  # ```
  #
  #
  # Also aliased as:
  # [`decode`](https://docs.ruby-lang.org/en/2.7.0/URI/Escape.html#method-i-decode)
  def unescape(*arg); end

end

# [`FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html)
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) syntax is defined by
# RFC1738 section 3.2.
#
# This class will be redesigned because of difference of implementations; the
# structure of its path. draft-hoffman-ftp-uri-04 is a draft but it is a good
# summary about the de facto spec.
# http://tools.ietf.org/html/draft-hoffman-ftp-uri-04
class URI::FTP < URI::Generic
  include ::OpenURI::OpenRead

  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  # An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the
  # available components for
  # [`URI::FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html).
  COMPONENT = T.let(T.unsafe(nil), T::Array[T.untyped])
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
  # A Default port of 21 for
  # [`URI::FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html).
  DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
  ESCAPED = T.let(T.unsafe(nil), Regexp)
  FRAGMENT = T.let(T.unsafe(nil), Regexp)
  HOST = T.let(T.unsafe(nil), Regexp)
  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
  OPAQUE = T.let(T.unsafe(nil), Regexp)
  PORT = T.let(T.unsafe(nil), Regexp)
  QUERY = T.let(T.unsafe(nil), Regexp)
  REGISTRY = T.let(T.unsafe(nil), Regexp)
  REL_PATH = T.let(T.unsafe(nil), Regexp)
  REL_URI = T.let(T.unsafe(nil), Regexp)
  REL_URI_REF = T.let(T.unsafe(nil), Regexp)
  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
  SCHEME = T.let(T.unsafe(nil), Regexp)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # Typecode is "a", "i", or "d".
  #
  # *   "a" indicates a text file (the
  #     [`FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html) command was
  #     ASCII)
  # *   "i" indicates a binary file
  #     ([`FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html) command
  #     IMAGE)
  # *   "d" indicates the contents of a directory should be displayed
  TYPECODE = T.let(T.unsafe(nil), T::Array[T.untyped])
  # Typecode prefix ";type=".
  TYPECODE_PREFIX = T.let(T.unsafe(nil), String)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # ## Description
  #
  # Creates a new [`URI::FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html)
  # object from generic URL components with no syntax checking.
  #
  # Unlike build(), this method does not escape the path component as required
  # by RFC1738; instead it is treated as per RFC2396.
  #
  # Arguments are `scheme`, `userinfo`, `host`, `port`, `registry`, `path`,
  # `opaque`, `query`, and `fragment`, in that order.
  def self.new(scheme, userinfo, host, port, registry, path, opaque, query, fragment, parser = _, arg_check = _); end

  def merge(oth); end

  # Returns the path from an
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html)
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # RFC 1738 specifically states that the path for an
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html)
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) does not include the /
  # which separates the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
  # path from the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) host.
  # Example:
  #
  # `ftp://ftp.example.com/pub/ruby`
  #
  # The above [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) indicates
  # that the client should connect to ftp.example.com then cd to pub/ruby from
  # the initial login directory.
  #
  # If you want to cd to an absolute directory, you must include an escaped /
  # (%2F) in the path. Example:
  #
  # `ftp://ftp.example.com/%2Fpub/ruby`
  #
  # This method will then return "/pub/ruby".
  def path; end

  # Returns a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # representation of the
  # [`URI::FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html).
  def to_s; end

  # typecode accessor.
  #
  # See
  # [`URI::FTP::COMPONENT`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html#COMPONENT).
  def typecode; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the typecode `v` (with validation).
  #
  # See also
  # [`URI::FTP.check_typecode`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html#method-i-check_typecode).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("ftp://john@ftp.example.com/my_file.img")
  # #=> #<URI::FTP ftp://john@ftp.example.com/my_file.img>
  # uri.typecode = "i"
  # uri
  # #=> #<URI::FTP ftp://john@ftp.example.com/my_file.img;type=i>
  # ```
  def typecode=(typecode); end

  protected

  # Private setter for the path of the
  # [`URI::FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html).
  def set_path(v); end

  # Private setter for the typecode `v`.
  #
  # See also
  # [`URI::FTP.typecode=`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html#method-i-typecode-3D).
  def set_typecode(v); end

  # ## Description
  #
  # Creates a new [`URI::FTP`](https://docs.ruby-lang.org/en/2.7.0/URI/FTP.html)
  # object from components, with syntax checking.
  #
  # The components accepted are `userinfo`, `host`, `port`, `path`, and
  # `typecode`.
  #
  # The components should be provided either as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html), or as a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) with keys formed by
  # preceding the component names with a colon.
  #
  # If an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) is used, the
  # components must be passed in the order `[userinfo, host, port, path,
  # typecode]`.
  #
  # If the path supplied is absolute, it will be escaped in order to make it
  # absolute in the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # Examples:
  #
  # ```ruby
  # require 'uri'
  #
  # uri1 = URI::FTP.build(['user:password', 'ftp.example.com', nil,
  #   '/path/file.zip', 'i'])
  # uri1.to_s  # => "ftp://user:password@ftp.example.com/%2Fpath/file.zip;type=i"
  #
  # uri2 = URI::FTP.build({:host => 'ftp.example.com',
  #   :path => 'ruby/src'})
  # uri2.to_s  # => "ftp://ftp.example.com/ruby/src"
  # ```
  def self.build(args); end

end

# Base class for all [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
# classes. Implements generic
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) syntax as per RFC 2396.
class URI::Generic < Object
  include URI
  include URI::RFC2396_REGEXP

  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  # An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the
  # available components for
  # [`URI::Generic`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html).
  COMPONENT = T.let(T.unsafe(nil), T::Array[T.untyped])
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
  # A Default port of nil for
  # [`URI::Generic`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html).
  DEFAULT_PORT = T.let(T.unsafe(nil), NilClass)
  ESCAPED = T.let(T.unsafe(nil), Regexp)
  FRAGMENT = T.let(T.unsafe(nil), Regexp)
  HOST = T.let(T.unsafe(nil), Regexp)
  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
  OPAQUE = T.let(T.unsafe(nil), Regexp)
  PORT = T.let(T.unsafe(nil), Regexp)
  QUERY = T.let(T.unsafe(nil), Regexp)
  REGISTRY = T.let(T.unsafe(nil), Regexp)
  REL_PATH = T.let(T.unsafe(nil), Regexp)
  REL_URI = T.let(T.unsafe(nil), Regexp)
  REL_URI_REF = T.let(T.unsafe(nil), Regexp)
  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
  SCHEME = T.let(T.unsafe(nil), Regexp)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Alias for:
  # [`merge`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-merge)
  sig {params(oth: T.untyped).returns(T.untyped)}
  def +(oth); end

  # Alias for:
  # [`route_from`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-route_from)
  sig {params(oth: T.untyped).returns(T.untyped)}
  def -(oth); end

  # Compares two URIs.
  sig {params(oth: T.untyped).returns(T::Boolean)}
  def ==(oth); end

  # Alias for:
  # [`absolute?`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-absolute-3F)
  sig {returns(T::Boolean)}
  def absolute; end

  # Returns true if [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) has a
  # scheme (e.g. http:// or https://) specified.
  #
  # Also aliased as:
  # [`absolute`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-absolute)
  sig {returns(T::Boolean)}
  def absolute?; end

  # ## Args
  #
  # `v`
  # :   [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) or
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Attempts to parse other
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) `oth`, returns
  # [parsed\_oth, self].
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com")
  # uri.coerce("http://foo.com")
  # #=> [#<URI::HTTP http://foo.com>, #<URI::HTTP http://my.example.com>]
  # ```
  sig {params(oth: T.any(URI::Generic, String)).returns(T.untyped)}
  def coerce(oth); end

  # Components of the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) in
  # the order.
  sig {returns(T.untyped)}
  def component; end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the
  # components defined from the
  # [`COMPONENT`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#COMPONENT)
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  sig {returns(T.untyped)}
  def component_ary; end

  # Returns default port.
  sig {returns(T.untyped)}
  def default_port; end

  sig {params(oth: T.untyped).returns(T::Boolean)}
  def eql?(oth); end

  # Returns a proxy [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html). The
  # proxy [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) is obtained from
  # environment variables such as http\_proxy, ftp\_proxy, no\_proxy, etc. If
  # there is no proper proxy, nil is returned.
  #
  # If the optional parameter `env` is specified, it is used instead of
  # [`ENV`](https://docs.ruby-lang.org/en/2.7.0/ENV.html).
  #
  # Note that capitalized variables (HTTP\_PROXY, FTP\_PROXY, NO\_PROXY, etc.)
  # are examined, too.
  #
  # But http\_proxy and HTTP\_PROXY is treated specially under
  # [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) environment. It's
  # because HTTP\_PROXY may be set by Proxy: header. So HTTP\_PROXY is not used.
  # http\_proxy is not used too if the variable is case insensitive.
  # CGI\_HTTP\_PROXY can be used instead.
  sig {params(env: T.untyped).returns(T.untyped)}
  def find_proxy(env = T.unsafe(nil)); end

  # Returns the fragment component of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ```ruby
  # URI("http://foo/bar/baz?search=FooBar#ponies").fragment #=> "ponies"
  # ```
  sig {returns(T.nilable(String))}
  def fragment; end

  # Checks the fragment `v` component against the
  # [`URI::Parser`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_Parser.html)
  # [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) for :FRAGMENT.
  #
  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the fragment component `v` (with validation).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com/?id=25#time=1305212049")
  # uri.fragment = "time=1305212086"
  # uri.to_s  #=> "http://my.example.com/?id=25#time=1305212086"
  # ```
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def fragment=(v); end

  # Returns true if [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) is
  # hierarchical.
  #
  # ## Description
  #
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) has components listed
  # in order of decreasing significance from left to right, see RFC3986
  # https://tools.ietf.org/html/rfc3986 1.2.3.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com/")
  # uri.hierarchical?
  # #=> true
  # uri = URI.parse("mailto:joe@example.com")
  # uri.hierarchical?
  # #=> false
  # ```
  sig {returns(T::Boolean)}
  def hierarchical?; end

  # Returns the host component of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ```ruby
  # URI("http://foo/bar/baz").host #=> "foo"
  # ```
  #
  # It returns nil if no host component exists.
  #
  # ```ruby
  # URI("mailto:foo@example.org").host #=> nil
  # ```
  #
  # The component does not contain the port number.
  #
  # ```ruby
  # URI("http://foo:8080/bar/baz").host #=> "foo"
  # ```
  #
  # Since IPv6 addresses are wrapped with brackets in URIs, this method returns
  # IPv6 addresses wrapped with brackets. This form is not appropriate to pass
  # to socket methods such as
  # [`TCPSocket.open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open).
  # If unwrapped host names are required, use the
  # [`hostname`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-hostname)
  # method.
  #
  # ```ruby
  # URI("http://[::1]/bar/baz").host     #=> "[::1]"
  # URI("http://[::1]/bar/baz").hostname #=> "::1"
  # ```
  sig {returns(T.nilable(String))}
  def host; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the host component `v` (with validation).
  #
  # See also
  # [`URI::Generic.check_host`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-check_host).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com")
  # uri.host = "foo.com"
  # uri.to_s  #=> "http://foo.com"
  # ```
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def host=(v); end

  # Extract the host part of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) and unwrap brackets
  # for IPv6 addresses.
  #
  # This method is the same as
  # [`URI::Generic#host`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#attribute-i-host)
  # except brackets for IPv6 (and future IP) addresses are removed.
  #
  # ```ruby
  # uri = URI("http://[::1]/bar")
  # uri.hostname      #=> "::1"
  # uri.host          #=> "[::1]"
  # ```
  sig {returns(T.nilable(String))}
  def hostname; end

  # Sets the host part of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) as the argument with
  # brackets for IPv6 addresses.
  #
  # This method is the same as
  # [`URI::Generic#host=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-host-3D)
  # except the argument can be a bare IPv6 address.
  #
  # ```ruby
  # uri = URI("http://foo/bar")
  # uri.hostname = "::1"
  # uri.to_s  #=> "http://[::1]/bar"
  # ```
  #
  # If the argument seems to be an IPv6 address, it is wrapped with brackets.
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def hostname=(v); end

  # == Args
  # `scheme`::
  #   Protocol scheme, i.e. 'http','ftp','mailto' and so on.
  # `userinfo`::
  #   User name and password, i.e. 'sdmitry:bla'.
  # `host`::
  #   Server host name.
  # `port`::
  #   Server port.
  # `registry`::
  #   Registry of naming authorities.
  # `path`::
  #   Path on server.
  # `opaque`::
  #   Opaque part.
  # `query`::
  #   Query data.
  # `fragment`::
  #   Part of the URI after '#' character.
  # `parser`::
  #   Parser for internal use [URI::DEFAULT_PARSER by default].
  # `arg_check`::
  #   Check arguments [false by default].
  #
  # Creates a new URI::Generic instance from `generic` components without check.
  sig do
    params(
      scheme: T.nilable(String),
      userinfo: T.nilable(String),
      host: T.nilable(String),
      port: T.nilable(Integer),
      registry: T.untyped,
      path: T.nilable(String),
      opaque: T.nilable(String),
      query: T.nilable(String),
      fragment: T.nilable(String),
      parser: T.untyped,
      arg_check: T::Boolean
    ).void
  end
  def initialize(scheme, userinfo, host, port, registry, path, opaque, query, fragment, parser = DEFAULT_PARSER, arg_check = false); end

  # ## Args
  #
  # `oth`
  # :   [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) or
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Merges two URIs.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com")
  # uri.merge("/main.rbx?page=1")
  # # => "http://my.example.com/main.rbx?page=1"
  # ```
  #
  #
  # Also aliased as:
  # [`+`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-2B)
  sig {params(oth: T.any(String, URI::Generic)).returns(URI::Generic)}
  def merge(oth); end

  # ## Args
  #
  # `oth`
  # :   [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) or
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Destructive form of
  # [`merge`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-merge).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com")
  # uri.merge!("/main.rbx?page=1")
  # uri.to_s  # => "http://my.example.com/main.rbx?page=1"
  # ```
  sig {params(oth: T.untyped).returns(T.untyped)}
  def merge!(oth); end

  # Returns normalized [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ```ruby
  # require 'uri'
  #
  # URI("HTTP://my.EXAMPLE.com").normalize
  # #=> #<URI::HTTP http://my.example.com/>
  # ```
  #
  # Normalization here means:
  #
  # *   scheme and host are converted to lowercase,
  # *   an empty path component is set to "/".
  sig {returns(URI::Generic)}
  def normalize; end

  # Destructive version of
  # [`normalize`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-normalize).
  sig {void}
  def normalize!; end

  # Returns the opaque part of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ```ruby
  # URI("mailto:foo@example.org").opaque #=> "foo@example.org"
  # URI("http://foo/bar/baz").opaque     #=> nil
  # ```
  #
  # The portion of the path that does not make use of the slash '/'. The path
  # typically refers to an absolute path or an opaque part. (See RFC2396 Section
  # 3 and 5.2.)
  sig {returns(T.nilable(String))}
  def opaque; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the opaque component `v` (with validation).
  #
  # See also
  # [`URI::Generic.check_opaque`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-check_opaque).
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def opaque=(v); end

  # Returns the parser to be used.
  #
  # Unless a
  # [`URI::Parser`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_Parser.html)
  # is defined, DEFAULT\_PARSER is used.
  sig {returns(T.untyped)}
  def parser; end

  # Returns the password component.
  sig {returns(T.nilable(String))}
  def password; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the `password` component (with validation).
  #
  # See also
  # [`URI::Generic.check_password`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-check_password).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://john:S3nsit1ve@my.example.com")
  # uri.password = "V3ry_S3nsit1ve"
  # uri.to_s  #=> "http://john:V3ry_S3nsit1ve@my.example.com"
  # ```
  sig {params(password: T.nilable(String)).returns(T.untyped)}
  def password=(password); end

  # Returns the path component of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ```ruby
  # URI("http://foo/bar/baz").path #=> "/bar/baz"
  # ```
  sig {returns(T.nilable(String))}
  def path; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the path component `v` (with validation).
  #
  # See also
  # [`URI::Generic.check_path`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-check_path).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com/pub/files")
  # uri.path = "/faq/"
  # uri.to_s  #=> "http://my.example.com/faq/"
  # ```
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def path=(v); end

  # Returns the port component of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ```ruby
  # URI("http://foo/bar/baz").port      #=> 80
  # URI("http://foo:8080/bar/baz").port #=> 8080
  # ```
  sig {returns(T.nilable(Integer))}
  def port; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the port component `v` (with validation).
  #
  # See also
  # [`URI::Generic.check_port`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-check_port).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com")
  # uri.port = 8080
  # uri.to_s  #=> "http://my.example.com:8080"
  # ```
  sig {params(v: T.nilable(Integer)).returns(T.untyped)}
  def port=(v); end

  # Returns the query component of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ```ruby
  # URI("http://foo/bar/baz?search=FooBar").query #=> "search=FooBar"
  # ```
  sig {returns(T.nilable(String))}
  def query; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the query component `v`.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com/?id=25")
  # uri.query = "id=1"
  # uri.to_s  #=> "http://my.example.com/?id=1"
  # ```
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def query=(v); end

  sig {returns(T.untyped)}
  def registry; end

  sig {params(v: T.untyped).returns(T.untyped)}
  def registry=(v); end

  # Returns true if [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) does
  # not have a scheme (e.g. http:// or https://) specified.
  sig {returns(T::Boolean)}
  def relative?; end

  # ## Args
  #
  # `oth`
  # :   [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) or
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Calculates relative path from oth to self.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse('http://my.example.com/main.rbx?page=1')
  # uri.route_from('http://my.example.com')
  # #=> #<URI::Generic /main.rbx?page=1>
  # ```
  #
  #
  # Also aliased as:
  # [`-`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-2D)
  sig {params(oth: T.untyped).returns(T.untyped)}
  def route_from(oth); end

  # ## Args
  #
  # `oth`
  # :   [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) or
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Calculates relative path to oth from self.
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse('http://my.example.com')
  # uri.route_to('http://my.example.com/main.rbx?page=1')
  # #=> #<URI::Generic /main.rbx?page=1>
  # ```
  sig {params(oth: T.untyped).returns(T.untyped)}
  def route_to(oth); end

  # Returns the scheme component of the
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ```ruby
  # URI("http://foo/bar/baz").scheme #=> "http"
  # ```
  sig {returns(T.nilable(String))}
  def scheme; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the scheme component `v` (with validation).
  #
  # See also
  # [`URI::Generic.check_scheme`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-check_scheme).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://my.example.com")
  # uri.scheme = "https"
  # uri.to_s  #=> "https://my.example.com"
  # ```
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def scheme=(v); end

  # ## Args
  #
  # `components`
  # :   Multiple [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html)
  #     arguments defined in
  #     [`URI::HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html).
  #
  #
  # ## Description
  #
  # Selects specified components from
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse('http://myuser:mypass@my.example.com/test.rbx')
  # uri.select(:userinfo, :host, :path)
  # # => ["myuser:mypass", "my.example.com", "/test.rbx"]
  # ```
  sig {params(components: T.untyped).returns(T.untyped)}
  def select(*components); end

  # Protected setter for the host component `v`.
  #
  # See also
  # [`URI::Generic.host=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-host-3D).
  sig {params(v: T.untyped).returns(T.untyped)}
  def set_host(v); end

  # Protected setter for the opaque component `v`.
  #
  # See also
  # [`URI::Generic.opaque=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-opaque-3D).
  sig {params(v: T.untyped).returns(T.untyped)}
  def set_opaque(v); end

  # Protected setter for the password component `v`.
  #
  # See also
  # [`URI::Generic.password=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-password-3D).
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def set_password(v); end

  # Protected setter for the path component `v`.
  #
  # See also
  # [`URI::Generic.path=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-path-3D).
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def set_path(v); end

  # Protected setter for the port component `v`.
  #
  # See also
  # [`URI::Generic.port=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-port-3D).
  sig {params(v: Integer).returns(T.untyped)}
  def set_port(v); end

  sig {params(v: T.untyped).returns(T.untyped)}
  def set_registry(v); end

  # Protected setter for the scheme component `v`.
  #
  # See also
  # [`URI::Generic.scheme=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-scheme-3D).
  sig {params(v: String).returns(T.untyped)}
  def set_scheme(v); end

  # Protected setter for the user component `v`.
  #
  # See also
  # [`URI::Generic.user=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-user-3D).
  sig {params(v: T.nilable(String)).returns(T.untyped)}
  def set_user(v); end

  # Protected setter for the `user` component, and `password` if available (with
  # validation).
  #
  # See also
  # [`URI::Generic.userinfo=`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-userinfo-3D).
  sig {params(user: T.untyped, password: T.untyped).returns(T.untyped)}
  def set_userinfo(user, password = nil); end

  # Returns the user component.
  sig {returns(T.nilable(String))}
  def user; end

  # ## Args
  #
  # `v`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Public setter for the `user` component (with validation).
  #
  # See also
  # [`URI::Generic.check_user`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-i-check_user).
  #
  # ## Usage
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("http://john:S3nsit1ve@my.example.com")
  # uri.user = "sam"
  # uri.to_s  #=> "http://sam:V3ry_S3nsit1ve@my.example.com"
  # ```
  sig {params(user: T.nilable(String)).returns(T.untyped)}
  def user=(user); end

  # Returns the userinfo, either as 'user' or 'user:password'.
  sig {returns(T.nilable(String))}
  def userinfo; end

  # Sets userinfo, argument is string like 'name:pass'.
  sig {params(userinfo: T.nilable(String)).returns(T.untyped)}
  def userinfo=(userinfo); end

  # ## Synopsis
  #
  # See
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-c-new).
  #
  # ## Description
  #
  # Creates a new
  # [`URI::Generic`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html)
  # instance from components of
  # [`URI::Generic`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html) with
  # check. Components are: scheme, userinfo, host, port, registry, path, opaque,
  # query, and fragment. You can provide arguments either by an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) or a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html). See
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-c-new)
  # for hash keys to use or for order of array items.
  sig {params(args: T.untyped).returns(T.untyped)}
  def self.build(args); end

  # ## Synopsis
  #
  # See
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-c-new).
  #
  # ## Description
  #
  # At first, tries to create a new
  # [`URI::Generic`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html)
  # instance using
  # [`URI::Generic::build`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#method-c-build).
  # But, if exception
  # [`URI::InvalidComponentError`](https://docs.ruby-lang.org/en/2.7.0/URI/InvalidComponentError.html)
  # is raised, then it does
  # [`URI::Escape.escape`](https://docs.ruby-lang.org/en/2.7.0/URI/Escape.html#method-i-escape)
  # all [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) components and
  # tries again.
  sig {params(args: T.untyped).returns(T.untyped)}
  def self.build2(args); end

  # Components of the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) in
  # the order.
  sig {returns(T.untyped)}
  def self.component; end

  # Returns default port.
  sig {returns(T.untyped)}
  def self.default_port; end

  sig do
    params(
      hostname: T.untyped,
      addr: T.untyped,
      port: T.untyped,
      no_proxy: T.untyped
    ).returns(T::Boolean)
  end
  def self.use_proxy?(hostname, addr, port, no_proxy); end

  sig {returns(T.untyped)}
  def self.use_registry; end
end

# The syntax of [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html) URIs
# is defined in RFC1738 section 3.3.
#
# Note that the Ruby [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
# library allows [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html)
# URLs containing usernames and passwords. This is not legal as per the RFC, but
# used to be supported in Internet Explorer 5 and 6, before the MS04-004
# security update. See <URL:http://support.microsoft.com/kb/834489>.
class URI::HTTP < URI::Generic
  include ::OpenURI::OpenRead

  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  # An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the
  # available components for
  # [`URI::HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html).
  COMPONENT = T.let(T.unsafe(nil), T::Array[T.untyped])
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
  # A Default port of 80 for
  # [`URI::HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html).
  DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
  ESCAPED = T.let(T.unsafe(nil), Regexp)
  FRAGMENT = T.let(T.unsafe(nil), Regexp)
  HOST = T.let(T.unsafe(nil), Regexp)
  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
  OPAQUE = T.let(T.unsafe(nil), Regexp)
  PORT = T.let(T.unsafe(nil), Regexp)
  QUERY = T.let(T.unsafe(nil), Regexp)
  REGISTRY = T.let(T.unsafe(nil), Regexp)
  REL_PATH = T.let(T.unsafe(nil), Regexp)
  REL_URI = T.let(T.unsafe(nil), Regexp)
  REL_URI_REF = T.let(T.unsafe(nil), Regexp)
  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
  SCHEME = T.let(T.unsafe(nil), Regexp)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # ## Description
  #
  # Returns the full path for an
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html) request, as
  # required by
  # [`Net::HTTP::Get`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Get.html).
  #
  # If the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) contains a
  # query, the full path is URI#path + '?' + URI#query. Otherwise, the path is
  # simply URI#path.
  #
  # Example:
  #
  # ```ruby
  # uri = URI::HTTP.build(path: '/foo/bar', query: 'test=true')
  # uri.request_uri #  => "/foo/bar?test=true"
  # ```
  sig { returns(String) }
  def request_uri; end

  # ## Description
  #
  # Creates a new
  # [`URI::HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html) object from
  # components, with syntax checking.
  #
  # The components accepted are userinfo, host, port, path, query, and fragment.
  #
  # The components should be provided either as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html), or as a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) with keys formed by
  # preceding the component names with a colon.
  #
  # If an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) is used, the
  # components must be passed in the order `[userinfo, host, port, path, query,
  # fragment]`.
  #
  # Example:
  #
  # ```ruby
  # uri = URI::HTTP.build(host: 'www.example.com', path: '/foo/bar')
  #
  # uri = URI::HTTP.build([nil, "www.example.com", nil, "/path",
  #   "query", 'fragment'])
  # ```
  #
  # Currently, if passed userinfo components this method generates invalid
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html) URIs as per RFC
  # 1738.
  def self.build(args); end
end

# The default port for
# [`HTTPS`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTPS.html) URIs is 443, and
# the scheme is 'https:' rather than 'http:'. Other than that,
# [`HTTPS`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTPS.html) URIs are
# identical to HTTP URIs; see
# [`URI::HTTP`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html).
class URI::HTTPS < URI::HTTP
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), T::Array[T.untyped])
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
  # A Default port of 443 for
  # [`URI::HTTPS`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTPS.html)
  DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
  ESCAPED = T.let(T.unsafe(nil), Regexp)
  FRAGMENT = T.let(T.unsafe(nil), Regexp)
  HOST = T.let(T.unsafe(nil), Regexp)
  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
  OPAQUE = T.let(T.unsafe(nil), Regexp)
  PORT = T.let(T.unsafe(nil), Regexp)
  QUERY = T.let(T.unsafe(nil), Regexp)
  REGISTRY = T.let(T.unsafe(nil), Regexp)
  REL_PATH = T.let(T.unsafe(nil), Regexp)
  REL_URI = T.let(T.unsafe(nil), Regexp)
  REL_URI_REF = T.let(T.unsafe(nil), Regexp)
  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
  SCHEME = T.let(T.unsafe(nil), Regexp)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
end

# Not a [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) component.
class URI::InvalidComponentError < URI::Error
end

# Not a [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
class URI::InvalidURIError < URI::Error
end

# [`LDAP`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAP.html)
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) SCHEMA (described in
# RFC2255).
class URI::LDAP < URI::Generic
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  # An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the
  # available components for
  # [`URI::LDAP`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAP.html).
  COMPONENT = T.let(T.unsafe(nil), T::Array[T.untyped])
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
  # A Default port of 389 for
  # [`URI::LDAP`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAP.html).
  DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
  ESCAPED = T.let(T.unsafe(nil), Regexp)
  FRAGMENT = T.let(T.unsafe(nil), Regexp)
  HOST = T.let(T.unsafe(nil), Regexp)
  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
  OPAQUE = T.let(T.unsafe(nil), Regexp)
  PORT = T.let(T.unsafe(nil), Regexp)
  QUERY = T.let(T.unsafe(nil), Regexp)
  REGISTRY = T.let(T.unsafe(nil), Regexp)
  REL_PATH = T.let(T.unsafe(nil), Regexp)
  REL_URI = T.let(T.unsafe(nil), Regexp)
  REL_URI_REF = T.let(T.unsafe(nil), Regexp)
  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
  SCHEME = T.let(T.unsafe(nil), Regexp)
  # Scopes available for the starting point.
  #
  # *   SCOPE\_BASE - the Base DN
  # *   SCOPE\_ONE  - one level under the Base DN, not including the base DN and
  #     not including any entries under this
  # *   SCOPE\_SUB  - subtrees, all entries at all levels
  SCOPE = T.let(T.unsafe(nil), T::Array[T.untyped])
  SCOPE_BASE = T.let(T.unsafe(nil), String)
  SCOPE_ONE = T.let(T.unsafe(nil), String)
  SCOPE_SUB = T.let(T.unsafe(nil), String)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
end

# The default port for
# [`LDAPS`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAPS.html) URIs is 636, and
# the scheme is 'ldaps:' rather than 'ldap:'. Other than that,
# [`LDAPS`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAPS.html) URIs are
# identical to LDAP URIs; see
# [`URI::LDAP`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAP.html).
class URI::LDAPS < URI::LDAP
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), T::Array[T.untyped])
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
  # A Default port of 636 for
  # [`URI::LDAPS`](https://docs.ruby-lang.org/en/2.7.0/URI/LDAPS.html)
  DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
  ESCAPED = T.let(T.unsafe(nil), Regexp)
  FRAGMENT = T.let(T.unsafe(nil), Regexp)
  HOST = T.let(T.unsafe(nil), Regexp)
  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
  OPAQUE = T.let(T.unsafe(nil), Regexp)
  PORT = T.let(T.unsafe(nil), Regexp)
  QUERY = T.let(T.unsafe(nil), Regexp)
  REGISTRY = T.let(T.unsafe(nil), Regexp)
  REL_PATH = T.let(T.unsafe(nil), Regexp)
  REL_URI = T.let(T.unsafe(nil), Regexp)
  REL_URI_REF = T.let(T.unsafe(nil), Regexp)
  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
  SCHEME = T.let(T.unsafe(nil), Regexp)
  SCOPE = T.let(T.unsafe(nil), T::Array[T.untyped])
  SCOPE_BASE = T.let(T.unsafe(nil), String)
  SCOPE_ONE = T.let(T.unsafe(nil), String)
  SCOPE_SUB = T.let(T.unsafe(nil), String)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # ## Description
  #
  # Creates a new
  # [`URI::LDAP`](https://docs.ruby-lang.org/en/2.6.0/URI/LDAP.html) object from
  # generic [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) components as
  # per RFC 2396. No LDAP-specific syntax checking is performed.
  #
  # Arguments are `scheme`, `userinfo`, `host`, `port`, `registry`, `path`,
  # `opaque`, `query`, and `fragment`, in that order.
  #
  # Example:
  #
  # ```ruby
  # uri = URI::LDAP.new("ldap", nil, "ldap.example.com", nil, nil,
  #   "/dc=example;dc=com", nil, "query", nil)
  # ```
  #
  # See also
  # [`URI::Generic.new`](https://docs.ruby-lang.org/en/2.6.0/URI/Generic.html#method-c-new).
  def self.new(*arg); end

  # Returns attributes.
  def attributes; end

  # Setter for attributes `val`.
  def attributes=(val); end

  # Returns dn.
  def dn; end

  # Setter for dn `val`.
  def dn=(val); end

  # Returns extensions.
  def extensions; end

  # Setter for extensions `val`.
  def extensions=(val); end

  # Returns filter.
  def filter; end

  # Setter for filter `val`.
  def filter=(val); end

  # Checks if [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) has a path.
  # For [`URI::LDAP`](https://docs.ruby-lang.org/en/2.6.0/URI/LDAP.html) this
  # will return `false`.
  def hierarchical?; end

  # Returns scope.
  def scope; end

  # Setter for scope `val`.
  def scope=(val); end

  protected

  # Private setter for attributes `val`.
  def set_attributes(val); end

  # Private setter for dn `val`.
  def set_dn(val); end

  # Private setter for extensions `val`.
  def set_extensions(val); end

  # Private setter for filter `val`.
  def set_filter(val); end

  # Private setter for scope `val`.
  def set_scope(val); end

  # ## Description
  #
  # Creates a new
  # [`URI::LDAP`](https://docs.ruby-lang.org/en/2.6.0/URI/LDAP.html) object from
  # components, with syntax checking.
  #
  # The components accepted are host, port, dn, attributes, scope, filter, and
  # extensions.
  #
  # The components should be provided either as an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html), or as a
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) with keys formed by
  # preceding the component names with a colon.
  #
  # If an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) is used, the
  # components must be passed in the order `[host, port, dn, attributes, scope,
  # filter, extensions]`.
  #
  # Example:
  #
  # ```ruby
  # uri = URI::LDAP.build({:host => 'ldap.example.com',
  #   :dn => '/dc=example'})
  #
  # uri = URI::LDAP.build(["ldap.example.com", nil,
  #   "/dc=example;dc=com", "query", nil, nil, nil])
  # ```
  def self.build(args); end
end

# RFC6068, the mailto URL scheme.
class URI::MailTo < URI::Generic
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  # An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the
  # available components for
  # [`URI::MailTo`](https://docs.ruby-lang.org/en/2.7.0/URI/MailTo.html).
  COMPONENT = T.let(T.unsafe(nil), T::Array[T.untyped])
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
  # A Default port of nil for
  # [`URI::MailTo`](https://docs.ruby-lang.org/en/2.7.0/URI/MailTo.html).
  DEFAULT_PORT = T.let(T.unsafe(nil), NilClass)
  EMAIL_REGEXP = T.let(T.unsafe(nil), Regexp)
  ESCAPED = T.let(T.unsafe(nil), Regexp)
  FRAGMENT = T.let(T.unsafe(nil), Regexp)
  HEADER_REGEXP = T.let(T.unsafe(nil), Regexp)
  HOST = T.let(T.unsafe(nil), Regexp)
  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
  OPAQUE = T.let(T.unsafe(nil), Regexp)
  PORT = T.let(T.unsafe(nil), Regexp)
  QUERY = T.let(T.unsafe(nil), Regexp)
  REGISTRY = T.let(T.unsafe(nil), Regexp)
  REL_PATH = T.let(T.unsafe(nil), Regexp)
  REL_URI = T.let(T.unsafe(nil), Regexp)
  REL_URI_REF = T.let(T.unsafe(nil), Regexp)
  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
  SCHEME = T.let(T.unsafe(nil), Regexp)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # ## Description
  #
  # Creates a new
  # [`URI::MailTo`](https://docs.ruby-lang.org/en/2.7.0/URI/MailTo.html) object
  # from generic URL components with no syntax checking.
  #
  # This method is usually called from
  # [`URI::parse`](https://docs.ruby-lang.org/en/2.7.0/URI.html#method-c-parse),
  # which checks the validity of each component.
  def self.new(*arg); end

  # E-mail headers set by the URL, as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Arrays.
  def headers; end

  # Setter for headers `v`.
  def headers=(v); end

  # The primary e-mail address of the URL, as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  def to; end

  # Setter for to `v`.
  def to=(v); end

  # Returns the RFC822 e-mail text equivalent of the URL, as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # Example:
  #
  # ```ruby
  # require 'uri'
  #
  # uri = URI.parse("mailto:ruby-list@ruby-lang.org?Subject=subscribe&cc=myaddr")
  # uri.to_mailtext
  # # => "To: ruby-list@ruby-lang.org\nSubject: subscribe\nCc: myaddr\n\n\n"
  # ```
  #
  #
  # Also aliased as:
  # [`to_rfc822text`](https://docs.ruby-lang.org/en/2.7.0/URI/MailTo.html#method-i-to_rfc822text)
  def to_mailtext; end

  # Alias for:
  # [`to_mailtext`](https://docs.ruby-lang.org/en/2.7.0/URI/MailTo.html#method-i-to_mailtext)
  def to_rfc822text; end

  # Constructs [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) from
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
  def to_s; end

  protected

  # Private setter for headers `v`.
  def set_headers(v); end

  # Private setter for to `v`.
  def set_to(v); end

  # ## Description
  #
  # Creates a new
  # [`URI::MailTo`](https://docs.ruby-lang.org/en/2.7.0/URI/MailTo.html) object
  # from components, with syntax checking.
  #
  # Components can be provided as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) or
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html). If an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) is used, the
  # components must be supplied as `[to, headers]`.
  #
  # If a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) is used, the
  # keys are the component names preceded by colons.
  #
  # The headers can be supplied as a pre-encoded string, such as
  # `"subject=subscribe&cc=address"`, or as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Arrays like
  # `[['subject', 'subscribe'], ['cc', 'address']]`.
  #
  # Examples:
  #
  # ```ruby
  # require 'uri'
  #
  # m1 = URI::MailTo.build(['joe@example.com', 'subject=Ruby'])
  # m1.to_s  # => "mailto:joe@example.com?subject=Ruby"
  #
  # m2 = URI::MailTo.build(['john@example.com', [['Subject', 'Ruby'], ['Cc', 'jack@example.com']]])
  # m2.to_s  # => "mailto:john@example.com?Subject=Ruby&Cc=jack@example.com"
  #
  # m3 = URI::MailTo.build({:to => 'listman@example.com', :headers => [['subject', 'subscribe']]})
  # m3.to_s  # => "mailto:listman@example.com?subject=subscribe"
  # ```
  def self.build(args); end
end

# Includes URI::REGEXP::PATTERN
module URI::RFC2396_REGEXP
end

# Patterns used to parse URI's
module URI::RFC2396_REGEXP::PATTERN
  ABS_PATH = T.let(T.unsafe(nil), String)
  ABS_URI = T.let(T.unsafe(nil), String)
  ALNUM = T.let(T.unsafe(nil), String)
  ALPHA = T.let(T.unsafe(nil), String)
  DOMLABEL = T.let(T.unsafe(nil), String)
  ESCAPED = T.let(T.unsafe(nil), String)
  FRAGMENT = T.let(T.unsafe(nil), String)
  HEX = T.let(T.unsafe(nil), String)
  HIER_PART = T.let(T.unsafe(nil), String)
  HOST = T.let(T.unsafe(nil), String)
  HOSTNAME = T.let(T.unsafe(nil), String)
  HOSTPORT = T.let(T.unsafe(nil), String)
  IPV4ADDR = T.let(T.unsafe(nil), String)
  IPV6ADDR = T.let(T.unsafe(nil), String)
  IPV6REF = T.let(T.unsafe(nil), String)
  NET_PATH = T.let(T.unsafe(nil), String)
  OPAQUE_PART = T.let(T.unsafe(nil), String)
  PATH_SEGMENTS = T.let(T.unsafe(nil), String)
  PORT = T.let(T.unsafe(nil), String)
  QUERY = T.let(T.unsafe(nil), String)
  REG_NAME = T.let(T.unsafe(nil), String)
  REL_PATH = T.let(T.unsafe(nil), String)
  REL_SEGMENT = T.let(T.unsafe(nil), String)
  REL_URI = T.let(T.unsafe(nil), String)
  RESERVED = T.let(T.unsafe(nil), String)
  SCHEME = T.let(T.unsafe(nil), String)
  TOPLABEL = T.let(T.unsafe(nil), String)
  UNRESERVED = T.let(T.unsafe(nil), String)
  URIC = T.let(T.unsafe(nil), String)
  URIC_NO_SLASH = T.let(T.unsafe(nil), String)
  URI_REF = T.let(T.unsafe(nil), String)
  USERINFO = T.let(T.unsafe(nil), String)
  X_ABS_URI = T.let(T.unsafe(nil), String)
  X_REL_URI = T.let(T.unsafe(nil), String)
end

# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) that parses String's
# into URI's.
#
# It contains a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) set of
# patterns and Regexp's that match and validate.
class URI::RFC2396_Parser < Object
  include URI::RFC2396_REGEXP

  # ## Synopsis
  #
  # ```ruby
  # URI::Parser.new([opts])
  # ```
  #
  # ## Args
  #
  # The constructor accepts a hash as options for parser. Keys of options are
  # pattern names of [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
  # components and values of options are pattern strings. The constructor
  # generates set of regexps for parsing URIs.
  #
  # You can use the following keys:
  #
  # ```
  # * :ESCAPED (URI::PATTERN::ESCAPED in default)
  # * :UNRESERVED (URI::PATTERN::UNRESERVED in default)
  # * :DOMLABEL (URI::PATTERN::DOMLABEL in default)
  # * :TOPLABEL (URI::PATTERN::TOPLABEL in default)
  # * :HOSTNAME (URI::PATTERN::HOSTNAME in default)
  # ```
  #
  # ## Examples
  #
  # ```ruby
  # p = URI::Parser.new(:ESCAPED => "(?:%[a-fA-F0-9]{2}|%u[a-fA-F0-9]{4})")
  # u = p.parse("http://example.jp/%uABCD") #=> #<URI::HTTP http://example.jp/%uABCD>
  # URI.parse(u.to_s) #=> raises URI::InvalidURIError
  #
  # s = "http://example.com/ABCD"
  # u1 = p.parse(s) #=> #<URI::HTTP http://example.com/ABCD>
  # u2 = URI.parse(s) #=> #<URI::HTTP http://example.com/ABCD>
  # u1 == u2 #=> true
  # u1.eql?(u2) #=> false
  # ```
  def self.new(opts = _); end

  # ## Args
  #
  # `str`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to make safe
  # `unsafe`
  # :   [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) to apply.
  #     Defaults to [self.regexp](:UNSAFE)
  #
  #
  # ## Description
  #
  # Constructs a safe
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) from `str`,
  # removing unsafe characters, replacing them with codes.
  def escape(str, unsafe = _); end

  # ## Args
  #
  # `str`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to search
  # `schemes`
  # :   Patterns to apply to `str`
  #
  #
  # ## Description
  #
  # Attempts to parse and merge a set of URIs. If no `block` given, then returns
  # the result, else it calls `block` for each element in result.
  #
  # See also
  # [`URI::Parser.make_regexp`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_Parser.html#method-i-make_regexp).
  def extract(str, schemes = _); end

  def inspect; end

  # ## Args
  #
  # `uris`
  # :   an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Strings
  #
  #
  # ## Description
  #
  # Attempts to parse and merge a set of URIs.
  def join(*uris); end

  # Returns [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) that is
  # default [self.regexp](:ABS\_URI\_REF), unless `schemes` is provided. Then it
  # is a
  # [`Regexp.union`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-union)
  # with [self.pattern](:X\_ABS\_URI).
  def make_regexp(schemes = _); end

  # ## Args
  #
  # `uri`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  #
  # ## Description
  #
  # Parses `uri` and constructs either matching
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) scheme object
  # ([`File`](https://docs.ruby-lang.org/en/2.7.0/File.html), FTP, HTTP, HTTPS,
  # LDAP, LDAPS, or MailTo) or
  # [`URI::Generic`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html).
  #
  # ## Usage
  #
  # ```ruby
  # p = URI::Parser.new
  # p.parse("ldap://ldap.example.com/dc=example?user=john")
  # #=> #<URI::LDAP ldap://ldap.example.com/dc=example?user=john>
  # ```
  def parse(uri); end

  # The [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of patterns.
  #
  # See also
  # [`URI::Parser.initialize_pattern`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_Parser.html#method-i-initialize_pattern).
  def pattern; end

  # The [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of
  # [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html).
  #
  # See also
  # [`URI::Parser.initialize_regexp`](https://docs.ruby-lang.org/en/2.7.0/URI/RFC2396_Parser.html#method-i-initialize_regexp).
  def regexp; end

  # Returns a split [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
  # against [regexp](:ABS_URI).
  def split(uri); end

  # ## Args
  #
  # `str`
  # :   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to remove
  #     escapes from
  # `escaped`
  # :   [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) to apply.
  #     Defaults to [self.regexp](:ESCAPED)
  #
  #
  # ## Description
  #
  # Removes escapes from `str`.
  def unescape(str, escaped = _); end
end

class URI::RFC3986_Parser < Object
  RFC3986_URI = T.let(T.unsafe(nil), Regexp)
end

module URI::Util
end

# The "file" [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) is defined by
# RFC8089.
class URI::File
end
