# typed: strict

class CGI
  include ::CGI::Util
  extend ::CGI::Escape
  extend ::CGI::Util
  CR = ::T.let(nil, ::T.untyped)
  EOL = ::T.let(nil, ::T.untyped)
  HTTP_STATUS = ::T.let(nil, ::T.untyped)
  LF = ::T.let(nil, ::T.untyped)
  MAX_MULTIPART_COUNT = ::T.let(nil, ::T.untyped)
  NEEDS_BINMODE = ::T.let(nil, ::T.untyped)
  PATH_SEPARATOR = ::T.let(nil, ::T.untyped)
  REVISION = ::T.let(nil, ::T.untyped)

  sig {returns(::T.untyped)}
  def accept_charset(); end

  sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def header(options=T.unsafe(nil)); end

  sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def http_header(options=T.unsafe(nil)); end

  sig do
    params(
      options: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(options=T.unsafe(nil), &block); end

  sig {returns(::T.untyped)}
  def nph?(); end

  sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def out(options=T.unsafe(nil)); end

  sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def print(*options); end

  sig {returns(::T.untyped)}
  def self.accept_charset(); end

  sig do
    params(
      accept_charset: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.accept_charset=(accept_charset); end

  sig do
    params(
      query: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(query); end
end

class CGI::Cookie < Array
  Elem = type_member(:out, fixed: String)

  sig {returns(::T.untyped)}
  def domain(); end

  sig do
    params(
      domain: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def domain=(domain); end

  sig {returns(::T.untyped)}
  def expires(); end

  sig do
    params(
      expires: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def expires=(expires); end

  sig {returns(::T.untyped)}
  def httponly(); end

  sig do
    params(
      val: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def httponly=(val); end

  sig do
    params(
      name: ::T.untyped,
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(name=T.unsafe(nil), *value); end

  sig {returns(::T.untyped)}
  def inspect(); end

  sig {returns(::T.untyped)}
  def name(); end

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def name=(name); end

  sig {returns(::T.untyped)}
  def path(); end

  sig do
    params(
      path: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def path=(path); end

  sig {returns(::T.untyped)}
  def secure(); end

  sig do
    params(
      val: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def secure=(val); end

  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def value(); end

  sig do
    params(
      val: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def value=(val); end

  sig do
    params(
      raw_cookie: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(raw_cookie); end
end

module CGI::QueryExtension
  sig do
    params(
      key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def [](key); end

  sig {returns(::T.untyped)}
  def accept(); end

  sig {returns(::T.untyped)}
  def accept_charset(); end

  sig {returns(::T.untyped)}
  def accept_encoding(); end

  sig {returns(::T.untyped)}
  def accept_language(); end

  sig {returns(::T.untyped)}
  def auth_type(); end

  sig {returns(::T.untyped)}
  def cache_control(); end

  sig {returns(::T.untyped)}
  def content_length(); end

  sig {returns(::T.untyped)}
  def content_type(); end

  sig {returns(::T.untyped)}
  def cookies(); end

  sig do
    params(
      cookies: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cookies=(cookies); end

  sig do
    params(
      is_large: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def create_body(is_large); end

  sig {returns(::T.untyped)}
  def files(); end

  sig {returns(::T.untyped)}
  def from(); end

  sig {returns(::T.untyped)}
  def gateway_interface(); end

  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def has_key?(*args); end

  sig {returns(::T.untyped)}
  def host(); end

  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def include?(*args); end

  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def key?(*args); end

  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def keys(*args); end

  sig {returns(::T.untyped)}
  def multipart?(); end

  sig {returns(::T.untyped)}
  def negotiate(); end

  sig {returns(::T.untyped)}
  def params(); end

  sig do
    params(
      hash: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def params=(hash); end

  sig {returns(::T.untyped)}
  def path_info(); end

  sig {returns(::T.untyped)}
  def path_translated(); end

  sig {returns(::T.untyped)}
  def pragma(); end

  sig {returns(::T.untyped)}
  def query_string(); end

  sig {returns(::T.untyped)}
  def raw_cookie(); end

  sig {returns(::T.untyped)}
  def raw_cookie2(); end

  sig {returns(::T.untyped)}
  def referer(); end

  sig {returns(::T.untyped)}
  def remote_addr(); end

  sig {returns(::T.untyped)}
  def remote_host(); end

  sig {returns(::T.untyped)}
  def remote_ident(); end

  sig {returns(::T.untyped)}
  def remote_user(); end

  sig {returns(::T.untyped)}
  def request_method(); end

  sig {returns(::T.untyped)}
  def script_name(); end

  sig {returns(::T.untyped)}
  def server_name(); end

  sig {returns(::T.untyped)}
  def server_port(); end

  sig {returns(::T.untyped)}
  def server_protocol(); end

  sig {returns(::T.untyped)}
  def server_software(); end

  sig {returns(::T.untyped)}
  def unescape_filename?(); end

  sig {returns(::T.untyped)}
  def user_agent(); end
end

module CGI::Util
  include ::CGI::Escape
  RFC822_DAYS = ::T.let(nil, ::T.untyped)
  RFC822_MONTHS = ::T.let(nil, ::T.untyped)
  TABLE_FOR_ESCAPE_HTML__ = ::T.let(nil, ::T.untyped)

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape(_); end

  sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escapeElement(string, *elements); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escapeHTML(_); end

  sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape_element(string, *elements); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape_html(_); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def h(_); end

  sig do
    params(
      string: ::T.untyped,
      shift: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pretty(string, shift=T.unsafe(nil)); end

  sig do
    params(
      time: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def rfc1123_date(time); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape(*_); end

  sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescapeElement(string, *elements); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescapeHTML(_); end

  sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape_element(string, *elements); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape_html(_); end
end

module CGI::Escape
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape(_); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escapeHTML(_); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape(*_); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescapeHTML(_); end
end

class CGI::InvalidEncoding < Exception
end
