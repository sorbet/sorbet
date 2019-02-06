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

  Sorbet.sig {returns(::T.untyped)}
  def accept_charset(); end

  Sorbet.sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def header(options=T.unsafe(nil)); end

  Sorbet.sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def http_header(options=T.unsafe(nil)); end

  Sorbet.sig do
    params(
      options: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(options=T.unsafe(nil), &block); end

  Sorbet.sig {returns(::T.untyped)}
  def nph?(); end

  Sorbet.sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def out(options=T.unsafe(nil)); end

  Sorbet.sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def print(*options); end

  Sorbet.sig {returns(::T.untyped)}
  def self.accept_charset(); end

  Sorbet.sig do
    params(
      accept_charset: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.accept_charset=(accept_charset); end

  Sorbet.sig do
    params(
      query: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(query); end
end

class CGI::Cookie < Array
  Elem = type_member(:out, fixed: String)

  Sorbet.sig {returns(::T.untyped)}
  def domain(); end

  Sorbet.sig do
    params(
      domain: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def domain=(domain); end

  Sorbet.sig {returns(::T.untyped)}
  def expires(); end

  Sorbet.sig do
    params(
      expires: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def expires=(expires); end

  Sorbet.sig {returns(::T.untyped)}
  def httponly(); end

  Sorbet.sig do
    params(
      val: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def httponly=(val); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(name=T.unsafe(nil), *value); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def name(); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def name=(name); end

  Sorbet.sig {returns(::T.untyped)}
  def path(); end

  Sorbet.sig do
    params(
      path: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def path=(path); end

  Sorbet.sig {returns(::T.untyped)}
  def secure(); end

  Sorbet.sig do
    params(
      val: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def secure=(val); end

  Sorbet.sig {returns(::T.untyped)}
  def to_s(); end

  Sorbet.sig {returns(::T.untyped)}
  def value(); end

  Sorbet.sig do
    params(
      val: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def value=(val); end

  Sorbet.sig do
    params(
      raw_cookie: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(raw_cookie); end
end

module CGI::QueryExtension
  Sorbet.sig do
    params(
      key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def [](key); end

  Sorbet.sig {returns(::T.untyped)}
  def accept(); end

  Sorbet.sig {returns(::T.untyped)}
  def accept_charset(); end

  Sorbet.sig {returns(::T.untyped)}
  def accept_encoding(); end

  Sorbet.sig {returns(::T.untyped)}
  def accept_language(); end

  Sorbet.sig {returns(::T.untyped)}
  def auth_type(); end

  Sorbet.sig {returns(::T.untyped)}
  def cache_control(); end

  Sorbet.sig {returns(::T.untyped)}
  def content_length(); end

  Sorbet.sig {returns(::T.untyped)}
  def content_type(); end

  Sorbet.sig {returns(::T.untyped)}
  def cookies(); end

  Sorbet.sig do
    params(
      cookies: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cookies=(cookies); end

  Sorbet.sig do
    params(
      is_large: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def create_body(is_large); end

  Sorbet.sig {returns(::T.untyped)}
  def files(); end

  Sorbet.sig {returns(::T.untyped)}
  def from(); end

  Sorbet.sig {returns(::T.untyped)}
  def gateway_interface(); end

  Sorbet.sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def has_key?(*args); end

  Sorbet.sig {returns(::T.untyped)}
  def host(); end

  Sorbet.sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def include?(*args); end

  Sorbet.sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def key?(*args); end

  Sorbet.sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def keys(*args); end

  Sorbet.sig {returns(::T.untyped)}
  def multipart?(); end

  Sorbet.sig {returns(::T.untyped)}
  def negotiate(); end

  Sorbet.sig {returns(::T.untyped)}
  def params(); end

  Sorbet.sig do
    params(
      hash: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def params=(hash); end

  Sorbet.sig {returns(::T.untyped)}
  def path_info(); end

  Sorbet.sig {returns(::T.untyped)}
  def path_translated(); end

  Sorbet.sig {returns(::T.untyped)}
  def pragma(); end

  Sorbet.sig {returns(::T.untyped)}
  def query_string(); end

  Sorbet.sig {returns(::T.untyped)}
  def raw_cookie(); end

  Sorbet.sig {returns(::T.untyped)}
  def raw_cookie2(); end

  Sorbet.sig {returns(::T.untyped)}
  def referer(); end

  Sorbet.sig {returns(::T.untyped)}
  def remote_addr(); end

  Sorbet.sig {returns(::T.untyped)}
  def remote_host(); end

  Sorbet.sig {returns(::T.untyped)}
  def remote_ident(); end

  Sorbet.sig {returns(::T.untyped)}
  def remote_user(); end

  Sorbet.sig {returns(::T.untyped)}
  def request_method(); end

  Sorbet.sig {returns(::T.untyped)}
  def script_name(); end

  Sorbet.sig {returns(::T.untyped)}
  def server_name(); end

  Sorbet.sig {returns(::T.untyped)}
  def server_port(); end

  Sorbet.sig {returns(::T.untyped)}
  def server_protocol(); end

  Sorbet.sig {returns(::T.untyped)}
  def server_software(); end

  Sorbet.sig {returns(::T.untyped)}
  def unescape_filename?(); end

  Sorbet.sig {returns(::T.untyped)}
  def user_agent(); end
end

module CGI::Util
  include ::CGI::Escape
  RFC822_DAYS = ::T.let(nil, ::T.untyped)
  RFC822_MONTHS = ::T.let(nil, ::T.untyped)
  TABLE_FOR_ESCAPE_HTML__ = ::T.let(nil, ::T.untyped)

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape(_); end

  Sorbet.sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escapeElement(string, *elements); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escapeHTML(_); end

  Sorbet.sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape_element(string, *elements); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape_html(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def h(_); end

  Sorbet.sig do
    params(
      string: ::T.untyped,
      shift: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pretty(string, shift=T.unsafe(nil)); end

  Sorbet.sig do
    params(
      time: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def rfc1123_date(time); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape(*_); end

  Sorbet.sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescapeElement(string, *elements); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescapeHTML(_); end

  Sorbet.sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape_element(string, *elements); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape_html(_); end
end

module CGI::Escape
  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escapeHTML(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescapeHTML(_); end
end

class CGI::InvalidEncoding < Exception
end
