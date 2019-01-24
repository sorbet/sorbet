# typed: strict

module WEBrick
  CR = T.let(_, T.untyped)
  CRLF = T.let(_, T.untyped)
  LF = T.let(_, T.untyped)
  VERSION = T.let(_, T.untyped)
end

module WEBrick::AccessLog
  AGENT_LOG_FORMAT = T.let(_, T.untyped)
  CLF = T.let(_, T.untyped)
  CLF_TIME_FORMAT = T.let(_, T.untyped)
  COMBINED_LOG_FORMAT = T.let(_, T.untyped)
  COMMON_LOG_FORMAT = T.let(_, T.untyped)
  REFERER_LOG_FORMAT = T.let(_, T.untyped)

  sig do
    params(
      data: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.escape(data); end

  sig do
    params(
      format_string: T.untyped,
      params: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.format(format_string, params); end

  sig do
    params(
      config: T.untyped,
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.setup_params(config, req, res); end
end

class WEBrick::BasicLog
  DEBUG = T.let(_, T.untyped)
  ERROR = T.let(_, T.untyped)
  FATAL = T.let(_, T.untyped)
  INFO = T.let(_, T.untyped)
  WARN = T.let(_, T.untyped)

  sig do
    params(
      obj: T.untyped,
    )
    .returns(T.untyped)
  end
  def <<(obj); end

  sig {returns(T.untyped)}
  def close(); end

  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def debug(msg); end

  sig {returns(T.untyped)}
  def debug?(); end

  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def error(msg); end

  sig {returns(T.untyped)}
  def error?(); end

  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def fatal(msg); end

  sig {returns(T.untyped)}
  def fatal?(); end

  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def info(msg); end

  sig {returns(T.untyped)}
  def info?(); end

  sig do
    params(
      log_file: T.untyped,
      level: T.untyped,
    )
    .void
  end
  def initialize(log_file=T.unsafe(nil), level=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def level(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def level=(_); end

  sig do
    params(
      level: T.untyped,
      data: T.untyped,
    )
    .returns(T.untyped)
  end
  def log(level, data); end

  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def warn(msg); end

  sig {returns(T.untyped)}
  def warn?(); end
end

class WEBrick::Cookie
  sig {returns(T.untyped)}
  def comment(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def comment=(_); end

  sig {returns(T.untyped)}
  def domain(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def domain=(_); end

  sig {returns(T.untyped)}
  def expires(); end

  sig do
    params(
      t: T.untyped,
    )
    .returns(T.untyped)
  end
  def expires=(t); end

  sig do
    params(
      name: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(name, value); end

  sig {returns(T.untyped)}
  def max_age(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def max_age=(_); end

  sig {returns(T.untyped)}
  def name(); end

  sig {returns(T.untyped)}
  def path(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def path=(_); end

  sig {returns(T.untyped)}
  def secure(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def secure=(_); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def value(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def value=(_); end

  sig {returns(T.untyped)}
  def version(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def version=(_); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_set_cookie(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_set_cookies(str); end
end

class WEBrick::Daemon
  sig {returns(T.untyped)}
  def self.start(); end
end

class WEBrick::GenericServer
  sig do
    params(
      key: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](key); end

  sig {returns(T.untyped)}
  def config(); end

  sig do
    params(
      config: T.untyped,
      default: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(config=T.unsafe(nil), default=T.unsafe(nil)); end

  sig do
    params(
      address: T.untyped,
      port: T.untyped,
    )
    .returns(T.untyped)
  end
  def listen(address, port); end

  sig {returns(T.untyped)}
  def listeners(); end

  sig {returns(T.untyped)}
  def logger(); end

  sig do
    params(
      sock: T.untyped,
    )
    .returns(T.untyped)
  end
  def run(sock); end

  sig {returns(T.untyped)}
  def shutdown(); end

  sig do
    params(
    )
    .returns(T.untyped)
  end
  def start(); end

  sig {returns(T.untyped)}
  def status(); end

  sig {returns(T.untyped)}
  def stop(); end

  sig {returns(T.untyped)}
  def tokens(); end
end

module WEBrick::HTMLUtils
  sig do
    params(
      string: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.escape(string); end
end

module WEBrick::HTTPAuth
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
      realm: T.untyped,
      req_field: T.untyped,
      res_field: T.untyped,
      err_type: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def self._basic_auth(req, res, realm, req_field, res_field, err_type, block); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
      realm: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.basic_auth(req, res, realm); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
      realm: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.proxy_basic_auth(req, res, realm); end
end

module WEBrick::HTTPAuth::Authenticator
  AuthScheme = T.let(_, T.untyped)
  RequestField = T.let(_, T.untyped)
  ResponseField = T.let(_, T.untyped)
  ResponseInfoField = T.let(_, T.untyped)

  sig {returns(T.untyped)}
  def logger(); end

  sig {returns(T.untyped)}
  def realm(); end

  sig {returns(T.untyped)}
  def userdb(); end
end

class WEBrick::HTTPAuth::BasicAuth
  include WEBrick::HTTPAuth::Authenticator
  AuthScheme = T.let(_, T.untyped)

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def authenticate(req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def challenge(req, res); end

  sig do
    params(
      config: T.untyped,
      default: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(config, default=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def logger(); end

  sig {returns(T.untyped)}
  def realm(); end

  sig {returns(T.untyped)}
  def userdb(); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      pass: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.make_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::DigestAuth
  include WEBrick::HTTPAuth::Authenticator
  AuthScheme = T.let(_, T.untyped)
  MustParams = T.let(_, T.untyped)
  MustParamsAuth = T.let(_, T.untyped)

  sig {returns(T.untyped)}
  def algorithm(); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def authenticate(req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
      stale: T.untyped,
    )
    .returns(T.untyped)
  end
  def challenge(req, res, stale=T.unsafe(nil)); end

  sig do
    params(
      config: T.untyped,
      default: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(config, default=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def qop(); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      pass: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.make_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::DigestAuth::OpaqueInfo < Struct
  sig {returns(T.untyped)}
  def nc(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def nc=(_); end

  sig {returns(T.untyped)}
  def nonce(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def nonce=(_); end

  sig {returns(T.untyped)}
  def time(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def time=(_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end
end

class WEBrick::HTTPAuth::Htdigest
  include WEBrick::HTTPAuth::UserDB
  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
    )
    .returns(T.untyped)
  end
  def delete_passwd(realm, user); end

  sig {returns(T.untyped)}
  def each(); end

  sig do
    params(
      output: T.untyped,
    )
    .returns(T.untyped)
  end
  def flush(output=T.unsafe(nil)); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      reload_db: T.untyped,
    )
    .returns(T.untyped)
  end
  def get_passwd(realm, user, reload_db); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(path); end

  sig {returns(T.untyped)}
  def reload(); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      pass: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::Htgroup
  sig do
    params(
      group: T.untyped,
      members: T.untyped,
    )
    .returns(T.untyped)
  end
  def add(group, members); end

  sig do
    params(
      output: T.untyped,
    )
    .returns(T.untyped)
  end
  def flush(output=T.unsafe(nil)); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(path); end

  sig do
    params(
      group: T.untyped,
    )
    .returns(T.untyped)
  end
  def members(group); end

  sig {returns(T.untyped)}
  def reload(); end
end

class WEBrick::HTTPAuth::Htpasswd
  include WEBrick::HTTPAuth::UserDB
  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
    )
    .returns(T.untyped)
  end
  def delete_passwd(realm, user); end

  sig {returns(T.untyped)}
  def each(); end

  sig do
    params(
      output: T.untyped,
    )
    .returns(T.untyped)
  end
  def flush(output=T.unsafe(nil)); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      reload_db: T.untyped,
    )
    .returns(T.untyped)
  end
  def get_passwd(realm, user, reload_db); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(path); end

  sig {returns(T.untyped)}
  def reload(); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      pass: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_passwd(realm, user, pass); end
end

module WEBrick::HTTPAuth::ProxyAuthenticator
  InfoField = T.let(_, T.untyped)
  RequestField = T.let(_, T.untyped)
  ResponseField = T.let(_, T.untyped)

end

class WEBrick::HTTPAuth::ProxyBasicAuth < WEBrick::HTTPAuth::BasicAuth
  include WEBrick::HTTPAuth::ProxyAuthenticator
end

class WEBrick::HTTPAuth::ProxyDigestAuth < WEBrick::HTTPAuth::DigestAuth
  include WEBrick::HTTPAuth::ProxyAuthenticator
end

module WEBrick::HTTPAuth::UserDB
  sig {returns(T.untyped)}
  def auth_type(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def auth_type=(_); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      reload_db: T.untyped,
    )
    .returns(T.untyped)
  end
  def get_passwd(realm, user, reload_db=T.unsafe(nil)); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      pass: T.untyped,
    )
    .returns(T.untyped)
  end
  def make_passwd(realm, user, pass); end

  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      pass: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_passwd(realm, user, pass); end
end

class WEBrick::HTTPRequest
  BODY_CONTAINABLE_METHODS = T.let(_, T.untyped)
  MAX_URI_LENGTH = T.let(_, T.untyped)
  PrivateNetworkRegexp = T.let(_, T.untyped)

  sig do
    params(
      header_name: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](header_name); end

  sig {returns(T.untyped)}
  def accept(); end

  sig {returns(T.untyped)}
  def accept_charset(); end

  sig {returns(T.untyped)}
  def accept_encoding(); end

  sig {returns(T.untyped)}
  def accept_language(); end

  sig {returns(T.untyped)}
  def addr(); end

  sig {returns(T.untyped)}
  def attributes(); end

  sig do
    params(
    )
    .returns(T.untyped)
  end
  def body(); end

  sig {returns(T.untyped)}
  def content_length(); end

  sig {returns(T.untyped)}
  def content_type(); end

  sig {returns(T.untyped)}
  def continue(); end

  sig {returns(T.untyped)}
  def cookies(); end

  sig {returns(T.untyped)}
  def each(); end

  sig {returns(T.untyped)}
  def fixup(); end

  sig {returns(T.untyped)}
  def header(); end

  sig {returns(T.untyped)}
  def host(); end

  sig {returns(T.untyped)}
  def http_version(); end

  sig do
    params(
      config: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(config); end

  sig {returns(T.untyped)}
  def keep_alive(); end

  sig {returns(T.untyped)}
  def keep_alive?(); end

  sig {returns(T.untyped)}
  def meta_vars(); end

  sig do
    params(
      socket: T.untyped,
    )
    .returns(T.untyped)
  end
  def parse(socket=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def path(); end

  sig {returns(T.untyped)}
  def path_info(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def path_info=(_); end

  sig {returns(T.untyped)}
  def peeraddr(); end

  sig {returns(T.untyped)}
  def port(); end

  sig {returns(T.untyped)}
  def query(); end

  sig {returns(T.untyped)}
  def query_string(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def query_string=(_); end

  sig {returns(T.untyped)}
  def raw_header(); end

  sig {returns(T.untyped)}
  def remote_ip(); end

  sig {returns(T.untyped)}
  def request_line(); end

  sig {returns(T.untyped)}
  def request_method(); end

  sig {returns(T.untyped)}
  def request_time(); end

  sig {returns(T.untyped)}
  def request_uri(); end

  sig {returns(T.untyped)}
  def script_name(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def script_name=(_); end

  sig {returns(T.untyped)}
  def server_name(); end

  sig {returns(T.untyped)}
  def ssl?(); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def unparsed_uri(); end

  sig {returns(T.untyped)}
  def user(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def user=(_); end
end

class WEBrick::HTTPResponse
  sig do
    params(
      field: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](field); end

  sig do
    params(
      field: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def []=(field, value); end

  sig {returns(T.untyped)}
  def body(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def body=(_); end

  sig do
    params(
      val: T.untyped,
    )
    .returns(T.untyped)
  end
  def chunked=(val); end

  sig {returns(T.untyped)}
  def chunked?(); end

  sig {returns(T.untyped)}
  def config(); end

  sig {returns(T.untyped)}
  def content_length(); end

  sig do
    params(
      len: T.untyped,
    )
    .returns(T.untyped)
  end
  def content_length=(len); end

  sig {returns(T.untyped)}
  def content_type(); end

  sig do
    params(
      type: T.untyped,
    )
    .returns(T.untyped)
  end
  def content_type=(type); end

  sig {returns(T.untyped)}
  def cookies(); end

  sig {returns(T.untyped)}
  def each(); end

  sig {returns(T.untyped)}
  def filename(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def filename=(_); end

  sig {returns(T.untyped)}
  def header(); end

  sig {returns(T.untyped)}
  def http_version(); end

  sig do
    params(
      config: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(config); end

  sig {returns(T.untyped)}
  def keep_alive(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def keep_alive=(_); end

  sig {returns(T.untyped)}
  def keep_alive?(); end

  sig {returns(T.untyped)}
  def reason_phrase(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def reason_phrase=(_); end

  sig {returns(T.untyped)}
  def request_http_version(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def request_http_version=(_); end

  sig {returns(T.untyped)}
  def request_method(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def request_method=(_); end

  sig {returns(T.untyped)}
  def request_uri(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def request_uri=(_); end

  sig do
    params(
      socket: T.untyped,
    )
    .returns(T.untyped)
  end
  def send_body(socket); end

  sig do
    params(
      socket: T.untyped,
    )
    .returns(T.untyped)
  end
  def send_header(socket); end

  sig do
    params(
      socket: T.untyped,
    )
    .returns(T.untyped)
  end
  def send_response(socket); end

  sig {returns(T.untyped)}
  def sent_size(); end

  sig do
    params(
      ex: T.untyped,
      backtrace: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_error(ex, backtrace=T.unsafe(nil)); end

  sig do
    params(
      status: T.untyped,
      url: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_redirect(status, url); end

  sig {returns(T.untyped)}
  def setup_header(); end

  sig {returns(T.untyped)}
  def status(); end

  sig do
    params(
      status: T.untyped,
    )
    .returns(T.untyped)
  end
  def status=(status); end

  sig {returns(T.untyped)}
  def status_line(); end

  sig {returns(T.untyped)}
  def to_s(); end
end

class WEBrick::HTTPServer < WEBrick::GenericServer
  sig do
    params(
      config: T.untyped,
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def access_log(config, req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_OPTIONS(req, res); end

  sig do
    params(
      config: T.untyped,
      default: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(config=T.unsafe(nil), default=T.unsafe(nil)); end

  sig do
    params(
      req: T.untyped,
    )
    .returns(T.untyped)
  end
  def lookup_server(req); end

  sig do
    params(
      dir: T.untyped,
      servlet: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def mount(dir, servlet, *options); end

  sig do
    params(
      dir: T.untyped,
      proc: T.untyped,
    )
    .returns(T.untyped)
  end
  def mount_proc(dir, proc=T.unsafe(nil)); end

  sig do
    params(
      sock: T.untyped,
    )
    .returns(T.untyped)
  end
  def run(sock); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def search_servlet(path); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def service(req, res); end

  sig do
    params(
      dir: T.untyped,
    )
    .returns(T.untyped)
  end
  def umount(dir); end

  sig do
    params(
      dir: T.untyped,
    )
    .returns(T.untyped)
  end
  def unmount(dir); end

  sig do
    params(
      server: T.untyped,
    )
    .returns(T.untyped)
  end
  def virtual_host(server); end
end

class WEBrick::HTTPServer::MountTable
  sig do
    params(
      dir: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](dir); end

  sig do
    params(
      dir: T.untyped,
      val: T.untyped,
    )
    .returns(T.untyped)
  end
  def []=(dir, val); end

  sig do
    params(
      dir: T.untyped,
    )
    .returns(T.untyped)
  end
  def delete(dir); end

  sig {returns(T.untyped)}
  def initialize(); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def scan(path); end
end

class WEBrick::HTTPServlet::AbstractServlet
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_GET(req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_HEAD(req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_OPTIONS(req, res); end

  sig do
    params(
      server: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(server, *options); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def service(req, res); end

  sig do
    params(
      server: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.get_instance(server, *options); end
end

class WEBrick::HTTPServlet::CGIHandler < WEBrick::HTTPServlet::AbstractServlet
  CGIRunner = T.let(_, T.untyped)
  Ruby = T.let(_, T.untyped)

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_GET(req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_POST(req, res); end

  sig do
    params(
      server: T.untyped,
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(server, name); end
end

class WEBrick::HTTPServlet::DefaultFileHandler < WEBrick::HTTPServlet::AbstractServlet
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_GET(req, res); end

  sig do
    params(
      server: T.untyped,
      local_path: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(server, local_path); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
      filename: T.untyped,
      filesize: T.untyped,
    )
    .returns(T.untyped)
  end
  def make_partial_content(req, res, filename, filesize); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
      mtime: T.untyped,
      etag: T.untyped,
    )
    .returns(T.untyped)
  end
  def not_modified?(req, res, mtime, etag); end

  sig do
    params(
      range: T.untyped,
      filesize: T.untyped,
    )
    .returns(T.untyped)
  end
  def prepare_range(range, filesize); end
end

class WEBrick::HTTPServlet::ERBHandler < WEBrick::HTTPServlet::AbstractServlet
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_GET(req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_POST(req, res); end

  sig do
    params(
      server: T.untyped,
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(server, name); end
end

class WEBrick::HTTPServlet::FileHandler < WEBrick::HTTPServlet::AbstractServlet
  HandlerTable = T.let(_, T.untyped)

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_GET(req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_OPTIONS(req, res); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_POST(req, res); end

  sig do
    params(
      server: T.untyped,
      root: T.untyped,
      options: T.untyped,
      default: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(server, root, options=T.unsafe(nil), default=T.unsafe(nil)); end

  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def service(req, res); end

  sig do
    params(
      suffix: T.untyped,
      handler: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.add_handler(suffix, handler); end

  sig do
    params(
      suffix: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_handler(suffix); end
end

class WEBrick::HTTPServlet::ProcHandler < WEBrick::HTTPServlet::AbstractServlet
  sig do
    params(
      request: T.untyped,
      response: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_GET(request, response); end

  sig do
    params(
      request: T.untyped,
      response: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_POST(request, response); end

  sig do
    params(
      server: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def get_instance(server, *options); end

  sig do
    params(
      proc: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(proc); end
end

module WEBrick::HTTPStatus
  CodeToError = T.let(_, T.untyped)
  RC_ACCEPTED = T.let(_, T.untyped)
  RC_BAD_GATEWAY = T.let(_, T.untyped)
  RC_BAD_REQUEST = T.let(_, T.untyped)
  RC_CONFLICT = T.let(_, T.untyped)
  RC_CONTINUE = T.let(_, T.untyped)
  RC_CREATED = T.let(_, T.untyped)
  RC_EXPECTATION_FAILED = T.let(_, T.untyped)
  RC_FAILED_DEPENDENCY = T.let(_, T.untyped)
  RC_FORBIDDEN = T.let(_, T.untyped)
  RC_FOUND = T.let(_, T.untyped)
  RC_GATEWAY_TIMEOUT = T.let(_, T.untyped)
  RC_GONE = T.let(_, T.untyped)
  RC_HTTP_VERSION_NOT_SUPPORTED = T.let(_, T.untyped)
  RC_INSUFFICIENT_STORAGE = T.let(_, T.untyped)
  RC_INTERNAL_SERVER_ERROR = T.let(_, T.untyped)
  RC_LENGTH_REQUIRED = T.let(_, T.untyped)
  RC_LOCKED = T.let(_, T.untyped)
  RC_METHOD_NOT_ALLOWED = T.let(_, T.untyped)
  RC_MOVED_PERMANENTLY = T.let(_, T.untyped)
  RC_MULTIPLE_CHOICES = T.let(_, T.untyped)
  RC_MULTI_STATUS = T.let(_, T.untyped)
  RC_NETWORK_AUTHENTICATION_REQUIRED = T.let(_, T.untyped)
  RC_NON_AUTHORITATIVE_INFORMATION = T.let(_, T.untyped)
  RC_NOT_ACCEPTABLE = T.let(_, T.untyped)
  RC_NOT_FOUND = T.let(_, T.untyped)
  RC_NOT_IMPLEMENTED = T.let(_, T.untyped)
  RC_NOT_MODIFIED = T.let(_, T.untyped)
  RC_NO_CONTENT = T.let(_, T.untyped)
  RC_OK = T.let(_, T.untyped)
  RC_PARTIAL_CONTENT = T.let(_, T.untyped)
  RC_PAYMENT_REQUIRED = T.let(_, T.untyped)
  RC_PRECONDITION_FAILED = T.let(_, T.untyped)
  RC_PRECONDITION_REQUIRED = T.let(_, T.untyped)
  RC_PROXY_AUTHENTICATION_REQUIRED = T.let(_, T.untyped)
  RC_REQUEST_ENTITY_TOO_LARGE = T.let(_, T.untyped)
  RC_REQUEST_HEADER_FIELDS_TOO_LARGE = T.let(_, T.untyped)
  RC_REQUEST_RANGE_NOT_SATISFIABLE = T.let(_, T.untyped)
  RC_REQUEST_TIMEOUT = T.let(_, T.untyped)
  RC_REQUEST_URI_TOO_LARGE = T.let(_, T.untyped)
  RC_RESET_CONTENT = T.let(_, T.untyped)
  RC_SEE_OTHER = T.let(_, T.untyped)
  RC_SERVICE_UNAVAILABLE = T.let(_, T.untyped)
  RC_SWITCHING_PROTOCOLS = T.let(_, T.untyped)
  RC_TEMPORARY_REDIRECT = T.let(_, T.untyped)
  RC_TOO_MANY_REQUESTS = T.let(_, T.untyped)
  RC_UNAUTHORIZED = T.let(_, T.untyped)
  RC_UNAVAILABLE_FOR_LEGAL_REASONS = T.let(_, T.untyped)
  RC_UNPROCESSABLE_ENTITY = T.let(_, T.untyped)
  RC_UNSUPPORTED_MEDIA_TYPE = T.let(_, T.untyped)
  RC_UPGRADE_REQUIRED = T.let(_, T.untyped)
  RC_USE_PROXY = T.let(_, T.untyped)
  StatusMessage = T.let(_, T.untyped)

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](code); end

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.client_error?(code); end

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.error?(code); end

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.info?(code); end

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.reason_phrase(code); end

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.redirect?(code); end

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.server_error?(code); end

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.success?(code); end
end

class WEBrick::HTTPStatus::Accepted < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::BadGateway < WEBrick::HTTPStatus::ServerError
end

class WEBrick::HTTPStatus::BadRequest < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::ClientError < WEBrick::HTTPStatus::Error
end

class WEBrick::HTTPStatus::Conflict < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::Continue < WEBrick::HTTPStatus::Info
end

class WEBrick::HTTPStatus::Created < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::Error < WEBrick::HTTPStatus::Status
end

class WEBrick::HTTPStatus::ExpectationFailed < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::FailedDependency < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::Forbidden < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::Found < WEBrick::HTTPStatus::Redirect
end

class WEBrick::HTTPStatus::GatewayTimeout < WEBrick::HTTPStatus::ServerError
end

class WEBrick::HTTPStatus::Gone < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::HTTPVersionNotSupported < WEBrick::HTTPStatus::ServerError
end

class WEBrick::HTTPStatus::Info < WEBrick::HTTPStatus::Status
end

class WEBrick::HTTPStatus::InsufficientStorage < WEBrick::HTTPStatus::ServerError
end

class WEBrick::HTTPStatus::InternalServerError < WEBrick::HTTPStatus::ServerError
end

class WEBrick::HTTPStatus::LengthRequired < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::Locked < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::MethodNotAllowed < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::MovedPermanently < WEBrick::HTTPStatus::Redirect
end

class WEBrick::HTTPStatus::MultiStatus < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::MultipleChoices < WEBrick::HTTPStatus::Redirect
end

class WEBrick::HTTPStatus::NetworkAuthenticationRequired < WEBrick::HTTPStatus::ServerError
end

class WEBrick::HTTPStatus::NoContent < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::NonAuthoritativeInformation < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::NotAcceptable < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::NotFound < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::NotImplemented < WEBrick::HTTPStatus::ServerError
end

class WEBrick::HTTPStatus::NotModified < WEBrick::HTTPStatus::Redirect
end

class WEBrick::HTTPStatus::OK < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::PartialContent < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::PaymentRequired < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::PreconditionFailed < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::PreconditionRequired < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::ProxyAuthenticationRequired < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::Redirect < WEBrick::HTTPStatus::Status
end

class WEBrick::HTTPStatus::RequestEntityTooLarge < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::RequestHeaderFieldsTooLarge < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::RequestRangeNotSatisfiable < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::RequestTimeout < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::RequestURITooLarge < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::ResetContent < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::SeeOther < WEBrick::HTTPStatus::Redirect
end

class WEBrick::HTTPStatus::ServerError < WEBrick::HTTPStatus::Error
end

class WEBrick::HTTPStatus::ServiceUnavailable < WEBrick::HTTPStatus::ServerError
end

class WEBrick::HTTPStatus::Status < StandardError
  sig {returns(T.untyped)}
  def code(); end

  sig {returns(T.untyped)}
  def reason_phrase(); end

  sig {returns(T.untyped)}
  def to_i(); end

  sig {returns(T.untyped)}
  def self.code(); end

  sig {returns(T.untyped)}
  def self.reason_phrase(); end
end

class WEBrick::HTTPStatus::Success < WEBrick::HTTPStatus::Status
end

class WEBrick::HTTPStatus::SwitchingProtocols < WEBrick::HTTPStatus::Info
end

class WEBrick::HTTPStatus::TemporaryRedirect < WEBrick::HTTPStatus::Redirect
end

class WEBrick::HTTPStatus::TooManyRequests < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::Unauthorized < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::UnavailableForLegalReasons < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::UnprocessableEntity < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::UnsupportedMediaType < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::UpgradeRequired < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::UseProxy < WEBrick::HTTPStatus::Redirect
end

module WEBrick::HTTPUtils
  DefaultMimeTypes = T.let(_, T.untyped)
  ESCAPED = T.let(_, T.untyped)
  NONASCII = T.let(_, T.untyped)
  UNESCAPED = T.let(_, T.untyped)
  UNESCAPED_FORM = T.let(_, T.untyped)
  UNESCAPED_PCHAR = T.let(_, T.untyped)

  sig do
    params(
      str: T.untyped,
      regex: T.untyped,
    )
    .returns(T.untyped)
  end
  def self._escape(str, regex); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self._make_regex(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self._make_regex!(str); end

  sig do
    params(
      str: T.untyped,
      regex: T.untyped,
    )
    .returns(T.untyped)
  end
  def self._unescape(str, regex); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.dequote(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.escape(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.escape8bit(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.escape_form(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.escape_path(str); end

  sig do
    params(
      file: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.load_mime_types(file); end

  sig do
    params(
      filename: T.untyped,
      mime_tab: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mime_type(filename, mime_tab); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.normalize_path(path); end

  sig do
    params(
      io: T.untyped,
      boundary: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_form_data(io, boundary); end

  sig do
    params(
      raw: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_header(raw); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_query(str); end

  sig do
    params(
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_qvalues(value); end

  sig do
    params(
      ranges_specifier: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_range_header(ranges_specifier); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.quote(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.split_header_value(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.unescape(str); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.unescape_form(str); end
end

class WEBrick::HTTPUtils::FormData < String
  EmptyHeader = T.let(_, T.untyped)
  EmptyRawHeader = T.let(_, T.untyped)

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def <<(str); end

  sig do
    params(
      key: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](*key); end

  sig do
    params(
      data: T.untyped,
    )
    .returns(T.untyped)
  end
  def append_data(data); end

  sig {returns(T.untyped)}
  def each_data(); end

  sig {returns(T.untyped)}
  def filename(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def filename=(_); end

  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(*args); end

  sig {returns(T.untyped)}
  def list(); end

  sig {returns(T.untyped)}
  def name(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def name=(_); end

  sig {returns(T.untyped)}
  def next_data(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def next_data=(_); end

  sig {returns(T.untyped)}
  def to_ary(); end

  sig {returns(T.untyped)}
  def to_s(); end
end

class WEBrick::HTTPVersion
  include Comparable
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def <=>(other); end

  sig do
    params(
      version: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(version); end

  sig {returns(T.untyped)}
  def major(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def major=(_); end

  sig {returns(T.untyped)}
  def minor(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def minor=(_); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig do
    params(
      version: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.convert(version); end
end

class WEBrick::Log < WEBrick::BasicLog
  sig do
    params(
      log_file: T.untyped,
      level: T.untyped,
    )
    .returns(T.untyped)
  end
  def initialize(log_file=T.unsafe(nil), level=T.unsafe(nil)); end

  sig do
    params(
      level: T.untyped,
      data: T.untyped,
    )
    .returns(T.untyped)
  end
  def log(level, data); end

  sig {returns(T.untyped)}
  def time_format(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def time_format=(_); end
end

class WEBrick::SimpleServer
  sig {returns(T.untyped)}
  def self.start(); end
end

module WEBrick::Utils
  RAND_CHARS = T.let(_, T.untyped)

  sig do
    params(
      address: T.untyped,
      port: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.create_listeners(address, port); end

  sig {returns(T.untyped)}
  def self.getservername(); end

  sig do
    params(
      len: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.random_string(len); end

  sig do
    params(
      io: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.set_close_on_exec(io); end

  sig do
    params(
      io: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.set_non_blocking(io); end

  sig do
    params(
      user: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.su(user); end

  sig do
    params(
      seconds: T.untyped,
      exception: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.timeout(seconds, exception=T.unsafe(nil)); end
end

class WEBrick::Utils::TimeoutHandler
  include Singleton
  extend Singleton::SingletonClassMethods
  TimeoutMutex = T.let(_, T.untyped)

  sig do
    params(
      thread: T.untyped,
      id: T.untyped,
    )
    .returns(T.untyped)
  end
  def cancel(thread, id); end

  sig {returns(T.untyped)}
  def initialize(); end

  sig do
    params(
      thread: T.untyped,
      id: T.untyped,
      exception: T.untyped,
    )
    .returns(T.untyped)
  end
  def interrupt(thread, id, exception); end

  sig do
    params(
      thread: T.untyped,
      time: T.untyped,
      exception: T.untyped,
    )
    .returns(T.untyped)
  end
  def register(thread, time, exception); end

  sig {returns(T.untyped)}
  def terminate(); end

  sig do
    params(
      id: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cancel(id); end

  sig {returns(T.untyped)}
  def self.instance(); end

  sig do
    params(
      seconds: T.untyped,
      exception: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.register(seconds, exception); end

  sig {returns(T.untyped)}
  def self.terminate(); end
end
