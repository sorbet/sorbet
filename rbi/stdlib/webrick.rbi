# typed: strict

module WEBrick::AccessLog
  AGENT_LOG_FORMAT = T.let(_, T.untyped)
  CLF = T.let(_, T.untyped)
  CLF_TIME_FORMAT = T.let(_, T.untyped)
  COMBINED_LOG_FORMAT = T.let(_, T.untyped)
  COMMON_LOG_FORMAT = T.let(_, T.untyped)
  REFERER_LOG_FORMAT = T.let(_, T.untyped)

  Sorbet.sig(
    data: T.untyped,
  )
  .returns(T.untyped)
  def self.escape(data); end

  Sorbet.sig(
    format_string: T.untyped,
    params: T.untyped,
  )
  .returns(T.untyped)
  def self.format(format_string, params); end

  Sorbet.sig(
    config: T.untyped,
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def self.setup_params(config, req, res); end
end

class WEBrick::BasicLog
  DEBUG = T.let(_, T.untyped)
  ERROR = T.let(_, T.untyped)
  FATAL = T.let(_, T.untyped)
  INFO = T.let(_, T.untyped)
  WARN = T.let(_, T.untyped)

  Sorbet.sig(
    obj: T.untyped,
  )
  .returns(T.untyped)
  def <<(obj); end

  Sorbet.sig.returns(T.untyped)
  def close(); end

  Sorbet.sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def debug(msg); end

  Sorbet.sig.returns(T.untyped)
  def debug?(); end

  Sorbet.sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def error(msg); end

  Sorbet.sig.returns(T.untyped)
  def error?(); end

  Sorbet.sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def fatal(msg); end

  Sorbet.sig.returns(T.untyped)
  def fatal?(); end

  Sorbet.sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def info(msg); end

  Sorbet.sig.returns(T.untyped)
  def info?(); end

  Sorbet.sig(
    log_file: T.untyped,
    level: T.untyped,
  )
  .void
  def initialize(log_file=_, level=_); end

  Sorbet.sig.returns(T.untyped)
  def level(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def level=(_); end

  Sorbet.sig(
    level: T.untyped,
    data: T.untyped,
  )
  .returns(T.untyped)
  def log(level, data); end

  Sorbet.sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def warn(msg); end

  Sorbet.sig.returns(T.untyped)
  def warn?(); end
end

class WEBrick::Cookie
  Sorbet.sig.returns(T.untyped)
  def comment(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def comment=(_); end

  Sorbet.sig.returns(T.untyped)
  def domain(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def domain=(_); end

  Sorbet.sig.returns(T.untyped)
  def expires(); end

  Sorbet.sig(
    t: T.untyped,
  )
  .returns(T.untyped)
  def expires=(t); end

  Sorbet.sig(
    name: T.untyped,
    value: T.untyped,
  )
  .returns(T.untyped)
  def initialize(name, value); end

  Sorbet.sig.returns(T.untyped)
  def max_age(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def max_age=(_); end

  Sorbet.sig.returns(T.untyped)
  def name(); end

  Sorbet.sig.returns(T.untyped)
  def path(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def path=(_); end

  Sorbet.sig.returns(T.untyped)
  def secure(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def secure=(_); end

  Sorbet.sig.returns(T.untyped)
  def to_s(); end

  Sorbet.sig.returns(T.untyped)
  def value(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def value=(_); end

  Sorbet.sig.returns(T.untyped)
  def version(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def version=(_); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.parse(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_set_cookie(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_set_cookies(str); end
end

class WEBrick::Daemon
  Sorbet.sig.returns(T.untyped)
  def self.start(); end
end

class WEBrick::GenericServer
  Sorbet.sig(
    key: T.untyped,
  )
  .returns(T.untyped)
  def [](key); end

  Sorbet.sig.returns(T.untyped)
  def config(); end

  Sorbet.sig(
    config: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config=_, default=_); end

  Sorbet.sig(
    address: T.untyped,
    port: T.untyped,
  )
  .returns(T.untyped)
  def listen(address, port); end

  Sorbet.sig.returns(T.untyped)
  def listeners(); end

  Sorbet.sig.returns(T.untyped)
  def logger(); end

  Sorbet.sig(
    sock: T.untyped,
  )
  .returns(T.untyped)
  def run(sock); end

  Sorbet.sig.returns(T.untyped)
  def shutdown(); end

  Sorbet.sig(
  )
  .returns(T.untyped)
  def start(); end

  Sorbet.sig.returns(T.untyped)
  def status(); end

  Sorbet.sig.returns(T.untyped)
  def stop(); end

  Sorbet.sig.returns(T.untyped)
  def tokens(); end
end

module WEBrick::HTMLUtils
  Sorbet.sig(
    string: T.untyped,
  )
  .returns(T.untyped)
  def self.escape(string); end
end

module WEBrick::HTTPAuth
  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
    realm: T.untyped,
    req_field: T.untyped,
    res_field: T.untyped,
    err_type: T.untyped,
    block: T.untyped,
  )
  .returns(T.untyped)
  def self._basic_auth(req, res, realm, req_field, res_field, err_type, block); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
    realm: T.untyped,
  )
  .returns(T.untyped)
  def self.basic_auth(req, res, realm); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
    realm: T.untyped,
  )
  .returns(T.untyped)
  def self.proxy_basic_auth(req, res, realm); end
end

module WEBrick::HTTPAuth::Authenticator
  AuthScheme = T.let(_, T.untyped)
  RequestField = T.let(_, T.untyped)
  ResponseField = T.let(_, T.untyped)
  ResponseInfoField = T.let(_, T.untyped)

  Sorbet.sig.returns(T.untyped)
  def logger(); end

  Sorbet.sig.returns(T.untyped)
  def realm(); end

  Sorbet.sig.returns(T.untyped)
  def userdb(); end
end

class WEBrick::HTTPAuth::BasicAuth
  include WEBrick::HTTPAuth::Authenticator
  AuthScheme = T.let(_, T.untyped)

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def authenticate(req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def challenge(req, res); end

  Sorbet.sig(
    config: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config, default=_); end

  Sorbet.sig.returns(T.untyped)
  def logger(); end

  Sorbet.sig.returns(T.untyped)
  def realm(); end

  Sorbet.sig.returns(T.untyped)
  def userdb(); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def self.make_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::DigestAuth
  include WEBrick::HTTPAuth::Authenticator
  AuthScheme = T.let(_, T.untyped)
  MustParams = T.let(_, T.untyped)
  MustParamsAuth = T.let(_, T.untyped)

  Sorbet.sig.returns(T.untyped)
  def algorithm(); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def authenticate(req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
    stale: T.untyped,
  )
  .returns(T.untyped)
  def challenge(req, res, stale=_); end

  Sorbet.sig(
    config: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config, default=_); end

  Sorbet.sig.returns(T.untyped)
  def qop(); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def self.make_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::DigestAuth::OpaqueInfo < Struct
  Sorbet.sig.returns(T.untyped)
  def nc(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def nc=(_); end

  Sorbet.sig.returns(T.untyped)
  def nonce(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def nonce=(_); end

  Sorbet.sig.returns(T.untyped)
  def time(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def time=(_); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def self.[](*_); end

  Sorbet.sig.returns(T.untyped)
  def self.members(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def self.new(*_); end
end

class WEBrick::HTTPAuth::Htdigest
  include WEBrick::HTTPAuth::UserDB
  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
  )
  .returns(T.untyped)
  def delete_passwd(realm, user); end

  Sorbet.sig.returns(T.untyped)
  def each(); end

  Sorbet.sig(
    output: T.untyped,
  )
  .returns(T.untyped)
  def flush(output=_); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    reload_db: T.untyped,
  )
  .returns(T.untyped)
  def get_passwd(realm, user, reload_db); end

  Sorbet.sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def initialize(path); end

  Sorbet.sig.returns(T.untyped)
  def reload(); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def set_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::Htgroup
  Sorbet.sig(
    group: T.untyped,
    members: T.untyped,
  )
  .returns(T.untyped)
  def add(group, members); end

  Sorbet.sig(
    output: T.untyped,
  )
  .returns(T.untyped)
  def flush(output=_); end

  Sorbet.sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def initialize(path); end

  Sorbet.sig(
    group: T.untyped,
  )
  .returns(T.untyped)
  def members(group); end

  Sorbet.sig.returns(T.untyped)
  def reload(); end
end

class WEBrick::HTTPAuth::Htpasswd
  include WEBrick::HTTPAuth::UserDB
  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
  )
  .returns(T.untyped)
  def delete_passwd(realm, user); end

  Sorbet.sig.returns(T.untyped)
  def each(); end

  Sorbet.sig(
    output: T.untyped,
  )
  .returns(T.untyped)
  def flush(output=_); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    reload_db: T.untyped,
  )
  .returns(T.untyped)
  def get_passwd(realm, user, reload_db); end

  Sorbet.sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def initialize(path); end

  Sorbet.sig.returns(T.untyped)
  def reload(); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
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
  Sorbet.sig.returns(T.untyped)
  def auth_type(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def auth_type=(_); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    reload_db: T.untyped,
  )
  .returns(T.untyped)
  def get_passwd(realm, user, reload_db=_); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def make_passwd(realm, user, pass); end

  Sorbet.sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def set_passwd(realm, user, pass); end
end

class WEBrick::HTTPRequest
  BODY_CONTAINABLE_METHODS = T.let(_, T.untyped)
  MAX_URI_LENGTH = T.let(_, T.untyped)
  PrivateNetworkRegexp = T.let(_, T.untyped)

  Sorbet.sig(
    header_name: T.untyped,
  )
  .returns(T.untyped)
  def [](header_name); end

  Sorbet.sig.returns(T.untyped)
  def accept(); end

  Sorbet.sig.returns(T.untyped)
  def accept_charset(); end

  Sorbet.sig.returns(T.untyped)
  def accept_encoding(); end

  Sorbet.sig.returns(T.untyped)
  def accept_language(); end

  Sorbet.sig.returns(T.untyped)
  def addr(); end

  Sorbet.sig.returns(T.untyped)
  def attributes(); end

  Sorbet.sig(
  )
  .returns(T.untyped)
  def body(); end

  Sorbet.sig.returns(T.untyped)
  def content_length(); end

  Sorbet.sig.returns(T.untyped)
  def content_type(); end

  Sorbet.sig.returns(T.untyped)
  def continue(); end

  Sorbet.sig.returns(T.untyped)
  def cookies(); end

  Sorbet.sig.returns(T.untyped)
  def each(); end

  Sorbet.sig.returns(T.untyped)
  def fixup(); end

  Sorbet.sig.returns(T.untyped)
  def header(); end

  Sorbet.sig.returns(T.untyped)
  def host(); end

  Sorbet.sig.returns(T.untyped)
  def http_version(); end

  Sorbet.sig(
    config: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config); end

  Sorbet.sig.returns(T.untyped)
  def keep_alive(); end

  Sorbet.sig.returns(T.untyped)
  def keep_alive?(); end

  Sorbet.sig.returns(T.untyped)
  def meta_vars(); end

  Sorbet.sig(
    socket: T.untyped,
  )
  .returns(T.untyped)
  def parse(socket=_); end

  Sorbet.sig.returns(T.untyped)
  def path(); end

  Sorbet.sig.returns(T.untyped)
  def path_info(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def path_info=(_); end

  Sorbet.sig.returns(T.untyped)
  def peeraddr(); end

  Sorbet.sig.returns(T.untyped)
  def port(); end

  Sorbet.sig.returns(T.untyped)
  def query(); end

  Sorbet.sig.returns(T.untyped)
  def query_string(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def query_string=(_); end

  Sorbet.sig.returns(T.untyped)
  def raw_header(); end

  Sorbet.sig.returns(T.untyped)
  def remote_ip(); end

  Sorbet.sig.returns(T.untyped)
  def request_line(); end

  Sorbet.sig.returns(T.untyped)
  def request_method(); end

  Sorbet.sig.returns(T.untyped)
  def request_time(); end

  Sorbet.sig.returns(T.untyped)
  def request_uri(); end

  Sorbet.sig.returns(T.untyped)
  def script_name(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def script_name=(_); end

  Sorbet.sig.returns(T.untyped)
  def server_name(); end

  Sorbet.sig.returns(T.untyped)
  def ssl?(); end

  Sorbet.sig.returns(T.untyped)
  def to_s(); end

  Sorbet.sig.returns(T.untyped)
  def unparsed_uri(); end

  Sorbet.sig.returns(T.untyped)
  def user(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def user=(_); end
end

class WEBrick::HTTPResponse
  Sorbet.sig(
    field: T.untyped,
  )
  .returns(T.untyped)
  def [](field); end

  Sorbet.sig(
    field: T.untyped,
    value: T.untyped,
  )
  .returns(T.untyped)
  def []=(field, value); end

  Sorbet.sig.returns(T.untyped)
  def body(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def body=(_); end

  Sorbet.sig(
    val: T.untyped,
  )
  .returns(T.untyped)
  def chunked=(val); end

  Sorbet.sig.returns(T.untyped)
  def chunked?(); end

  Sorbet.sig.returns(T.untyped)
  def config(); end

  Sorbet.sig.returns(T.untyped)
  def content_length(); end

  Sorbet.sig(
    len: T.untyped,
  )
  .returns(T.untyped)
  def content_length=(len); end

  Sorbet.sig.returns(T.untyped)
  def content_type(); end

  Sorbet.sig(
    type: T.untyped,
  )
  .returns(T.untyped)
  def content_type=(type); end

  Sorbet.sig.returns(T.untyped)
  def cookies(); end

  Sorbet.sig.returns(T.untyped)
  def each(); end

  Sorbet.sig.returns(T.untyped)
  def filename(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def filename=(_); end

  Sorbet.sig.returns(T.untyped)
  def header(); end

  Sorbet.sig.returns(T.untyped)
  def http_version(); end

  Sorbet.sig(
    config: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config); end

  Sorbet.sig.returns(T.untyped)
  def keep_alive(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def keep_alive=(_); end

  Sorbet.sig.returns(T.untyped)
  def keep_alive?(); end

  Sorbet.sig.returns(T.untyped)
  def reason_phrase(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def reason_phrase=(_); end

  Sorbet.sig.returns(T.untyped)
  def request_http_version(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def request_http_version=(_); end

  Sorbet.sig.returns(T.untyped)
  def request_method(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def request_method=(_); end

  Sorbet.sig.returns(T.untyped)
  def request_uri(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def request_uri=(_); end

  Sorbet.sig(
    socket: T.untyped,
  )
  .returns(T.untyped)
  def send_body(socket); end

  Sorbet.sig(
    socket: T.untyped,
  )
  .returns(T.untyped)
  def send_header(socket); end

  Sorbet.sig(
    socket: T.untyped,
  )
  .returns(T.untyped)
  def send_response(socket); end

  Sorbet.sig.returns(T.untyped)
  def sent_size(); end

  Sorbet.sig(
    ex: T.untyped,
    backtrace: T.untyped,
  )
  .returns(T.untyped)
  def set_error(ex, backtrace=_); end

  Sorbet.sig(
    status: T.untyped,
    url: T.untyped,
  )
  .returns(T.untyped)
  def set_redirect(status, url); end

  Sorbet.sig.returns(T.untyped)
  def setup_header(); end

  Sorbet.sig.returns(T.untyped)
  def status(); end

  Sorbet.sig(
    status: T.untyped,
  )
  .returns(T.untyped)
  def status=(status); end

  Sorbet.sig.returns(T.untyped)
  def status_line(); end

  Sorbet.sig.returns(T.untyped)
  def to_s(); end
end

class WEBrick::HTTPServer < WEBrick::GenericServer
  Sorbet.sig(
    config: T.untyped,
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def access_log(config, req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_OPTIONS(req, res); end

  Sorbet.sig(
    config: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config=_, default=_); end

  Sorbet.sig(
    req: T.untyped,
  )
  .returns(T.untyped)
  def lookup_server(req); end

  Sorbet.sig(
    dir: T.untyped,
    servlet: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def mount(dir, servlet, *options); end

  Sorbet.sig(
    dir: T.untyped,
    proc: T.untyped,
  )
  .returns(T.untyped)
  def mount_proc(dir, proc=_); end

  Sorbet.sig(
    sock: T.untyped,
  )
  .returns(T.untyped)
  def run(sock); end

  Sorbet.sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def search_servlet(path); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def service(req, res); end

  Sorbet.sig(
    dir: T.untyped,
  )
  .returns(T.untyped)
  def umount(dir); end

  Sorbet.sig(
    dir: T.untyped,
  )
  .returns(T.untyped)
  def unmount(dir); end

  Sorbet.sig(
    server: T.untyped,
  )
  .returns(T.untyped)
  def virtual_host(server); end
end

class WEBrick::HTTPServer::MountTable
  Sorbet.sig(
    dir: T.untyped,
  )
  .returns(T.untyped)
  def [](dir); end

  Sorbet.sig(
    dir: T.untyped,
    val: T.untyped,
  )
  .returns(T.untyped)
  def []=(dir, val); end

  Sorbet.sig(
    dir: T.untyped,
  )
  .returns(T.untyped)
  def delete(dir); end

  Sorbet.sig.returns(T.untyped)
  def initialize(); end

  Sorbet.sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def scan(path); end
end

class WEBrick::HTTPServlet::AbstractServlet
  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_HEAD(req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_OPTIONS(req, res); end

  Sorbet.sig(
    server: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, *options); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def service(req, res); end

  Sorbet.sig(
    server: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def self.get_instance(server, *options); end
end

class WEBrick::HTTPServlet::CGIHandler < WEBrick::HTTPServlet::AbstractServlet
  CGIRunner = T.let(_, T.untyped)
  Ruby = T.let(_, T.untyped)

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_POST(req, res); end

  Sorbet.sig(
    server: T.untyped,
    name: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, name); end
end

class WEBrick::HTTPServlet::DefaultFileHandler < WEBrick::HTTPServlet::AbstractServlet
  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  Sorbet.sig(
    server: T.untyped,
    local_path: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, local_path); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
    filename: T.untyped,
    filesize: T.untyped,
  )
  .returns(T.untyped)
  def make_partial_content(req, res, filename, filesize); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
    mtime: T.untyped,
    etag: T.untyped,
  )
  .returns(T.untyped)
  def not_modified?(req, res, mtime, etag); end

  Sorbet.sig(
    range: T.untyped,
    filesize: T.untyped,
  )
  .returns(T.untyped)
  def prepare_range(range, filesize); end
end

class WEBrick::HTTPServlet::ERBHandler < WEBrick::HTTPServlet::AbstractServlet
  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_POST(req, res); end

  Sorbet.sig(
    server: T.untyped,
    name: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, name); end
end

class WEBrick::HTTPServlet::FileHandler < WEBrick::HTTPServlet::AbstractServlet
  HandlerTable = T.let(_, T.untyped)

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_OPTIONS(req, res); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_POST(req, res); end

  Sorbet.sig(
    server: T.untyped,
    root: T.untyped,
    options: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, root, options=_, default=_); end

  Sorbet.sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def service(req, res); end

  Sorbet.sig(
    suffix: T.untyped,
    handler: T.untyped,
  )
  .returns(T.untyped)
  def self.add_handler(suffix, handler); end

  Sorbet.sig(
    suffix: T.untyped,
  )
  .returns(T.untyped)
  def self.remove_handler(suffix); end
end

class WEBrick::HTTPServlet::ProcHandler < WEBrick::HTTPServlet::AbstractServlet
  Sorbet.sig(
    request: T.untyped,
    response: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(request, response); end

  Sorbet.sig(
    request: T.untyped,
    response: T.untyped,
  )
  .returns(T.untyped)
  def do_POST(request, response); end

  Sorbet.sig(
    server: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def get_instance(server, *options); end

  Sorbet.sig(
    proc: T.untyped,
  )
  .returns(T.untyped)
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

  Sorbet.sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.[](code); end

  Sorbet.sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.client_error?(code); end

  Sorbet.sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.error?(code); end

  Sorbet.sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.info?(code); end

  Sorbet.sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.reason_phrase(code); end

  Sorbet.sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.redirect?(code); end

  Sorbet.sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.server_error?(code); end

  Sorbet.sig(
    code: T.untyped,
  )
  .returns(T.untyped)
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
  Sorbet.sig.returns(T.untyped)
  def code(); end

  Sorbet.sig.returns(T.untyped)
  def reason_phrase(); end

  Sorbet.sig.returns(T.untyped)
  def to_i(); end

  Sorbet.sig.returns(T.untyped)
  def self.code(); end

  Sorbet.sig.returns(T.untyped)
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

  Sorbet.sig(
    str: T.untyped,
    regex: T.untyped,
  )
  .returns(T.untyped)
  def self._escape(str, regex); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self._make_regex(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self._make_regex!(str); end

  Sorbet.sig(
    str: T.untyped,
    regex: T.untyped,
  )
  .returns(T.untyped)
  def self._unescape(str, regex); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.dequote(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.escape(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.escape8bit(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.escape_form(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.escape_path(str); end

  Sorbet.sig(
    file: T.untyped,
  )
  .returns(T.untyped)
  def self.load_mime_types(file); end

  Sorbet.sig(
    filename: T.untyped,
    mime_tab: T.untyped,
  )
  .returns(T.untyped)
  def self.mime_type(filename, mime_tab); end

  Sorbet.sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def self.normalize_path(path); end

  Sorbet.sig(
    io: T.untyped,
    boundary: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_form_data(io, boundary); end

  Sorbet.sig(
    raw: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_header(raw); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_query(str); end

  Sorbet.sig(
    value: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_qvalues(value); end

  Sorbet.sig(
    ranges_specifier: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_range_header(ranges_specifier); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.quote(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.split_header_value(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.unescape(str); end

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.unescape_form(str); end
end

class WEBrick::HTTPUtils::FormData < String
  EmptyHeader = T.let(_, T.untyped)
  EmptyRawHeader = T.let(_, T.untyped)

  Sorbet.sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def <<(str); end

  Sorbet.sig(
    key: T.untyped,
  )
  .returns(T.untyped)
  def [](*key); end

  Sorbet.sig(
    data: T.untyped,
  )
  .returns(T.untyped)
  def append_data(data); end

  Sorbet.sig.returns(T.untyped)
  def each_data(); end

  Sorbet.sig.returns(T.untyped)
  def filename(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def filename=(_); end

  Sorbet.sig(
    args: T.untyped,
  )
  .returns(T.untyped)
  def initialize(*args); end

  Sorbet.sig.returns(T.untyped)
  def list(); end

  Sorbet.sig.returns(T.untyped)
  def name(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def name=(_); end

  Sorbet.sig.returns(T.untyped)
  def next_data(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def next_data=(_); end

  Sorbet.sig.returns(T.untyped)
  def to_ary(); end

  Sorbet.sig.returns(T.untyped)
  def to_s(); end
end

class WEBrick::HTTPVersion
  include Comparable
  Sorbet.sig(
    other: T.untyped,
  )
  .returns(T.untyped)
  def <=>(other); end

  Sorbet.sig(
    version: T.untyped,
  )
  .returns(T.untyped)
  def initialize(version); end

  Sorbet.sig.returns(T.untyped)
  def major(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def major=(_); end

  Sorbet.sig.returns(T.untyped)
  def minor(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def minor=(_); end

  Sorbet.sig.returns(T.untyped)
  def to_s(); end

  Sorbet.sig(
    version: T.untyped,
  )
  .returns(T.untyped)
  def self.convert(version); end
end

class WEBrick::Log < WEBrick::BasicLog
  Sorbet.sig(
    log_file: T.untyped,
    level: T.untyped,
  )
  .returns(T.untyped)
  def initialize(log_file=_, level=_); end

  Sorbet.sig(
    level: T.untyped,
    data: T.untyped,
  )
  .returns(T.untyped)
  def log(level, data); end

  Sorbet.sig.returns(T.untyped)
  def time_format(); end

  Sorbet.sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def time_format=(_); end
end

class WEBrick::SimpleServer
  Sorbet.sig.returns(T.untyped)
  def self.start(); end
end

module WEBrick::Utils
  RAND_CHARS = T.let(_, T.untyped)

  Sorbet.sig(
    address: T.untyped,
    port: T.untyped,
  )
  .returns(T.untyped)
  def self.create_listeners(address, port); end

  Sorbet.sig.returns(T.untyped)
  def self.getservername(); end

  Sorbet.sig(
    len: T.untyped,
  )
  .returns(T.untyped)
  def self.random_string(len); end

  Sorbet.sig(
    io: T.untyped,
  )
  .returns(T.untyped)
  def self.set_close_on_exec(io); end

  Sorbet.sig(
    io: T.untyped,
  )
  .returns(T.untyped)
  def self.set_non_blocking(io); end

  Sorbet.sig(
    user: T.untyped,
  )
  .returns(T.untyped)
  def self.su(user); end

  Sorbet.sig(
    seconds: T.untyped,
    exception: T.untyped,
  )
  .returns(T.untyped)
  def self.timeout(seconds, exception=_); end
end

class WEBrick::Utils::TimeoutHandler
  include Singleton
  extend Singleton::SingletonClassMethods
  TimeoutMutex = T.let(_, T.untyped)

  Sorbet.sig(
    thread: T.untyped,
    id: T.untyped,
  )
  .returns(T.untyped)
  def cancel(thread, id); end

  Sorbet.sig.returns(T.untyped)
  def initialize(); end

  Sorbet.sig(
    thread: T.untyped,
    id: T.untyped,
    exception: T.untyped,
  )
  .returns(T.untyped)
  def interrupt(thread, id, exception); end

  Sorbet.sig(
    thread: T.untyped,
    time: T.untyped,
    exception: T.untyped,
  )
  .returns(T.untyped)
  def register(thread, time, exception); end

  Sorbet.sig.returns(T.untyped)
  def terminate(); end

  Sorbet.sig(
    id: T.untyped,
  )
  .returns(T.untyped)
  def self.cancel(id); end

  Sorbet.sig.returns(T.untyped)
  def self.instance(); end

  Sorbet.sig(
    seconds: T.untyped,
    exception: T.untyped,
  )
  .returns(T.untyped)
  def self.register(seconds, exception); end

  Sorbet.sig.returns(T.untyped)
  def self.terminate(); end
end
