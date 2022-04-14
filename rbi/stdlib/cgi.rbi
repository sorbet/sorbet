# typed: __STDLIB_INTERNAL

# ## Overview
#
# The Common Gateway Interface
# ([`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)) is a simple protocol
# for passing an HTTP request from a web server to a standalone program, and
# returning the output to the web browser. Basically, a
# [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) program is called with
# the parameters of the request passed in either in the environment (GET) or via
# $stdin (POST), and everything it prints to $stdout is returned to the client.
#
# This file holds the [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
# class. This class provides functionality for retrieving HTTP request
# parameters, managing cookies, and generating HTML output.
#
# The file
# [`CGI::Session`](https://docs.ruby-lang.org/en/2.7.0/CGI/Session.html)
# provides session management functionality; see that class for more details.
#
# See http://www.w3.org/CGI/ for more information on the
# [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) protocol.
#
# ## Introduction
#
# [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) is a large class,
# providing several categories of methods, many of which are mixed in from other
# modules. Some of the documentation is in this class, some in the modules
# [`CGI::QueryExtension`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html)
# and
# [`CGI::HtmlExtension`](https://docs.ruby-lang.org/en/2.7.0/CGI/HtmlExtension.html).
# See [`CGI::Cookie`](https://docs.ruby-lang.org/en/2.7.0/CGI/Cookie.html) for
# specific information on handling cookies, and cgi/session.rb
# ([`CGI::Session`](https://docs.ruby-lang.org/en/2.7.0/CGI/Session.html)) for
# information on sessions.
#
# For queries, [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) provides
# methods to get at environmental variables, parameters, cookies, and multipart
# request data. For responses,
# [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) provides methods for
# writing output and generating HTML.
#
# Read on for more details. Examples are provided at the bottom.
#
# ## Queries
#
# The [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) class dynamically
# mixes in parameter and cookie-parsing functionality,  environmental variable
# access, and support for parsing multipart requests (including uploaded files)
# from the
# [`CGI::QueryExtension`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html)
# module.
#
# ### Environmental Variables
#
# The standard [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
# environmental variables are available as read-only attributes of a
# [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) object. The following is
# a list of these variables:
#
# ```ruby
# AUTH_TYPE               HTTP_HOST          REMOTE_IDENT
# CONTENT_LENGTH          HTTP_NEGOTIATE     REMOTE_USER
# CONTENT_TYPE            HTTP_PRAGMA        REQUEST_METHOD
# GATEWAY_INTERFACE       HTTP_REFERER       SCRIPT_NAME
# HTTP_ACCEPT             HTTP_USER_AGENT    SERVER_NAME
# HTTP_ACCEPT_CHARSET     PATH_INFO          SERVER_PORT
# HTTP_ACCEPT_ENCODING    PATH_TRANSLATED    SERVER_PROTOCOL
# HTTP_ACCEPT_LANGUAGE    QUERY_STRING       SERVER_SOFTWARE
# HTTP_CACHE_CONTROL      REMOTE_ADDR
# HTTP_FROM               REMOTE_HOST
# ```
#
# For each of these variables, there is a corresponding attribute with the same
# name, except all lower case and without a preceding HTTP\_. `content_length`
# and `server_port` are integers; the rest are strings.
#
# ### Parameters
#
# The method params() returns a hash of all parameters in the request as
# name/value-list pairs, where the value-list is an
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of one or more
# values. The [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) object
# itself also behaves as a hash of parameter names to values, but only returns a
# single value (as a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)) for each
# parameter name.
#
# For instance, suppose the request contains the parameter "favourite\_colours"
# with the multiple values "blue" and "green". The following behavior would
# occur:
#
# ```ruby
# cgi.params["favourite_colours"]  # => ["blue", "green"]
# cgi["favourite_colours"]         # => "blue"
# ```
#
# If a parameter does not exist, the former method will return an empty array,
# the latter an empty string. The simplest way to test for existence of a
# parameter is by the has\_key? method.
#
# ### Cookies
#
# HTTP Cookies are automatically parsed from the request. They are available
# from the cookies() accessor, which returns a hash from cookie name to
# [`CGI::Cookie`](https://docs.ruby-lang.org/en/2.7.0/CGI/Cookie.html) object.
#
# ### Multipart requests
#
# If a request's method is POST and its content type is multipart/form-data,
# then it may contain uploaded files. These are stored by the QueryExtension
# module in the parameters of the request. The parameter name is the name
# attribute of the file input field, as usual. However, the value is not a
# string, but an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object,
# either an IOString for small files, or a
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) for larger
# ones. This object also has the additional singleton methods:
#
# local\_path()
# :   the path of the uploaded file on the local filesystem
# original\_filename()
# :   the name of the file on the client computer
# content\_type()
# :   the content type of the file
#
#
# ## Responses
#
# The [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) class provides
# methods for sending header and content output to the HTTP client, and mixes in
# methods for programmatic HTML generation from
# [`CGI::HtmlExtension`](https://docs.ruby-lang.org/en/2.7.0/CGI/HtmlExtension.html)
# and CGI::TagMaker modules. The precise version of HTML to use for HTML
# generation is specified at object creation time.
#
# ### Writing output
#
# The simplest way to send output to the HTTP client is using the
# [`out()`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-out) method.
# This takes the HTTP headers as a hash parameter, and the body content via a
# block. The headers can be generated as a string using the
# [`http_header()`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-http_header)
# method. The output stream can be written directly to using the
# [`print()`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-print)
# method.
#
# ### Generating HTML
#
# Each HTML element has a corresponding method for generating that element as a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). The name of this
# method is the same as that of the element, all lowercase. The attributes of
# the element are passed in as a hash, and the body as a no-argument block that
# evaluates to a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
# The HTML generation module knows which elements are always empty, and silently
# drops any passed-in body. It also knows which elements require matching
# closing tags and which don't. However, it does not know what attributes are
# legal for which elements.
#
# There are also some additional HTML generation methods mixed in from the
# [`CGI::HtmlExtension`](https://docs.ruby-lang.org/en/2.7.0/CGI/HtmlExtension.html)
# module. These include individual methods for the different types of form
# inputs, and methods for elements that commonly take particular attributes
# where the attributes can be directly specified as arguments, rather than via a
# hash.
#
# ### Utility HTML escape and other methods like a function.
#
# There are some utility tool defined in cgi/util.rb . And when include, you can
# use utility methods like a function.
#
# ## Examples of use
#
# ### Get form values
#
# ```ruby
# require "cgi"
# cgi = CGI.new
# value = cgi['field_name']   # <== value string for 'field_name'
#   # if not 'field_name' included, then return "".
# fields = cgi.keys            # <== array of field names
#
# # returns true if form has 'field_name'
# cgi.has_key?('field_name')
# cgi.has_key?('field_name')
# cgi.include?('field_name')
# ```
#
# CAUTION! [cgi]('field_name') returned an
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) with the old
# cgi.rb(included in Ruby 1.6)
#
# ### Get form values as hash
#
# ```ruby
# require "cgi"
# cgi = CGI.new
# params = cgi.params
# ```
#
# cgi.params is a hash.
#
# ```ruby
# cgi.params['new_field_name'] = ["value"]  # add new param
# cgi.params['field_name'] = ["new_value"]  # change value
# cgi.params.delete('field_name')           # delete param
# cgi.params.clear                          # delete all params
# ```
#
# ### Save form values to file
#
# ```ruby
# require "pstore"
# db = PStore.new("query.db")
# db.transaction do
#   db["params"] = cgi.params
# end
# ```
#
# ### Restore form values from file
#
# ```ruby
# require "pstore"
# db = PStore.new("query.db")
# db.transaction do
#   cgi.params = db["params"]
# end
# ```
#
# ### Get multipart form values
#
# ```ruby
# require "cgi"
# cgi = CGI.new
# value = cgi['field_name']   # <== value string for 'field_name'
# value.read                  # <== body of value
# value.local_path            # <== path to local file of value
# value.original_filename     # <== original filename of value
# value.content_type          # <== content_type of value
# ```
#
# and value has [`StringIO`](https://docs.ruby-lang.org/en/2.7.0/StringIO.html)
# or [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) class
# methods.
#
# ### Get cookie values
#
# ```ruby
# require "cgi"
# cgi = CGI.new
# values = cgi.cookies['name']  # <== array of 'name'
#   # if not 'name' included, then return [].
# names = cgi.cookies.keys      # <== array of cookie names
# ```
#
# and cgi.cookies is a hash.
#
# ### Get cookie objects
#
# ```ruby
# require "cgi"
# cgi = CGI.new
# for name, cookie in cgi.cookies
#   cookie.expires = Time.now + 30
# end
# cgi.out("cookie" => cgi.cookies) {"string"}
#
# cgi.cookies # { "name1" => cookie1, "name2" => cookie2, ... }
#
# require "cgi"
# cgi = CGI.new
# cgi.cookies['name'].expires = Time.now + 30
# cgi.out("cookie" => cgi.cookies['name']) {"string"}
# ```
#
# ### Print http header and html string to $DEFAULT\_OUTPUT ($>)
#
# ```ruby
# require "cgi"
# cgi = CGI.new("html4")  # add HTML generation methods
# cgi.out do
#   cgi.html do
#     cgi.head do
#       cgi.title { "TITLE" }
#     end +
#     cgi.body do
#       cgi.form("ACTION" => "uri") do
#         cgi.p do
#           cgi.textarea("get_text") +
#           cgi.br +
#           cgi.submit
#         end
#       end +
#       cgi.pre do
#         CGI.escapeHTML(
#           "params: #{cgi.params.inspect}\n" +
#           "cookies: #{cgi.cookies.inspect}\n" +
#           ENV.collect do |key, value|
#             "#{key} --> #{value}\n"
#           end.join("")
#         )
#       end
#     end
#   end
# end
#
# # add HTML generation methods
# CGI.new("html3")    # html3.2
# CGI.new("html4")    # html4.01 (Strict)
# CGI.new("html4Tr")  # html4.01 Transitional
# CGI.new("html4Fr")  # html4.01 Frameset
# CGI.new("html5")    # html5
# ```
#
# ### Some utility methods
#
# ```ruby
# require 'cgi/util'
# CGI.escapeHTML('Usage: foo "bar" <baz>')
# ```
#
# ### Some utility methods like a function
#
# ```ruby
# require 'cgi/util'
# include CGI::Util
# escapeHTML('Usage: foo "bar" <baz>')
# h('Usage: foo "bar" <baz>') # alias
# ```
class CGI
  include ::CGI::Util
  extend ::CGI::Escape
  extend ::CGI::Util
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) for carriage
  # return
  CR = ::T.let(nil, ::T.untyped)
  # Standard internet newline sequence
  EOL = ::T.let(nil, ::T.untyped)
  # HTTP status codes.
  HTTP_STATUS = ::T.let(nil, ::T.untyped)
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) for linefeed
  LF = ::T.let(nil, ::T.untyped)
  # Maximum number of request parameters when multipart
  MAX_MULTIPART_COUNT = ::T.let(nil, ::T.untyped)
  # Whether processing will be required in binary vs text
  NEEDS_BINMODE = ::T.let(nil, ::T.untyped)
  # Path separators in different environments.
  PATH_SEPARATOR = ::T.let(nil, ::T.untyped)
  REVISION = ::T.let(nil, ::T.untyped)

  # Return the accept character set for this
  # [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) instance.
  sig {returns(::T.untyped)}
  def accept_charset(); end

  # This method is an alias for
  # [`http_header`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-http_header),
  # when HTML5 tag maker is inactive.
  #
  # NOTE: use
  # [`http_header`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-http_header)
  # to create HTTP header blocks, this alias is only provided for backwards
  # compatibility.
  #
  # Using
  # [`header`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-header)
  # with the HTML5 tag maker will create a <header> element.
  #
  # Alias for:
  # [`http_header`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-http_header)
  sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def header(options=T.unsafe(nil)); end

  # Create an HTTP header block as a string.
  #
  # Includes the empty line that ends the header block.
  #
  # `content_type_string`
  # :   If this form is used, this string is the `Content-Type`
  # `headers_hash`
  # :   A [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of header
  #     values. The following header keys are recognized:
  #
  #     type
  # :       The Content-Type header. Defaults to "text/html"
  #     charset
  # :       The charset of the body, appended to the Content-Type header.
  #     nph
  # :       A boolean value. If true, prepend protocol string and status code,
  #         and date; and sets default values for "server" and "connection" if
  #         not explicitly set.
  #     status
  # :       The HTTP status code as a
  #         [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html),
  #         returned as the Status header. The values are:
  #
  #         OK
  # :           200 OK
  #         PARTIAL\_CONTENT
  # :           206 Partial Content
  #         MULTIPLE\_CHOICES
  # :           300 Multiple Choices
  #         MOVED
  # :           301 Moved Permanently
  #         REDIRECT
  # :           302 Found
  #         NOT\_MODIFIED
  # :           304 Not Modified
  #         BAD\_REQUEST
  # :           400 Bad Request
  #         AUTH\_REQUIRED
  # :           401 Authorization Required
  #         FORBIDDEN
  # :           403 Forbidden
  #         NOT\_FOUND
  # :           404 Not Found
  #         METHOD\_NOT\_ALLOWED
  # :           405 [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html)
  #             Not Allowed
  #         NOT\_ACCEPTABLE
  # :           406 Not Acceptable
  #         LENGTH\_REQUIRED
  # :           411 Length Required
  #         PRECONDITION\_FAILED
  # :           412 Precondition Failed
  #         SERVER\_ERROR
  # :           500 Internal Server
  #             [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html)
  #         NOT\_IMPLEMENTED
  # :           501 [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html)
  #             Not Implemented
  #         BAD\_GATEWAY
  # :           502 Bad Gateway
  #         VARIANT\_ALSO\_VARIES
  # :           506 Variant Also Negotiates
  #
  #
  #     server
  # :       The server software, returned as the Server header.
  #     connection
  # :       The connection type, returned as the Connection header (for
  #         instance, "close".
  #     length
  # :       The length of the content that will be sent, returned as the
  #         Content-Length header.
  #     language
  # :       The language of the content, returned as the Content-Language
  #         header.
  #     expires
  # :       The time on which the current content expires, as a `Time` object,
  #         returned as the Expires header.
  #     cookie
  # :       A cookie or cookies, returned as one or more Set-Cookie headers. The
  #         value can be the literal string of the cookie; a
  #         [`CGI::Cookie`](https://docs.ruby-lang.org/en/2.7.0/CGI/Cookie.html)
  #         object; an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  #         of literal cookie strings or Cookie objects; or a hash all of whose
  #         values are literal cookie strings or Cookie objects.
  #
  #         These cookies are in addition to the cookies held in the
  #         @output\_cookies field.
  #
  #
  #     Other headers can also be set; they are appended as key: value.
  #
  #
  # Examples:
  #
  # ```ruby
  # http_header
  #   # Content-Type: text/html
  #
  # http_header("text/plain")
  #   # Content-Type: text/plain
  #
  # http_header("nph"        => true,
  #             "status"     => "OK",  # == "200 OK"
  #               # "status"     => "200 GOOD",
  #             "server"     => ENV['SERVER_SOFTWARE'],
  #             "connection" => "close",
  #             "type"       => "text/html",
  #             "charset"    => "iso-2022-jp",
  #               # Content-Type: text/html; charset=iso-2022-jp
  #             "length"     => 103,
  #             "language"   => "ja",
  #             "expires"    => Time.now + 30,
  #             "cookie"     => [cookie1, cookie2],
  #             "my_header1" => "my_value",
  #             "my_header2" => "my_value")
  # ```
  #
  # This method does not perform charset conversion.
  #
  # Also aliased as:
  # [`header`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-header)
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
    .void
  end
  def initialize(options=T.unsafe(nil), &block); end

  sig {returns(::T.untyped)}
  def nph?(); end

  # Print an HTTP header and body to $DEFAULT\_OUTPUT ($>)
  #
  # `content_type_string`
  # :   If a string is passed, it is assumed to be the content type.
  # `headers_hash`
  # :   This is a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of
  #     headers, similar to that used by
  #     [`http_header`](https://docs.ruby-lang.org/en/2.7.0/CGI.html#method-i-http_header).
  # `block`
  # :   A block is required and should evaluate to the body of the response.
  #
  #
  # `Content-Length` is automatically calculated from the size of the
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) returned by the
  # content block.
  #
  # If `ENV['REQUEST_METHOD'] == "HEAD"`, then only the header is output (the
  # content block is still required, but it is ignored).
  #
  # If the charset is "iso-2022-jp" or "euc-jp" or "shift\_jis" then the content
  # is converted to this charset, and the language is set to "ja".
  #
  # Example:
  #
  # ```ruby
  # cgi = CGI.new
  # cgi.out{ "string" }
  #   # Content-Type: text/html
  #   # Content-Length: 6
  #   #
  #   # string
  #
  # cgi.out("text/plain") { "string" }
  #   # Content-Type: text/plain
  #   # Content-Length: 6
  #   #
  #   # string
  #
  # cgi.out("nph"        => true,
  #         "status"     => "OK",  # == "200 OK"
  #         "server"     => ENV['SERVER_SOFTWARE'],
  #         "connection" => "close",
  #         "type"       => "text/html",
  #         "charset"    => "iso-2022-jp",
  #           # Content-Type: text/html; charset=iso-2022-jp
  #         "language"   => "ja",
  #         "expires"    => Time.now + (3600 * 24 * 30),
  #         "cookie"     => [cookie1, cookie2],
  #         "my_header1" => "my_value",
  #         "my_header2" => "my_value") { "string" }
  #    # HTTP/1.1 200 OK
  #    # Date: Sun, 15 May 2011 17:35:54 GMT
  #    # Server: Apache 2.2.0
  #    # Connection: close
  #    # Content-Type: text/html; charset=iso-2022-jp
  #    # Content-Length: 6
  #    # Content-Language: ja
  #    # Expires: Tue, 14 Jun 2011 17:35:54 GMT
  #    # Set-Cookie: foo
  #    # Set-Cookie: bar
  #    # my_header1: my_value
  #    # my_header2: my_value
  #    #
  #    # string
  # ```
  sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def out(options=T.unsafe(nil)); end

  # Print an argument or list of arguments to the default output stream
  #
  # ```ruby
  # cgi = CGI.new
  # cgi.print    # default:  cgi.print == $DEFAULT_OUTPUT.print
  # ```
  sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def print(*options); end

  # Return the accept character set for all new
  # [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) instances.
  sig {returns(::T.untyped)}
  def self.accept_charset(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the accept character
  # set for all new [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
  # instances.
  sig do
    params(
      accept_charset: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.accept_charset=(accept_charset); end

  # Parse an HTTP query string into a hash of key=>value pairs.
  #
  # ```ruby
  # params = CGI.parse("query_string")
  #   # {"name1" => ["value1", "value2", ...],
  #   #  "name2" => ["value1", "value2", ...], ... }
  # ```
  sig do
    params(
      query: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(query); end
end

# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) representing an HTTP
# cookie.
#
# In addition to its specific fields and methods, a
# [`Cookie`](https://docs.ruby-lang.org/en/2.7.0/CGI/Cookie.html) instance is a
# delegator to the array of its values.
#
# See RFC 2965.
#
# ## Examples of use
#
# ```
# cookie1 = CGI::Cookie.new("name", "value1", "value2", ...)
# cookie1 = CGI::Cookie.new("name" => "name", "value" => "value")
# cookie1 = CGI::Cookie.new('name'     => 'name',
#                           'value'    => ['value1', 'value2', ...],
#                           'path'     => 'path',   # optional
#                           'domain'   => 'domain', # optional
#                           'expires'  => Time.now, # optional
#                           'secure'   => true,     # optional
#                           'httponly' => true      # optional
#                           )
#
# cgi.out("cookie" => [cookie1, cookie2]) { "string" }
#
# name     = cookie1.name
# values   = cookie1.value
# path     = cookie1.path
# domain   = cookie1.domain
# expires  = cookie1.expires
# secure   = cookie1.secure
# httponly = cookie1.httponly
#
# cookie1.name     = 'name'
# cookie1.value    = ['value1', 'value2', ...]
# cookie1.path     = 'path'
# cookie1.domain   = 'domain'
# cookie1.expires  = Time.now + 30
# cookie1.secure   = true
# cookie1.httponly = true
# ```
class CGI::Cookie < Array
  Elem = type_member(:out) {{fixed: String}}

  # Domain for which this cookie applies, as a `String`
  sig {returns(::T.untyped)}
  def domain(); end

  # Domain for which this cookie applies, as a `String`
  sig do
    params(
      domain: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def domain=(domain); end

  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) at which this cookie
  # expires, as a `Time`
  sig {returns(::T.untyped)}
  def expires(); end

  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) at which this cookie
  # expires, as a `Time`
  sig do
    params(
      expires: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def expires=(expires); end

  # True if this cookie is httponly; false otherwise
  sig {returns(::T.untyped)}
  def httponly(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) whether the
  # [`Cookie`](https://docs.ruby-lang.org/en/2.7.0/CGI/Cookie.html) is a
  # httponly cookie or not.
  #
  # `val` must be a boolean.
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
    .void
  end
  def initialize(name=T.unsafe(nil), *value); end

  # A summary of cookie string.
  sig {returns(::T.untyped)}
  def inspect(); end

  # Name of this cookie, as a `String`
  sig {returns(::T.untyped)}
  def name(); end

  # Name of this cookie, as a `String`
  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def name=(name); end

  # Path for which this cookie applies, as a `String`
  sig {returns(::T.untyped)}
  def path(); end

  # Path for which this cookie applies, as a `String`
  sig do
    params(
      path: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def path=(path); end

  # True if this cookie is secure; false otherwise
  sig {returns(::T.untyped)}
  def secure(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) whether the
  # [`Cookie`](https://docs.ruby-lang.org/en/2.7.0/CGI/Cookie.html) is a secure
  # cookie or not.
  #
  # `val` must be a boolean.
  sig do
    params(
      val: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def secure=(val); end

  # Convert the [`Cookie`](https://docs.ruby-lang.org/en/2.7.0/CGI/Cookie.html)
  # to its string representation.
  sig {returns(::T.untyped)}
  def to_s(); end

  # Returns the value or list of values for this cookie.
  sig {returns(::T.untyped)}
  def value(); end

  # Replaces the value of this cookie with a new value or list of values.
  sig do
    params(
      val: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def value=(val); end

  # Parse a raw cookie string into a hash of cookie-name=>Cookie pairs.
  #
  # ```ruby
  # cookies = CGI::Cookie.parse("raw_cookie_string")
  #   # { "name1" => cookie1, "name2" => cookie2, ... }
  # ```
  sig do
    params(
      raw_cookie: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(raw_cookie); end
end

# Mixin module that provides the following:
#
# 1.  Access to the [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
#     environment variables as methods. See documentation to the
#     [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) class for a list of
#     these variables. The methods are exposed by removing the leading `HTTP_`
#     (if it exists) and downcasing the name. For example, `auth_type` will
#     return the environment variable `AUTH_TYPE`, and `accept` will return the
#     value for `HTTP_ACCEPT`.
#
# 2.  Access to cookies, including the cookies attribute.
#
# 3.  Access to parameters, including the params attribute, and overloading
#     [`[]`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html#method-i-5B-5D)
#     to perform parameter value lookup by key.
#
# 4.  The
#     [`initialize_query`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html#method-i-initialize_query)
#     method, for initializing the above mechanisms, handling multipart forms,
#     and allowing the class to be used in "offline" mode.
module CGI::QueryExtension
  # Get the value for the parameter with a given key.
  #
  # If the parameter has multiple values, only the first will be retrieved; use
  # [`params`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html#attribute-i-params)
  # to get the array of values.
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

  # Get the cookies as a hash of cookie-name=>Cookie pairs.
  sig {returns(::T.untyped)}
  def cookies(); end

  # Get the cookies as a hash of cookie-name=>Cookie pairs.
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

  # Get the uploaded files as a hash of name=>values pairs
  sig {returns(::T.untyped)}
  def files(); end

  sig {returns(::T.untyped)}
  def from(); end

  sig {returns(::T.untyped)}
  def gateway_interface(); end

  # Returns true if a given query string parameter exists.
  #
  # Also aliased as:
  # [`key?`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html#method-i-key-3F),
  # [`include?`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html#method-i-include-3F)
  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def has_key?(*args); end

  sig {returns(::T.untyped)}
  def host(); end

  # Alias for:
  # [`has_key?`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html#method-i-has_key-3F)
  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def include?(*args); end

  # Alias for:
  # [`has_key?`](https://docs.ruby-lang.org/en/2.7.0/CGI/QueryExtension.html#method-i-has_key-3F)
  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def key?(*args); end

  # Return all query parameter names as an array of
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def keys(*args); end

  # Returns whether the form contained multipart/form-data
  sig {returns(::T.untyped)}
  def multipart?(); end

  sig {returns(::T.untyped)}
  def negotiate(); end

  # Get the parameters as a hash of name=>values pairs, where values is an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  sig {returns(::T.untyped)}
  def params(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) all the parameters.
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

  # Get the raw cookies as a string.
  sig {returns(::T.untyped)}
  def raw_cookie(); end

  # Get the raw RFC2965 cookies as a string.
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
  # Abbreviated day-of-week names specified by RFC 822
  RFC822_DAYS = ::T.let(nil, ::T.untyped)
  # Abbreviated month names specified by RFC 822
  RFC822_MONTHS = ::T.let(nil, ::T.untyped)
  # The set of special characters and their escaped values
  TABLE_FOR_ESCAPE_HTML__ = ::T.let(nil, ::T.untyped)

  # URL-encode a string.
  #
  # ```ruby
  # url_encoded_string = CGI.escape("'Stop!' said Fred")
  #    # => "%27Stop%21%27+said+Fred"
  # ```
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape(_); end

  # Escape only the tags of certain HTML elements in `string`.
  #
  # Takes an element or elements or array of elements. Each element is specified
  # by the name of the element, without angle brackets. This matches both the
  # start and the end tag of that element. The attribute list of the open tag
  # will also be escaped (for instance, the double-quotes surrounding attribute
  # values).
  #
  # ```ruby
  # print CGI.escapeElement('<BR><A HREF="url"></A>', "A", "IMG")
  #   # "<BR>&lt;A HREF=&quot;url&quot;&gt;&lt;/A&gt"
  #
  # print CGI.escapeElement('<BR><A HREF="url"></A>', ["A", "IMG"])
  #   # "<BR>&lt;A HREF=&quot;url&quot;&gt;&lt;/A&gt"
  # ```
  #
  #
  # Also aliased as:
  # [`escape_element`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escape_element)
  sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escapeElement(string, *elements); end

  # Escape special characters in HTML, namely '&"<>
  #
  # ```ruby
  # CGI.escapeHTML('Usage: foo "bar" <baz>')
  #    # => "Usage: foo &quot;bar&quot; &lt;baz&gt;"
  # ```
  #
  #
  # Also aliased as:
  # [`escape_html`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escape_html),
  # [`h`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-h)
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escapeHTML(_); end

  # Synonym for
  # [`CGI.escapeElement(str)`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escapeElement)
  #
  # Alias for:
  # [`escapeElement`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escapeElement)
  sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape_element(string, *elements); end

  # Synonym for
  # [`CGI.escapeHTML(str)`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escapeHTML)
  #
  # Alias for:
  # [`escapeHTML`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escapeHTML)
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def escape_html(_); end

  # Alias for:
  # [`escapeHTML`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escapeHTML)
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def h(_); end

  # Prettify (indent) an HTML string.
  #
  # `string` is the HTML string to indent. `shift` is the indentation unit to
  # use; it defaults to two spaces.
  #
  # ```ruby
  # print CGI.pretty("<HTML><BODY></BODY></HTML>")
  #   # <HTML>
  #   #   <BODY>
  #   #   </BODY>
  #   # </HTML>
  #
  # print CGI.pretty("<HTML><BODY></BODY></HTML>", "\t")
  #   # <HTML>
  #   #         <BODY>
  #   #         </BODY>
  #   # </HTML>
  # ```
  sig do
    params(
      string: ::T.untyped,
      shift: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pretty(string, shift=T.unsafe(nil)); end

  # Format a `Time` object as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) using the format
  # specified by RFC 1123.
  #
  # ```ruby
  # CGI.rfc1123_date(Time.now)
  #   # Sat, 01 Jan 2000 00:00:00 GMT
  # ```
  sig do
    params(
      time: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def rfc1123_date(time); end

  # URL-decode a string with encoding(optional).
  #
  # ```ruby
  # string = CGI.unescape("%27Stop%21%27+said+Fred")
  #    # => "'Stop!' said Fred"
  # ```
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape(*_); end

  # Undo escaping such as that done by
  # [`CGI.escapeElement()`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escapeElement)
  #
  # ```ruby
  # print CGI.unescapeElement(
  #         CGI.escapeHTML('<BR><A HREF="url"></A>'), "A", "IMG")
  #   # "&lt;BR&gt;<A HREF="url"></A>"
  #
  # print CGI.unescapeElement(
  #         CGI.escapeHTML('<BR><A HREF="url"></A>'), ["A", "IMG"])
  #   # "&lt;BR&gt;<A HREF="url"></A>"
  # ```
  #
  #
  # Also aliased as:
  # [`unescape_element`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-unescape_element)
  sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescapeElement(string, *elements); end

  # Unescape a string that has been HTML-escaped
  #
  # ```ruby
  # CGI.unescapeHTML("Usage: foo &quot;bar&quot; &lt;baz&gt;")
  #    # => "Usage: foo \"bar\" <baz>"
  # ```
  #
  #
  # Also aliased as:
  # [`unescape_html`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-unescape_html)
  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescapeHTML(_); end

  # Synonym for
  # [`CGI.unescapeElement(str)`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-unescapeElement)
  #
  # Alias for:
  # [`unescapeElement`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-unescapeElement)
  sig do
    params(
      string: ::T.untyped,
      elements: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unescape_element(string, *elements); end

  # Synonym for
  # [`CGI.unescapeHTML(str)`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-unescapeHTML)
  #
  # Alias for:
  # [`unescapeHTML`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-unescapeHTML)
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

# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) raised when
# there is an invalid encoding detected
class CGI::InvalidEncoding < Exception
end
