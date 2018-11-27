# typed: true

class CGI
  include(CGI::Util)
  extend(CGI::Escape)
  extend(CGI::Util)

  class Cookie < ::Array
    def httponly=(val)
    end

    def name=(_)
    end

    def to_s()
    end

    def inspect()
    end

    def domain=(_)
    end

    def expires=(_)
    end

    def value()
    end

    def name()
    end

    def secure()
    end

    def value=(val)
    end

    def path=(_)
    end

    def httponly()
    end

    def secure=(val)
    end

    def domain()
    end

    def path()
    end

    def expires()
    end
  end

  module HtmlExtension
    def checkbox_group(name = _, *values)
    end

    def file_field(name = _, size = _, maxlength = _)
    end

    def form(method = _, action = _, enctype = _)
    end

    def hidden(name = _, value = _)
    end

    def html(attributes = _)
    end

    def image_button(src = _, name = _, alt = _)
    end

    def a(href = _)
    end

    def img(src = _, alt = _, width = _, height = _)
    end

    def multipart_form(action = _, enctype = _)
    end

    def password_field(name = _, value = _, size = _, maxlength = _)
    end

    def popup_menu(name = _, *values)
    end

    def base(href = _)
    end

    def radio_button(name = _, value = _, checked = _)
    end

    def radio_group(name = _, *values)
    end

    def scrolling_list(name = _, *values)
    end

    def submit(value = _, name = _)
    end

    def text_field(name = _, value = _, size = _, maxlength = _)
    end

    def textarea(name = _, cols = _, rows = _)
    end

    def blockquote(cite = _)
    end

    def caption(align = _)
    end

    def reset(value = _, name = _)
    end

    def checkbox(name = _, value = _, checked = _)
    end
  end

  class InvalidEncoding < ::Exception

  end

  CR = T.let(T.unsafe(nil), String)

  LF = T.let(T.unsafe(nil), String)

  EOL = T.let(T.unsafe(nil), String)

  REVISION = T.let(T.unsafe(nil), String)

  HTTP_STATUS = T.let(T.unsafe(nil), Hash)

  MAX_MULTIPART_COUNT = T.let(T.unsafe(nil), Integer)

  module QueryExtension
    def unescape_filename?()
    end

    def accept()
    end

    def user_agent()
    end

    def [](key)
    end

    def multipart?()
    end

    def accept_charset()
    end

    def include?(*args)
    end

    def server_port()
    end

    def auth_type()
    end

    def gateway_interface()
    end

    def path_info()
    end

    def path_translated()
    end

    def query_string()
    end

    def remote_addr()
    end

    def remote_host()
    end

    def remote_ident()
    end

    def remote_user()
    end

    def request_method()
    end

    def script_name()
    end

    def server_name()
    end

    def server_protocol()
    end

    def server_software()
    end

    def accept_encoding()
    end

    def accept_language()
    end

    def cache_control()
    end

    def negotiate()
    end

    def from()
    end

    def pragma()
    end

    def referer()
    end

    def cookies=(_)
    end

    def keys(*args)
    end

    def content_type()
    end

    def has_key?(*args)
    end

    def key?(*args)
    end

    def files()
    end

    def raw_cookie()
    end

    def raw_cookie2()
    end

    def cookies()
    end

    def params=(hash)
    end

    def content_length()
    end

    def params()
    end

    def host()
    end

    def create_body(is_large)
    end
  end

  module Util
    prepend(CGI::Escape)

    RFC822_MONTHS = T.let(T.unsafe(nil), Array)

    TABLE_FOR_ESCAPE_HTML__ = T.let(T.unsafe(nil), Hash)

    RFC822_DAYS = T.let(T.unsafe(nil), Array)

    def escape_html(_)
    end

    def unescape_html(_)
    end

    def escapeElement(string, *elements)
    end

    def unescapeElement(string, *elements)
    end

    def escape_element(string, *elements)
    end

    def unescape_element(string, *elements)
    end

    def pretty(string, shift = _)
    end

    def h(_)
    end

    def rfc1123_date(time)
    end
  end

  PATH_SEPARATOR = T.let(T.unsafe(nil), Hash)

  module Escape
    def unescapeHTML(_)
    end

    def escape(_)
    end

    def unescape(*_)
    end

    def escapeHTML(_)
    end
  end

  def header(options = _)
  end

  def out(options = _)
  end

  def http_header(options = _)
  end

  def nph?()
  end

  def accept_charset()
  end

  def print(*options)
  end
end
