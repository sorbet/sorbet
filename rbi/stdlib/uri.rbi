# typed: __STDLIB_INTERNAL

module Kernel
  sig {params(uri: T.any(URI::Generic, String)).returns(URI::Generic)}
  def URI(uri); end
end

module URI
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
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
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)

  sig do
    params(
        str: String,
        enc: Encoding,
    )
    .returns(T::Array[[String, String]])
  end
  def self.decode_www_form_component(str, enc=T.unsafe(nil)); end

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

  sig do
    params(
        str: String,
        schemes: Array,
        blk: BasicObject,
    )
    .returns(T::Array[String])
  end
  def self.extract(str, schemes=T.unsafe(nil), &blk); end

  sig do
    params(
        str: T.any(URI::Generic, String),
    )
    .returns(URI::Generic)
  end
  def self.join(*str); end

  sig do
    params(
        uri: String,
    )
    .returns(URI::HTTP)
  end
  def self.parse(uri); end

  sig do
    params(
        schemes: Array,
    )
    .returns(T::Array[String])
  end
  def self.regexp(schemes=T.unsafe(nil)); end

  sig {returns(T::Hash[String, Class])}
  def self.scheme_list(); end

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

class URI::BadURIError < URI::Error
end

class URI::Error < StandardError
end

module URI::Escape
end

class URI::FTP < URI::Generic
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), Array)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
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
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TYPECODE = T.let(T.unsafe(nil), Array)
  TYPECODE_PREFIX = T.let(T.unsafe(nil), String)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
end

class URI::Generic < Object
  include URI
  include URI::RFC2396_REGEXP

  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), Array)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
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
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)

  sig { params(oth: T.untyped).returns(T.untyped) }
  def +(oth); end

  sig { params(oth: T.untyped).returns(T.untyped) }
  def -(oth); end

  # Compares two URIs.
  sig { params(oth: T.untyped).returns(T::Boolean) }
  def ==(oth); end

  # Returns true if URI has a scheme (e.g. http:// or https://) specified.
  sig { returns(T::Boolean) }
  def absolute; end

  # Returns true if URI has a scheme (e.g. http:// or https://) specified.
  sig { returns(T::Boolean) }
  def absolute?; end

  # Attempts to parse other URI `oth`,
  # returns [parsed_oth, self].
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com")
  # uri.coerce("http://foo.com")
  # #=> [#<URI::HTTP http://foo.com>, #<URI::HTTP http://my.example.com>]
  # ```
  sig { params(oth: T.any(URI, String)).returns(T.untyped) }
  def coerce(oth); end

  # Components of the URI in the order.
  sig { returns(T.untyped) }
  def component; end

  sig { returns(T.untyped) }
  def component_ary; end

  # Returns default port.
  sig { returns(T.untyped) }
  def default_port; end

  sig { params(oth: T.untyped).returns(T::Boolean) }
  def eql?(oth); end

  # Returns a proxy URI.
  # The proxy URI is obtained from environment variables such as http_proxy,
  # ftp_proxy, no_proxy, etc.
  # If there is no proper proxy, nil is returned.
  # 
  # If the optional parameter `env` is specified, it is used instead of ENV.
  # 
  # Note that capitalized variables (HTTP_PROXY, FTP_PROXY, NO_PROXY, etc.)
  # are examined, too.
  # 
  # But http_proxy and HTTP_PROXY is treated specially under CGI environment.
  # It's because HTTP_PROXY may be set by Proxy: header.
  # So HTTP_PROXY is not used.
  # http_proxy is not used too if the variable is case insensitive.
  # CGI_HTTP_PROXY can be used instead.
  sig { params(env: T.untyped).returns(T.untyped) }
  def find_proxy(env = T.unsafe(nil)); end

  # Returns the fragment component of the URI.
  # 
  # ```ruby
  # URI("http://foo/bar/baz?search=FooBar#ponies").fragment #=> "ponies"
  # ```
  sig { returns(T.nilable(String)) }
  def fragment; end

  # Checks the fragment `v` component against the URI::Parser Regexp for :FRAGMENT.
  # 
  # Public setter for the fragment component `v`
  # (with validation).
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com/?id=25#time=1305212049")
  # uri.fragment = "time=1305212086"
  # uri.to_s  #=> "http://my.example.com/?id=25#time=1305212086"
  # ```
  sig { params(v: T.nilable(String)).returns(T.untyped) }
  def fragment=(v); end

  # Returns true if URI is hierarchical.
  # 
  # URI has components listed in order of decreasing significance from left to right,
  # see RFC3986 https://tools.ietf.org/html/rfc3986 1.2.3.
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
  sig { returns(T::Boolean) }
  def hierarchical?; end

  # Returns the host component of the URI.
  # 
  # ```
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
  # Since IPv6 addresses are wrapped with brackets in URIs,
  # this method returns IPv6 addresses wrapped with brackets.
  # This form is not appropriate to pass to socket methods such as TCPSocket.open.
  # If unwrapped host names are required, use the #hostname method.
  # 
  # ```ruby
  # URI("http://[::1]/bar/baz").host     #=> "[::1]"
  # URI("http://[::1]/bar/baz").hostname #=> "::1"
  # ```
  sig { returns(T.nilable(String)) }
  def host; end

  # Public setter for the host component `v`
  # (with validation).
  # 
  # See also URI::Generic.check_host.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com")
  # uri.host = "foo.com"
  # uri.to_s  #=> "http://foo.com"
  # ```
  sig { params(v: String).returns(T.untyped) }
  def host=(v); end

  # Extract the host part of the URI and unwrap brackets for IPv6 addresses.
  # 
  # This method is the same as URI::Generic#host except
  # brackets for IPv6 (and future IP) addresses are removed.
  # 
  # ```ruby
  # uri = URI("http://[::1]/bar")
  # uri.hostname      #=> "::1"
  # uri.host          #=> "[::1]"
  # ```
  sig { returns(String) }
  def hostname; end

  # Sets the host part of the URI as the argument with brackets for IPv6 addresses.
  # 
  # This method is the same as URI::Generic#host= except
  # the argument can be a bare IPv6 address.
  # 
  # ```ruby
  # uri = URI("http://foo/bar")
  # uri.hostname = "::1"
  # uri.to_s  #=> "http://[::1]/bar"
  # ```
  # 
  # If the argument seems to be an IPv6 address,
  # it is wrapped with brackets.
  sig { params(v: String).returns(T.untyped) }
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

  # Merges two URIs.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com")
  # uri.merge("/main.rbx?page=1")
  # # => "http://my.example.com/main.rbx?page=1"
  # ```
  sig { params(oth: String).returns(URI) }
  def merge(oth); end

  # Destructive form of #merge.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com")
  # uri.merge!("/main.rbx?page=1")
  # uri.to_s  # => "http://my.example.com/main.rbx?page=1"
  # ```
  sig { params(oth: T.untyped).returns(T.untyped) }
  def merge!(oth); end

  # Returns normalized URI.
  # 
  # ```
  # require 'uri'
  # 
  # URI("HTTP://my.EXAMPLE.com").normalize
  # #=> #<URI::HTTP http://my.example.com/>
  # ```
  # 
  # Normalization here means:
  # 
  # * scheme and host are converted to lowercase,
  # * an empty path component is set to "/".
  sig { returns(URI) }
  def normalize; end

  # Destructive version of #normalize.
  sig { void }
  def normalize!; end

  # Returns the opaque part of the URI.
  # 
  # ```ruby
  # URI("mailto:foo@example.org").opaque #=> "foo@example.org"
  # URI("http://foo/bar/baz").opaque     #=> nil
  # ```
  # 
  # The portion of the path that does not make use of the slash '/'.
  # The path typically refers to an absolute path or an opaque part.
  # (See RFC2396 Section 3 and 5.2.)
  sig { returns(T.nilable(String)) }
  def opaque; end

  # Public setter for the opaque component `v`
  # (with validation).
  # 
  # See also URI::Generic.check_opaque.
  sig { params(v: T.nilable(String)).returns(T.untyped) }
  def opaque=(v); end

  # Returns the parser to be used.
  # 
  # Unless a URI::Parser is defined, DEFAULT_PARSER is used.
  sig { returns(T.untyped) }
  def parser; end

  # Returns the password component.
  sig { returns(T.nilable(String)) }
  def password; end

  # Public setter for the `password` component
  # (with validation).
  # 
  # See also URI::Generic.check_password.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://john:S3nsit1ve@my.example.com")
  # uri.password = "V3ry_S3nsit1ve"
  # uri.to_s  #=> "http://john:V3ry_S3nsit1ve@my.example.com"
  # ```
  sig { params(password: T.nilable(String)).returns(T.untyped) }
  def password=(password); end

  # Returns the path component of the URI.
  # 
  # ```ruby
  # URI("http://foo/bar/baz").path #=> "/bar/baz"
  # ```
  sig { returns(T.nilable(String)) }
  def path; end

  # Public setter for the path component `v`
  # (with validation).
  # 
  # See also URI::Generic.check_path.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com/pub/files")
  # uri.path = "/faq/"
  # uri.to_s  #=> "http://my.example.com/faq/"
  # ```
  sig { params(v: T.nilable(String)).returns(T.untyped) }
  def path=(v); end

  # Returns the port component of the URI.
  # 
  # ```
  # URI("http://foo/bar/baz").port      #=> 80
  # URI("http://foo:8080/bar/baz").port #=> 8080
  # ```
  sig { returns(T.nilable(Integer)) }
  def port; end

  # Public setter for the port component `v`
  # (with validation).
  # 
  # See also URI::Generic.check_port.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com")
  # uri.port = 8080
  # uri.to_s  #=> "http://my.example.com:8080"
  # ```
  sig { params(v: T.nilable(Integer)).returns(T.untyped) }
  def port=(v); end

  # Returns the query component of the URI.
  # 
  # ```ruby
  # URI("http://foo/bar/baz?search=FooBar").query #=> "search=FooBar"
  # ```
  sig { returns(T.nilable(String)) }
  def query; end

  # Public setter for the query component `v`.
  # 
  # ```
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com/?id=25")
  # uri.query = "id=1"
  # uri.to_s  #=> "http://my.example.com/?id=1"
  # ```
  sig { params(v: T.nilable(String)).returns(T.untyped) }
  def query=(v); end

  sig { returns(T.untyped) }
  def registry; end

  sig { params(v: T.untyped).returns(T.untyped) }
  def registry=(v); end

  # Returns true if URI does not have a scheme (e.g. http:// or https://) specified.
  sig { returns(T::Boolean) }
  def relative?; end

  # Calculates relative path from oth to self.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse('http://my.example.com/main.rbx?page=1')
  # uri.route_from('http://my.example.com')
  # #=> #<URI::Generic /main.rbx?page=1>
  # ```
  sig { params(oth: T.untyped).returns(T.untyped) }
  def route_from(oth); end

  # Calculates relative path to oth from self.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse('http://my.example.com')
  # uri.route_to('http://my.example.com/main.rbx?page=1')
  # #=> #<URI::Generic /main.rbx?page=1>
  # ```
  sig { params(oth: T.untyped).returns(T.untyped) }
  def route_to(oth); end

  # Returns the scheme component of the URI.
  # 
  # ```ruby
  # URI("http://foo/bar/baz").scheme #=> "http"
  # ```
  sig { returns(T.nilable(String)) }
  def scheme; end

  # Public setter for the scheme component `v`
  # (with validation).
  # 
  # See also URI::Generic.check_scheme.
  # 
  # ```
  # require 'uri'
  # 
  # uri = URI.parse("http://my.example.com")
  # uri.scheme = "https"
  # uri.to_s  #=> "https://my.example.com"
  # ```
  sig { params(v: T.nilable(String)).returns(T.untyped) }
  def scheme=(v); end

  # Selects specified components from URI.
  # 
  # ```
  # require 'uri'
  # 
  # uri = URI.parse('http://myuser:mypass@my.example.com/test.rbx')
  # uri.select(:userinfo, :host, :path)
  # # => ["myuser:mypass", "my.example.com", "/test.rbx"]
  # ```
  sig { params(components: T.untyped).returns(T.untyped) }
  def select(*components); end

  # Protected setter for the host component `v`.
  # 
  # See also URI::Generic.host=.
  sig { params(v: T.untyped).returns(T.untyped) }
  def set_host(v); end

  # Protected setter for the opaque component `v`.
  # 
  # See also URI::Generic.opaque=.
  sig { params(v: T.untyped).returns(T.untyped) }
  def set_opaque(v); end

  # Protected setter for the password component `v`.
  # 
  # See also URI::Generic.password=.
  sig { params(v: T.nilable(String)).returns(T.untyped) }
  def set_password(v); end

  # Protected setter for the path component `v`.
  # 
  # See also URI::Generic.path=.
  sig { params(v: T.nilable(String)).returns(T.untyped) }
  def set_path(v); end

  # Protected setter for the port component `v`.
  # 
  # See also URI::Generic.port=.
  sig { params(v: Integer).returns(T.untyped) }
  def set_port(v); end

  sig { params(v: T.untyped).returns(T.untyped) }
  def set_registry(v); end

  # Protected setter for the scheme component `v`.
  # 
  # See also URI::Generic.scheme=.
  sig { params(v: String).returns(T.untyped) }
  def set_scheme(v); end

  # Protected setter for the user component `v`.
  # 
  # See also URI::Generic.user=.
  sig { params(v: T.nilable(String)).returns(T.untyped) }
  def set_user(v); end

  # Protected setter for the `user` component, and `password` if available
  # (with validation).
  # 
  # See also URI::Generic.userinfo=.
  sig { params(user: T.untyped, password: T.untyped).returns(T.untyped) }
  def set_userinfo(user, password = nil); end

  # Returns the user component.
  sig { returns(T.nilable(String)) }
  def user; end

  # Public setter for the `user` component
  # (with validation).
  # 
  # See also URI::Generic.check_user.
  # 
  # ```ruby
  # require 'uri'
  # 
  # uri = URI.parse("http://john:S3nsit1ve@my.example.com")
  # uri.user = "sam"
  # uri.to_s  #=> "http://sam:V3ry_S3nsit1ve@my.example.com"
  # ```
  sig { params(user: T.nilable(String)).returns(T.untyped) }
  def user=(user); end

  # Returns the userinfo, either as 'user' or 'user:password'.
  sig { returns(T.nilable(String)) }
  def userinfo; end

  # Sets userinfo, argument is string like 'name:pass'.
  sig { params(userinfo: T.nilable(String)).returns(T.untyped) }
  def userinfo=(userinfo); end

  # Creates a new URI::Generic instance from components of URI::Generic
  # with check. Components are: scheme, userinfo, host, port, registry, path,
  # opaque, query, and fragment. You can provide arguments either by an Array or a Hash.
  # See ::new for hash keys to use or for order of array items.
  sig { params(args: T.untyped).returns(T.untyped) }
  def self.build(args); end

  # At first, tries to create a new URI::Generic instance using
  # URI::Generic::build. But, if exception URI::InvalidComponentError is raised,
  # then it does URI::Escape.escape all URI components and tries again.
  sig { params(args: T.untyped).returns(T.untyped) }
  def self.build2(args); end

  # Components of the URI in the order.
  sig { returns(T.untyped) }
  def self.component; end

  # Returns default port.
  sig { returns(T.untyped) }
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

  sig { returns(T.untyped) }
  def self.use_registry; end
end

class URI::HTTP < URI::Generic
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), Array)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
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
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
end

class URI::HTTPS < URI::HTTP
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), Array)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
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
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
end

class URI::InvalidComponentError < URI::Error
end

class URI::InvalidURIError < URI::Error
end

class URI::LDAP < URI::Generic
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), Array)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
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
  SCOPE = T.let(T.unsafe(nil), Array)
  SCOPE_BASE = T.let(T.unsafe(nil), String)
  SCOPE_ONE = T.let(T.unsafe(nil), String)
  SCOPE_SUB = T.let(T.unsafe(nil), String)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
end

class URI::LDAPS < URI::LDAP
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), Array)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
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
  SCOPE = T.let(T.unsafe(nil), Array)
  SCOPE_BASE = T.let(T.unsafe(nil), String)
  SCOPE_ONE = T.let(T.unsafe(nil), String)
  SCOPE_SUB = T.let(T.unsafe(nil), String)
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
end

class URI::MailTo < URI::Generic
  ABS_PATH = T.let(T.unsafe(nil), Regexp)
  ABS_URI = T.let(T.unsafe(nil), Regexp)
  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
  COMPONENT = T.let(T.unsafe(nil), Array)
  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
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
  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
  UNSAFE = T.let(T.unsafe(nil), Regexp)
  URI_REF = T.let(T.unsafe(nil), Regexp)
  USERINFO = T.let(T.unsafe(nil), Regexp)
  USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
  VERSION = T.let(T.unsafe(nil), String)
  VERSION_CODE = T.let(T.unsafe(nil), String)
  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
end

module URI::RFC2396_REGEXP
end

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

class URI::RFC2396_Parser < Object
  include URI::RFC2396_REGEXP
end

class URI::RFC3986_Parser < Object
  RFC3986_URI = T.let(T.unsafe(nil), Regexp)
end

module URI::Util
end
