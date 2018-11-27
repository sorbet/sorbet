# typed: true

module URI
  include(URI::RFC2396_REGEXP)
  extend(URI::Escape)

  PORT = T.let(T.unsafe(nil), Regexp)

  class HTTPS < ::URI::HTTP
    DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
  end

  USERINFO = T.let(T.unsafe(nil), Regexp)

  class LDAP < ::URI::Generic
    SCOPE_ONE = T.let(T.unsafe(nil), String)

    SCOPE_SUB = T.let(T.unsafe(nil), String)

    SCOPE_BASE = T.let(T.unsafe(nil), String)

    DEFAULT_PORT = T.let(T.unsafe(nil), Integer)

    COMPONENT = T.let(T.unsafe(nil), Array)

    SCOPE = T.let(T.unsafe(nil), Array)

    def extensions=(val)
    end

    def dn()
    end

    def scope()
    end

    def filter()
    end

    def dn=(val)
    end

    def attributes=(val)
    end

    def scope=(val)
    end

    def attributes()
    end

    def filter=(val)
    end

    def hierarchical?()
    end

    def extensions()
    end
  end

  WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)

  class Generic
    include(URI)
    include(URI::RFC2396_REGEXP)

    COMPONENT = T.let(T.unsafe(nil), Array)

    def ==(oth)
    end

    def eql?(oth)
    end

    def default_port()
    end

    def component()
    end

    def scheme()
    end

    def parser()
    end

    def -(oth)
    end

    def +(oth)
    end

    def inspect()
    end

    def scheme=(v)
    end

    def userinfo=(userinfo)
    end

    def hostname=(v)
    end

    def port=(v)
    end

    def path=(v)
    end

    def query=(v)
    end

    def opaque=(v)
    end

    def password()
    end

    def user()
    end

    def user=(user)
    end

    def registry=(v)
    end

    def opaque()
    end

    def userinfo()
    end

    def port()
    end

    def registry()
    end

    def query()
    end

    def fragment()
    end

    def absolute?()
    end

    def absolute()
    end

    def relative?()
    end

    def fragment=(v)
    end

    def password=(password)
    end

    def select(*components)
    end

    def hierarchical?()
    end

    def merge!(oth)
    end

    def merge(oth)
    end

    def route_from(oth)
    end

    def normalize!()
    end

    def hostname()
    end

    def to_s()
    end

    def normalize()
    end

    def hash()
    end

    def route_to(oth)
    end

    def coerce(oth)
    end

    def find_proxy(env = _)
    end

    def host()
    end

    def host=(v)
    end

    def path()
    end
  end

  class MailTo < ::URI::Generic
    EMAIL_REGEXP = T.let(T.unsafe(nil), Regexp)

    COMPONENT = T.let(T.unsafe(nil), Array)

    HEADER_REGEXP = T.let(T.unsafe(nil), Regexp)

    def to=(v)
    end

    def headers=(v)
    end

    def to_mailtext()
    end

    def to_s()
    end

    def to_rfc822text()
    end

    def to()
    end

    def headers()
    end
  end

  ESCAPED = T.let(T.unsafe(nil), Regexp)

  SCHEME = T.let(T.unsafe(nil), Regexp)

  ABS_PATH = T.let(T.unsafe(nil), Regexp)

  REL_PATH = T.let(T.unsafe(nil), Regexp)

  ABS_URI = T.let(T.unsafe(nil), Regexp)

  URI_REF = T.let(T.unsafe(nil), Regexp)

  REL_URI_REF = T.let(T.unsafe(nil), Regexp)

  OPAQUE = T.let(T.unsafe(nil), Regexp)

  REGISTRY = T.let(T.unsafe(nil), Regexp)

  REL_URI = T.let(T.unsafe(nil), Regexp)

  REGEXP = URI::RFC2396_REGEXP

  Parser = URI::RFC2396_Parser

  RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)

  class RFC3986_Parser
    RFC3986_relative_ref = T.let(T.unsafe(nil), Regexp)

    RFC3986_URI = T.let(T.unsafe(nil), Regexp)

    def split(uri)
    end

    def regexp()
    end

    def inspect()
    end

    def parse(uri)
    end

    def join(*uris)
    end
  end

  DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)

  ABS_URI_REF = T.let(T.unsafe(nil), Regexp)

  UNSAFE = T.let(T.unsafe(nil), Regexp)

  VERSION_CODE = T.let(T.unsafe(nil), String)

  class HTTP < ::URI::Generic
    DEFAULT_PORT = T.let(T.unsafe(nil), Integer)

    COMPONENT = T.let(T.unsafe(nil), Array)

    def request_uri()
    end
  end

  module Escape
    def escape(*arg)
    end

    def encode(*arg)
    end

    def unescape(*arg)
    end

    def decode(*arg)
    end
  end

  VERSION = T.let(T.unsafe(nil), String)

  class LDAPS < ::URI::LDAP
    DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
  end

  module Util

  end

  QUERY = T.let(T.unsafe(nil), Regexp)

  FRAGMENT = T.let(T.unsafe(nil), Regexp)

  class Error < ::StandardError

  end

  class InvalidComponentError < ::URI::Error

  end

  class BadURIError < ::URI::Error

  end

  TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)

  TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)

  class InvalidURIError < ::URI::Error

  end

  HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)

  class FTP < ::URI::Generic
    TYPECODE = T.let(T.unsafe(nil), Array)

    TYPECODE_PREFIX = T.let(T.unsafe(nil), String)

    DEFAULT_PORT = T.let(T.unsafe(nil), Integer)

    COMPONENT = T.let(T.unsafe(nil), Array)

    def path()
    end

    def merge(oth)
    end

    def typecode()
    end

    def to_s()
    end

    def typecode=(typecode)
    end
  end

  HOST = T.let(T.unsafe(nil), Regexp)
end
