# typed: strict

module WEBrick::AccessLog
  AGENT_LOG_FORMAT = _
  CLF = _
  CLF_TIME_FORMAT = _
  COMBINED_LOG_FORMAT = _
  COMMON_LOG_FORMAT = _
  REFERER_LOG_FORMAT = _

  sig(
    data: T.untyped,
  )
  .returns(T.untyped)
  def self.escape(data); end

  sig(
    format_string: T.untyped,
    params: T.untyped,
  )
  .returns(T.untyped)
  def self.format(format_string, params); end

  sig(
    config: T.untyped,
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def self.setup_params(config, req, res); end
end

class WEBrick::BasicLog
  DEBUG = _
  ERROR = _
  FATAL = _
  INFO = _
  WARN = _

  sig(
    obj: T.untyped,
  )
  .returns(T.untyped)
  def <<(obj); end

  sig.returns(T.untyped)
  def close(); end

  sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def debug(msg); end

  sig.returns(T.untyped)
  def debug?(); end

  sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def error(msg); end

  sig.returns(T.untyped)
  def error?(); end

  sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def fatal(msg); end

  sig.returns(T.untyped)
  def fatal?(); end

  sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def info(msg); end

  sig.returns(T.untyped)
  def info?(); end

  sig(
    log_file: T.untyped,
    level: T.untyped,
  )
  .void
  def initialize(log_file=_, level=_); end

  sig.returns(T.untyped)
  def level(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def level=(_); end

  sig(
    level: T.untyped,
    data: T.untyped,
  )
  .returns(T.untyped)
  def log(level, data); end

  sig(
    msg: T.untyped,
  )
  .returns(T.untyped)
  def warn(msg); end

  sig.returns(T.untyped)
  def warn?(); end
end

class WEBrick::Cookie
  sig.returns(T.untyped)
  def comment(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def comment=(_); end

  sig.returns(T.untyped)
  def domain(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def domain=(_); end

  sig.returns(T.untyped)
  def expires(); end

  sig(
    t: T.untyped,
  )
  .returns(T.untyped)
  def expires=(t); end

  sig(
    name: T.untyped,
    value: T.untyped,
  )
  .returns(T.untyped)
  def initialize(name, value); end

  sig.returns(T.untyped)
  def max_age(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def max_age=(_); end

  sig.returns(T.untyped)
  def name(); end

  sig.returns(T.untyped)
  def path(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def path=(_); end

  sig.returns(T.untyped)
  def secure(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def secure=(_); end

  sig.returns(T.untyped)
  def to_s(); end

  sig.returns(T.untyped)
  def value(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def value=(_); end

  sig.returns(T.untyped)
  def version(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def version=(_); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.parse(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_set_cookie(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_set_cookies(str); end
end

class WEBrick::Daemon
  sig.returns(T.untyped)
  def self.start(); end
end

class WEBrick::GenericServer
  sig(
    key: T.untyped,
  )
  .returns(T.untyped)
  def [](key); end

  sig.returns(T.untyped)
  def config(); end

  sig(
    config: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config=_, default=_); end

  sig(
    address: T.untyped,
    port: T.untyped,
  )
  .returns(T.untyped)
  def listen(address, port); end

  sig.returns(T.untyped)
  def listeners(); end

  sig.returns(T.untyped)
  def logger(); end

  sig(
    sock: T.untyped,
  )
  .returns(T.untyped)
  def run(sock); end

  sig.returns(T.untyped)
  def shutdown(); end

  sig(
  )
  .returns(T.untyped)
  def start(); end

  sig.returns(T.untyped)
  def status(); end

  sig.returns(T.untyped)
  def stop(); end

  sig.returns(T.untyped)
  def tokens(); end
end

module WEBrick::HTMLUtils
  sig(
    string: T.untyped,
  )
  .returns(T.untyped)
  def self.escape(string); end
end

module WEBrick::HTTPAuth
  sig(
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

  sig(
    req: T.untyped,
    res: T.untyped,
    realm: T.untyped,
  )
  .returns(T.untyped)
  def self.basic_auth(req, res, realm); end

  sig(
    req: T.untyped,
    res: T.untyped,
    realm: T.untyped,
  )
  .returns(T.untyped)
  def self.proxy_basic_auth(req, res, realm); end
end

module WEBrick::HTTPAuth::Authenticator
  AuthScheme = _
  RequestField = _
  ResponseField = _
  ResponseInfoField = _

  sig.returns(T.untyped)
  def logger(); end

  sig.returns(T.untyped)
  def realm(); end

  sig.returns(T.untyped)
  def userdb(); end
end

class WEBrick::HTTPAuth::BasicAuth
  include WEBrick::HTTPAuth::Authenticator
  AuthScheme = _

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def authenticate(req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def challenge(req, res); end

  sig(
    config: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config, default=_); end

  sig.returns(T.untyped)
  def logger(); end

  sig.returns(T.untyped)
  def realm(); end

  sig.returns(T.untyped)
  def userdb(); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def self.make_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::DigestAuth
  include WEBrick::HTTPAuth::Authenticator
  AuthScheme = _
  MustParams = _
  MustParamsAuth = _

  sig.returns(T.untyped)
  def algorithm(); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def authenticate(req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
    stale: T.untyped,
  )
  .returns(T.untyped)
  def challenge(req, res, stale=_); end

  sig(
    config: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config, default=_); end

  sig.returns(T.untyped)
  def qop(); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def self.make_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::DigestAuth::OpaqueInfo < Struct
  sig.returns(T.untyped)
  def nc(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def nc=(_); end

  sig.returns(T.untyped)
  def nonce(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def nonce=(_); end

  sig.returns(T.untyped)
  def time(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def time=(_); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def self.[](*_); end

  sig.returns(T.untyped)
  def self.members(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def self.new(*_); end
end

class WEBrick::HTTPAuth::Htdigest
  include WEBrick::HTTPAuth::UserDB
  sig(
    realm: T.untyped,
    user: T.untyped,
  )
  .returns(T.untyped)
  def delete_passwd(realm, user); end

  sig.returns(T.untyped)
  def each(); end

  sig(
    output: T.untyped,
  )
  .returns(T.untyped)
  def flush(output=_); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    reload_db: T.untyped,
  )
  .returns(T.untyped)
  def get_passwd(realm, user, reload_db); end

  sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def initialize(path); end

  sig.returns(T.untyped)
  def reload(); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def set_passwd(realm, user, pass); end
end

class WEBrick::HTTPAuth::Htgroup
  sig(
    group: T.untyped,
    members: T.untyped,
  )
  .returns(T.untyped)
  def add(group, members); end

  sig(
    output: T.untyped,
  )
  .returns(T.untyped)
  def flush(output=_); end

  sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def initialize(path); end

  sig(
    group: T.untyped,
  )
  .returns(T.untyped)
  def members(group); end

  sig.returns(T.untyped)
  def reload(); end
end

class WEBrick::HTTPAuth::Htpasswd
  include WEBrick::HTTPAuth::UserDB
  sig(
    realm: T.untyped,
    user: T.untyped,
  )
  .returns(T.untyped)
  def delete_passwd(realm, user); end

  sig.returns(T.untyped)
  def each(); end

  sig(
    output: T.untyped,
  )
  .returns(T.untyped)
  def flush(output=_); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    reload_db: T.untyped,
  )
  .returns(T.untyped)
  def get_passwd(realm, user, reload_db); end

  sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def initialize(path); end

  sig.returns(T.untyped)
  def reload(); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def set_passwd(realm, user, pass); end
end

module WEBrick::HTTPAuth::ProxyAuthenticator
  InfoField = _
  RequestField = _
  ResponseField = _

end

class WEBrick::HTTPAuth::ProxyBasicAuth < WEBrick::HTTPAuth::BasicAuth
  include WEBrick::HTTPAuth::ProxyAuthenticator
end

class WEBrick::HTTPAuth::ProxyDigestAuth < WEBrick::HTTPAuth::DigestAuth
  include WEBrick::HTTPAuth::ProxyAuthenticator
end

module WEBrick::HTTPAuth::UserDB
  sig.returns(T.untyped)
  def auth_type(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def auth_type=(_); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    reload_db: T.untyped,
  )
  .returns(T.untyped)
  def get_passwd(realm, user, reload_db=_); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def make_passwd(realm, user, pass); end

  sig(
    realm: T.untyped,
    user: T.untyped,
    pass: T.untyped,
  )
  .returns(T.untyped)
  def set_passwd(realm, user, pass); end
end

class WEBrick::HTTPRequest
  BODY_CONTAINABLE_METHODS = _
  MAX_URI_LENGTH = _
  PrivateNetworkRegexp = _

  sig(
    header_name: T.untyped,
  )
  .returns(T.untyped)
  def [](header_name); end

  sig.returns(T.untyped)
  def accept(); end

  sig.returns(T.untyped)
  def accept_charset(); end

  sig.returns(T.untyped)
  def accept_encoding(); end

  sig.returns(T.untyped)
  def accept_language(); end

  sig.returns(T.untyped)
  def addr(); end

  sig.returns(T.untyped)
  def attributes(); end

  sig(
  )
  .returns(T.untyped)
  def body(); end

  sig.returns(T.untyped)
  def content_length(); end

  sig.returns(T.untyped)
  def content_type(); end

  sig.returns(T.untyped)
  def continue(); end

  sig.returns(T.untyped)
  def cookies(); end

  sig.returns(T.untyped)
  def each(); end

  sig.returns(T.untyped)
  def fixup(); end

  sig.returns(T.untyped)
  def header(); end

  sig.returns(T.untyped)
  def host(); end

  sig.returns(T.untyped)
  def http_version(); end

  sig(
    config: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config); end

  sig.returns(T.untyped)
  def keep_alive(); end

  sig.returns(T.untyped)
  def keep_alive?(); end

  sig.returns(T.untyped)
  def meta_vars(); end

  sig(
    socket: T.untyped,
  )
  .returns(T.untyped)
  def parse(socket=_); end

  sig.returns(T.untyped)
  def path(); end

  sig.returns(T.untyped)
  def path_info(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def path_info=(_); end

  sig.returns(T.untyped)
  def peeraddr(); end

  sig.returns(T.untyped)
  def port(); end

  sig.returns(T.untyped)
  def query(); end

  sig.returns(T.untyped)
  def query_string(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def query_string=(_); end

  sig.returns(T.untyped)
  def raw_header(); end

  sig.returns(T.untyped)
  def remote_ip(); end

  sig.returns(T.untyped)
  def request_line(); end

  sig.returns(T.untyped)
  def request_method(); end

  sig.returns(T.untyped)
  def request_time(); end

  sig.returns(T.untyped)
  def request_uri(); end

  sig.returns(T.untyped)
  def script_name(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def script_name=(_); end

  sig.returns(T.untyped)
  def server_name(); end

  sig.returns(T.untyped)
  def ssl?(); end

  sig.returns(T.untyped)
  def to_s(); end

  sig.returns(T.untyped)
  def unparsed_uri(); end

  sig.returns(T.untyped)
  def user(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def user=(_); end
end

class WEBrick::HTTPResponse
  sig(
    field: T.untyped,
  )
  .returns(T.untyped)
  def [](field); end

  sig(
    field: T.untyped,
    value: T.untyped,
  )
  .returns(T.untyped)
  def []=(field, value); end

  sig.returns(T.untyped)
  def body(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def body=(_); end

  sig(
    val: T.untyped,
  )
  .returns(T.untyped)
  def chunked=(val); end

  sig.returns(T.untyped)
  def chunked?(); end

  sig.returns(T.untyped)
  def config(); end

  sig.returns(T.untyped)
  def content_length(); end

  sig(
    len: T.untyped,
  )
  .returns(T.untyped)
  def content_length=(len); end

  sig.returns(T.untyped)
  def content_type(); end

  sig(
    type: T.untyped,
  )
  .returns(T.untyped)
  def content_type=(type); end

  sig.returns(T.untyped)
  def cookies(); end

  sig.returns(T.untyped)
  def each(); end

  sig.returns(T.untyped)
  def filename(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def filename=(_); end

  sig.returns(T.untyped)
  def header(); end

  sig.returns(T.untyped)
  def http_version(); end

  sig(
    config: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config); end

  sig.returns(T.untyped)
  def keep_alive(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def keep_alive=(_); end

  sig.returns(T.untyped)
  def keep_alive?(); end

  sig.returns(T.untyped)
  def reason_phrase(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def reason_phrase=(_); end

  sig.returns(T.untyped)
  def request_http_version(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def request_http_version=(_); end

  sig.returns(T.untyped)
  def request_method(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def request_method=(_); end

  sig.returns(T.untyped)
  def request_uri(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def request_uri=(_); end

  sig(
    socket: T.untyped,
  )
  .returns(T.untyped)
  def send_body(socket); end

  sig(
    socket: T.untyped,
  )
  .returns(T.untyped)
  def send_header(socket); end

  sig(
    socket: T.untyped,
  )
  .returns(T.untyped)
  def send_response(socket); end

  sig.returns(T.untyped)
  def sent_size(); end

  sig(
    ex: T.untyped,
    backtrace: T.untyped,
  )
  .returns(T.untyped)
  def set_error(ex, backtrace=_); end

  sig(
    status: T.untyped,
    url: T.untyped,
  )
  .returns(T.untyped)
  def set_redirect(status, url); end

  sig.returns(T.untyped)
  def setup_header(); end

  sig.returns(T.untyped)
  def status(); end

  sig(
    status: T.untyped,
  )
  .returns(T.untyped)
  def status=(status); end

  sig.returns(T.untyped)
  def status_line(); end

  sig.returns(T.untyped)
  def to_s(); end
end

class WEBrick::HTTPServer < WEBrick::GenericServer
  sig(
    config: T.untyped,
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def access_log(config, req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_OPTIONS(req, res); end

  sig(
    config: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(config=_, default=_); end

  sig(
    req: T.untyped,
  )
  .returns(T.untyped)
  def lookup_server(req); end

  sig(
    dir: T.untyped,
    servlet: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def mount(dir, servlet, *options); end

  sig(
    dir: T.untyped,
    proc: T.untyped,
  )
  .returns(T.untyped)
  def mount_proc(dir, proc=_); end

  sig(
    sock: T.untyped,
  )
  .returns(T.untyped)
  def run(sock); end

  sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def search_servlet(path); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def service(req, res); end

  sig(
    dir: T.untyped,
  )
  .returns(T.untyped)
  def umount(dir); end

  sig(
    dir: T.untyped,
  )
  .returns(T.untyped)
  def unmount(dir); end

  sig(
    server: T.untyped,
  )
  .returns(T.untyped)
  def virtual_host(server); end
end

class WEBrick::HTTPServer::MountTable
  sig(
    dir: T.untyped,
  )
  .returns(T.untyped)
  def [](dir); end

  sig(
    dir: T.untyped,
    val: T.untyped,
  )
  .returns(T.untyped)
  def []=(dir, val); end

  sig(
    dir: T.untyped,
  )
  .returns(T.untyped)
  def delete(dir); end

  sig.returns(T.untyped)
  def initialize(); end

  sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def scan(path); end
end

class WEBrick::HTTPServlet::AbstractServlet
  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_HEAD(req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_OPTIONS(req, res); end

  sig(
    server: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, *options); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def service(req, res); end

  sig(
    server: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def self.get_instance(server, *options); end
end

class WEBrick::HTTPServlet::CGIHandler < WEBrick::HTTPServlet::AbstractServlet
  CGIRunner = _
  Ruby = _

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_POST(req, res); end

  sig(
    server: T.untyped,
    name: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, name); end
end

class WEBrick::HTTPServlet::DefaultFileHandler < WEBrick::HTTPServlet::AbstractServlet
  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  sig(
    server: T.untyped,
    local_path: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, local_path); end

  sig(
    req: T.untyped,
    res: T.untyped,
    filename: T.untyped,
    filesize: T.untyped,
  )
  .returns(T.untyped)
  def make_partial_content(req, res, filename, filesize); end

  sig(
    req: T.untyped,
    res: T.untyped,
    mtime: T.untyped,
    etag: T.untyped,
  )
  .returns(T.untyped)
  def not_modified?(req, res, mtime, etag); end

  sig(
    range: T.untyped,
    filesize: T.untyped,
  )
  .returns(T.untyped)
  def prepare_range(range, filesize); end
end

class WEBrick::HTTPServlet::ERBHandler < WEBrick::HTTPServlet::AbstractServlet
  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_POST(req, res); end

  sig(
    server: T.untyped,
    name: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, name); end
end

class WEBrick::HTTPServlet::FileHandler < WEBrick::HTTPServlet::AbstractServlet
  HandlerTable = _

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_OPTIONS(req, res); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def do_POST(req, res); end

  sig(
    server: T.untyped,
    root: T.untyped,
    options: T.untyped,
    default: T.untyped,
  )
  .returns(T.untyped)
  def initialize(server, root, options=_, default=_); end

  sig(
    req: T.untyped,
    res: T.untyped,
  )
  .returns(T.untyped)
  def service(req, res); end

  sig(
    suffix: T.untyped,
    handler: T.untyped,
  )
  .returns(T.untyped)
  def self.add_handler(suffix, handler); end

  sig(
    suffix: T.untyped,
  )
  .returns(T.untyped)
  def self.remove_handler(suffix); end
end

class WEBrick::HTTPServlet::ProcHandler < WEBrick::HTTPServlet::AbstractServlet
  sig(
    request: T.untyped,
    response: T.untyped,
  )
  .returns(T.untyped)
  def do_GET(request, response); end

  sig(
    request: T.untyped,
    response: T.untyped,
  )
  .returns(T.untyped)
  def do_POST(request, response); end

  sig(
    server: T.untyped,
    options: T.untyped,
  )
  .returns(T.untyped)
  def get_instance(server, *options); end

  sig(
    proc: T.untyped,
  )
  .returns(T.untyped)
  def initialize(proc); end
end

module WEBrick::HTTPStatus
  CodeToError = _
  RC_ACCEPTED = _
  RC_BAD_GATEWAY = _
  RC_BAD_REQUEST = _
  RC_CONFLICT = _
  RC_CONTINUE = _
  RC_CREATED = _
  RC_EXPECTATION_FAILED = _
  RC_FAILED_DEPENDENCY = _
  RC_FORBIDDEN = _
  RC_FOUND = _
  RC_GATEWAY_TIMEOUT = _
  RC_GONE = _
  RC_HTTP_VERSION_NOT_SUPPORTED = _
  RC_INSUFFICIENT_STORAGE = _
  RC_INTERNAL_SERVER_ERROR = _
  RC_LENGTH_REQUIRED = _
  RC_LOCKED = _
  RC_METHOD_NOT_ALLOWED = _
  RC_MOVED_PERMANENTLY = _
  RC_MULTIPLE_CHOICES = _
  RC_MULTI_STATUS = _
  RC_NETWORK_AUTHENTICATION_REQUIRED = _
  RC_NON_AUTHORITATIVE_INFORMATION = _
  RC_NOT_ACCEPTABLE = _
  RC_NOT_FOUND = _
  RC_NOT_IMPLEMENTED = _
  RC_NOT_MODIFIED = _
  RC_NO_CONTENT = _
  RC_OK = _
  RC_PARTIAL_CONTENT = _
  RC_PAYMENT_REQUIRED = _
  RC_PRECONDITION_FAILED = _
  RC_PRECONDITION_REQUIRED = _
  RC_PROXY_AUTHENTICATION_REQUIRED = _
  RC_REQUEST_ENTITY_TOO_LARGE = _
  RC_REQUEST_HEADER_FIELDS_TOO_LARGE = _
  RC_REQUEST_RANGE_NOT_SATISFIABLE = _
  RC_REQUEST_TIMEOUT = _
  RC_REQUEST_URI_TOO_LARGE = _
  RC_RESET_CONTENT = _
  RC_SEE_OTHER = _
  RC_SERVICE_UNAVAILABLE = _
  RC_SWITCHING_PROTOCOLS = _
  RC_TEMPORARY_REDIRECT = _
  RC_TOO_MANY_REQUESTS = _
  RC_UNAUTHORIZED = _
  RC_UNAVAILABLE_FOR_LEGAL_REASONS = _
  RC_UNPROCESSABLE_ENTITY = _
  RC_UNSUPPORTED_MEDIA_TYPE = _
  RC_UPGRADE_REQUIRED = _
  RC_USE_PROXY = _
  StatusMessage = _

  sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.[](code); end

  sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.client_error?(code); end

  sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.error?(code); end

  sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.info?(code); end

  sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.reason_phrase(code); end

  sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.redirect?(code); end

  sig(
    code: T.untyped,
  )
  .returns(T.untyped)
  def self.server_error?(code); end

  sig(
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
  sig.returns(T.untyped)
  def code(); end

  sig.returns(T.untyped)
  def reason_phrase(); end

  sig.returns(T.untyped)
  def to_i(); end

  sig.returns(T.untyped)
  def self.code(); end

  sig.returns(T.untyped)
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
  DefaultMimeTypes = _
  ESCAPED = _
  NONASCII = _
  UNESCAPED = _
  UNESCAPED_FORM = _
  UNESCAPED_PCHAR = _

  sig(
    str: T.untyped,
    regex: T.untyped,
  )
  .returns(T.untyped)
  def self._escape(str, regex); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self._make_regex(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self._make_regex!(str); end

  sig(
    str: T.untyped,
    regex: T.untyped,
  )
  .returns(T.untyped)
  def self._unescape(str, regex); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.dequote(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.escape(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.escape8bit(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.escape_form(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.escape_path(str); end

  sig(
    file: T.untyped,
  )
  .returns(T.untyped)
  def self.load_mime_types(file); end

  sig(
    filename: T.untyped,
    mime_tab: T.untyped,
  )
  .returns(T.untyped)
  def self.mime_type(filename, mime_tab); end

  sig(
    path: T.untyped,
  )
  .returns(T.untyped)
  def self.normalize_path(path); end

  sig(
    io: T.untyped,
    boundary: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_form_data(io, boundary); end

  sig(
    raw: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_header(raw); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_query(str); end

  sig(
    value: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_qvalues(value); end

  sig(
    ranges_specifier: T.untyped,
  )
  .returns(T.untyped)
  def self.parse_range_header(ranges_specifier); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.quote(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.split_header_value(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.unescape(str); end

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def self.unescape_form(str); end
end

class WEBrick::HTTPUtils::FormData < String
  EmptyHeader = _
  EmptyRawHeader = _

  sig(
    str: T.untyped,
  )
  .returns(T.untyped)
  def <<(str); end

  sig(
    key: T.untyped,
  )
  .returns(T.untyped)
  def [](*key); end

  sig(
    data: T.untyped,
  )
  .returns(T.untyped)
  def append_data(data); end

  sig.returns(T.untyped)
  def each_data(); end

  sig.returns(T.untyped)
  def filename(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def filename=(_); end

  sig(
    args: T.untyped,
  )
  .returns(T.untyped)
  def initialize(*args); end

  sig.returns(T.untyped)
  def list(); end

  sig.returns(T.untyped)
  def name(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def name=(_); end

  sig.returns(T.untyped)
  def next_data(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def next_data=(_); end

  sig.returns(T.untyped)
  def to_ary(); end

  sig.returns(T.untyped)
  def to_s(); end
end

class WEBrick::HTTPVersion
  include Comparable
  sig(
    other: T.untyped,
  )
  .returns(T.untyped)
  def <=>(other); end

  sig(
    version: T.untyped,
  )
  .returns(T.untyped)
  def initialize(version); end

  sig.returns(T.untyped)
  def major(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def major=(_); end

  sig.returns(T.untyped)
  def minor(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def minor=(_); end

  sig.returns(T.untyped)
  def to_s(); end

  sig(
    version: T.untyped,
  )
  .returns(T.untyped)
  def self.convert(version); end
end

class WEBrick::Log < WEBrick::BasicLog
  sig(
    log_file: T.untyped,
    level: T.untyped,
  )
  .returns(T.untyped)
  def initialize(log_file=_, level=_); end

  sig(
    level: T.untyped,
    data: T.untyped,
  )
  .returns(T.untyped)
  def log(level, data); end

  sig.returns(T.untyped)
  def time_format(); end

  sig(
    _: T.untyped,
  )
  .returns(T.untyped)
  def time_format=(_); end
end

class WEBrick::SimpleServer
  sig.returns(T.untyped)
  def self.start(); end
end

module WEBrick::Utils
  RAND_CHARS = _

  sig(
    address: T.untyped,
    port: T.untyped,
  )
  .returns(T.untyped)
  def self.create_listeners(address, port); end

  sig.returns(T.untyped)
  def self.getservername(); end

  sig(
    len: T.untyped,
  )
  .returns(T.untyped)
  def self.random_string(len); end

  sig(
    io: T.untyped,
  )
  .returns(T.untyped)
  def self.set_close_on_exec(io); end

  sig(
    io: T.untyped,
  )
  .returns(T.untyped)
  def self.set_non_blocking(io); end

  sig(
    user: T.untyped,
  )
  .returns(T.untyped)
  def self.su(user); end

  sig(
    seconds: T.untyped,
    exception: T.untyped,
  )
  .returns(T.untyped)
  def self.timeout(seconds, exception=_); end
end

class WEBrick::Utils::TimeoutHandler
  include Singleton
  extend Singleton::SingletonClassMethods
  TimeoutMutex = _

  sig(
    thread: T.untyped,
    id: T.untyped,
  )
  .returns(T.untyped)
  def cancel(thread, id); end

  sig.returns(T.untyped)
  def initialize(); end

  sig(
    thread: T.untyped,
    id: T.untyped,
    exception: T.untyped,
  )
  .returns(T.untyped)
  def interrupt(thread, id, exception); end

  sig(
    thread: T.untyped,
    time: T.untyped,
    exception: T.untyped,
  )
  .returns(T.untyped)
  def register(thread, time, exception); end

  sig.returns(T.untyped)
  def terminate(); end

  sig(
    id: T.untyped,
  )
  .returns(T.untyped)
  def self.cancel(id); end

  sig.returns(T.untyped)
  def self.instance(); end

  sig(
    seconds: T.untyped,
    exception: T.untyped,
  )
  .returns(T.untyped)
  def self.register(seconds, exception); end

  sig.returns(T.untyped)
  def self.terminate(); end
end
