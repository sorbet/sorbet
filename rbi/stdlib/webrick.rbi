# typed: __STDLIB_INTERNAL

# # WEB server toolkit.
#
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) is an HTTP
# server toolkit that can be configured as an HTTPS server, a proxy server, and
# a virtual-host server.
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) features
# complete logging of both server operations and HTTP access.
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) supports both
# basic and digest authentication in addition to algorithms not in RFC 2617.
#
# A [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) server can be
# composed of multiple
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) servers or
# servlets to provide differing behavior on a per-host or per-path basis.
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) includes
# servlets for handling [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
# scripts, [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) pages, Ruby
# blocks and directory listings.
#
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) also includes
# tools for daemonizing a process and starting a process at a higher privilege
# level and dropping permissions.
#
# ## Starting an HTTP server
#
# To create a new
# [`WEBrick::HTTPServer`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServer.html)
# that will listen to connections on port 8000 and serve documents from the
# current user's public\_html folder:
#
# ```ruby
# require 'webrick'
#
# root = File.expand_path '~/public_html'
# server = WEBrick::HTTPServer.new :Port => 8000, :DocumentRoot => root
# ```
#
# To run the server you will need to provide a suitable shutdown hook as
# starting the server blocks the current thread:
#
# ```ruby
# trap 'INT' do server.shutdown end
#
# server.start
# ```
#
# ## Custom Behavior
#
# The easiest way to have a server perform custom operations is through
# [`WEBrick::HTTPServer#mount_proc`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServer.html#method-i-mount_proc).
# The block given will be called with a
# [`WEBrick::HTTPRequest`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPRequest.html)
# with request info and a
# [`WEBrick::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPResponse.html)
# which must be filled in appropriately:
#
# ```ruby
# server.mount_proc '/' do |req, res|
#   res.body = 'Hello, world!'
# end
# ```
#
# Remember that `server.mount_proc` must precede `server.start`.
#
# ## Servlets
#
# Advanced custom behavior can be obtained through mounting a subclass of
# [`WEBrick::HTTPServlet::AbstractServlet`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/AbstractServlet.html).
# Servlets provide more modularity when writing an HTTP server than mount\_proc
# allows. Here is a simple servlet:
#
# ```ruby
# class Simple < WEBrick::HTTPServlet::AbstractServlet
#   def do_GET request, response
#     status, content_type, body = do_stuff_with request
#
#     response.status = 200
#     response['Content-Type'] = 'text/plain'
#     response.body = 'Hello, World!'
#   end
# end
# ```
#
# To initialize the servlet you mount it on the server:
#
# ```ruby
# server.mount '/simple', Simple
# ```
#
# See
# [`WEBrick::HTTPServlet::AbstractServlet`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/AbstractServlet.html)
# for more details.
#
# ## Virtual Hosts
#
# A server can act as a virtual host for multiple host names. After creating the
# listening host, additional hosts that do not listen can be created and
# attached as virtual hosts:
#
# ```
# server = WEBrick::HTTPServer.new # ...
#
# vhost = WEBrick::HTTPServer.new :ServerName => 'vhost.example',
#                                 :DoNotListen => true, # ...
# vhost.mount '/', ...
#
# server.virtual_host vhost
# ```
#
# If no `:DocumentRoot` is provided and no servlets or procs are mounted on the
# main server it will return 404 for all URLs.
#
# ## HTTPS
#
# To create an HTTPS server you only need to enable SSL and provide an SSL
# certificate name:
#
# ```ruby
# require 'webrick'
# require 'webrick/https'
#
# cert_name = [
#   %w[CN localhost],
# ]
#
# server = WEBrick::HTTPServer.new(:Port => 8000,
#                                  :SSLEnable => true,
#                                  :SSLCertName => cert_name)
# ```
#
# This will start the server with a self-generated self-signed certificate. The
# certificate will be changed every time the server is restarted.
#
# To create a server with a pre-determined key and certificate you can provide
# them:
#
# ```ruby
# require 'webrick'
# require 'webrick/https'
# require 'openssl'
#
# cert = OpenSSL::X509::Certificate.new File.read '/path/to/cert.pem'
# pkey = OpenSSL::PKey::RSA.new File.read '/path/to/pkey.pem'
#
# server = WEBrick::HTTPServer.new(:Port => 8000,
#                                  :SSLEnable => true,
#                                  :SSLCertificate => cert,
#                                  :SSLPrivateKey => pkey)
# ```
#
# ## Proxy Server
#
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) can act as a
# proxy server:
#
# ```ruby
# require 'webrick'
# require 'webrick/httpproxy'
#
# proxy = WEBrick::HTTPProxyServer.new :Port => 8000
#
# trap 'INT' do proxy.shutdown end
# ```
#
# See WEBrick::HTTPProxy for further details including modifying proxied
# responses.
#
# ## Basic and [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) authentication
#
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) provides both
# Basic and [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html)
# authentication for regular and proxy servers. See
# [`WEBrick::HTTPAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth.html),
# [`WEBrick::HTTPAuth::BasicAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/BasicAuth.html)
# and
# [`WEBrick::HTTPAuth::DigestAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/DigestAuth.html).
#
# ## [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) as a Production Web Server
#
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) can be run as a
# production server for small loads.
#
# ### Daemonizing
#
# To start a [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html)
# server as a daemon simple run
# [`WEBrick::Daemon.start`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/Daemon.html#method-c-start)
# before starting the server.
#
# ### Dropping Permissions
#
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) can be started
# as one user to gain permission to bind to port 80 or 443 for serving HTTP or
# HTTPS traffic then can drop these permissions for regular operation. To listen
# on all interfaces for HTTP traffic:
#
# ```ruby
# sockets = WEBrick::Utils.create_listeners nil, 80
# ```
#
# Then drop privileges:
#
# ```ruby
# WEBrick::Utils.su 'www'
# ```
#
# Then create a server that does not listen by default:
#
# ```
# server = WEBrick::HTTPServer.new :DoNotListen => true, # ...
# ```
#
# Then overwrite the listening sockets with the port 80 sockets:
#
# ```ruby
# server.listeners.replace sockets
# ```
#
# ### Logging
#
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) can separately
# log server operations and end-user access. For server operations:
#
# ```ruby
# log_file = File.open '/var/log/webrick.log', 'a+'
# log = WEBrick::Log.new log_file
# ```
#
# For user access logging:
#
# ```ruby
# access_log = [
#   [log_file, WEBrick::AccessLog::COMBINED_LOG_FORMAT],
# ]
#
# server = WEBrick::HTTPServer.new :Logger => log, :AccessLog => access_log
# ```
#
# See
# [`WEBrick::AccessLog`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/AccessLog.html)
# for further log formats.
#
# ### Log Rotation
#
# To rotate logs in
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) on a HUP signal
# (like syslogd can send), open the log file in 'a+' mode (as above) and trap
# 'HUP' to reopen the log file:
#
# ```
# trap 'HUP' do log_file.reopen '/path/to/webrick.log', 'a+'
# ```
#
# ## Copyright
#
# Author: IPR -- Internet Programming with Ruby -- writers
#
# Copyright (c) 2000 TAKAHASHI Masayoshi, GOTOU YUUZOU Copyright (c) 2002
# Internet Programming with Ruby writers. All rights reserved.
module WEBrick
  CR = T.let(nil, T.untyped)
  CRLF = T.let(nil, T.untyped)
  LF = T.let(nil, T.untyped)
  # The [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) version
  VERSION = T.let(nil, T.untyped)
end

# [`AccessLog`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/AccessLog.html)
# provides logging to various files in various formats.
#
# Multiple logs may be written to at the same time:
#
# ```ruby
# access_log = [
#   [$stderr, WEBrick::AccessLog::COMMON_LOG_FORMAT],
#   [$stderr, WEBrick::AccessLog::REFERER_LOG_FORMAT],
# ]
#
# server = WEBrick::HTTPServer.new :AccessLog => access_log
# ```
#
# Custom log formats may be defined.
# [`WEBrick::AccessLog`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/AccessLog.html)
# provides a subset of the formatting from Apache's mod\_log\_config
# http://httpd.apache.org/docs/mod/mod\_log\_config.html#formats. See
# AccessLog::setup\_params for a list of supported options
module WEBrick::AccessLog
  # User-Agent Log Format
  AGENT_LOG_FORMAT = T.let(nil, T.untyped)
  # Short alias for Common Log Format
  CLF = T.let(nil, T.untyped)
  # The Common Log Format's time format
  CLF_TIME_FORMAT = T.let(nil, T.untyped)
  # Combined Log Format
  COMBINED_LOG_FORMAT = T.let(nil, T.untyped)
  # Common Log Format
  COMMON_LOG_FORMAT = T.let(nil, T.untyped)
  # Referer Log Format
  REFERER_LOG_FORMAT = T.let(nil, T.untyped)

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

# A generic logging class
class WEBrick::BasicLog
  # Debugging error level for messages used in server development or debugging
  DEBUG = T.let(nil, T.untyped)
  # [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) log level which
  # indicates a recoverable error
  ERROR = T.let(nil, T.untyped)
  # Fatal log level which indicates a server crash
  FATAL = T.let(nil, T.untyped)
  # Information log level which indicates possibly useful information
  INFO = T.let(nil, T.untyped)
  # [`Warning`](https://docs.ruby-lang.org/en/2.7.0/Warning.html) log level
  # which indicates a possible problem
  WARN = T.let(nil, T.untyped)

  # Synonym for
  # log([`INFO`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#INFO),
  # obj.to\_s)
  sig do
    params(
      obj: T.untyped,
    )
    .returns(T.untyped)
  end
  def <<(obj); end

  # Closes the logger (also closes the log device associated to the logger)
  sig {returns(T.untyped)}
  def close(); end

  # Shortcut for logging a
  # [`DEBUG`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#DEBUG)
  # message
  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def debug(msg); end

  # Will the logger output
  # [`DEBUG`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#DEBUG)
  # messages?
  sig {returns(T.untyped)}
  def debug?(); end

  # Shortcut for logging an
  # [`ERROR`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#ERROR)
  # message
  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def error(msg); end

  # Will the logger output
  # [`ERROR`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#ERROR)
  # messages?
  sig {returns(T.untyped)}
  def error?(); end

  # Shortcut for logging a
  # [`FATAL`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#FATAL)
  # message
  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def fatal(msg); end

  # Will the logger output
  # [`FATAL`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#FATAL)
  # messages?
  sig {returns(T.untyped)}
  def fatal?(); end

  # Shortcut for logging an
  # [`INFO`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#INFO)
  # message
  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def info(msg); end

  # Will the logger output
  # [`INFO`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#INFO)
  # messages?
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

  # log-level, messages above this level will be logged
  sig {returns(T.untyped)}
  def level(); end

  # log-level, messages above this level will be logged
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def level=(_); end

  # Logs `data` at `level` if the given level is above the current log level.
  sig do
    params(
      level: T.untyped,
      data: T.untyped,
    )
    .returns(T.untyped)
  end
  def log(level, data); end

  # Shortcut for logging a
  # [`WARN`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#WARN)
  # message
  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def warn(msg); end

  # Will the logger output
  # [`WARN`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/BasicLog.html#WARN)
  # messages?
  sig {returns(T.untyped)}
  def warn?(); end
end

# Raised if a parameter such as %e, %i, %o or %n is used without fetching a
# specific field.
class WEBrick::AccessLog::AccessLogError < ::StandardError
end

module WEBrick::Config
  # Default configuration for
  # [`WEBrick::HTTPAuth::BasicAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/BasicAuth.html)
  #
  # :AutoReloadUserDB
  # :   Reload the user database provided by :UserDB automatically?
  BasicAuth = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])

  # Default configuration for
  # [`WEBrick::HTTPAuth::DigestAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/DigestAuth.html).
  #
  # :Algorithm
  # :   MD5, MD5-sess (default), SHA1, SHA1-sess
  # :Domain
  # :   An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of URIs
  #     that define the protected space
  # :Qop
  # :   'auth' for authentication, 'auth-int' for integrity protection or both
  # :UseOpaque
  # :   Should the server send opaque values to the client?  This helps prevent
  #     replay attacks.
  # :CheckNc
  # :   Should the server check the nonce count?  This helps the server detect
  #     replay attacks.
  # :UseAuthenticationInfoHeader
  # :   Should the server send an AuthenticationInfo header?
  # :AutoReloadUserDB
  # :   Reload the user database provided by :UserDB automatically?
  # :NonceExpirePeriod
  # :   How long should we store used nonces?  Default is 30 minutes.
  # :NonceExpireDelta
  # :   How long is a nonce valid?  Default is 1 minute
  # :InternetExplorerHack
  # :   Hack which allows Internet Explorer to work.
  # :OperaHack
  # :   Hack which allows Opera to work.
  DigestAuth = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])

  # Default configuration for
  # [`WEBrick::HTTPServlet::FileHandler`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/FileHandler.html)
  #
  # :AcceptableLanguages
  # :   [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of languages
  #     allowed for accept-language. There is no default
  # :DirectoryCallback
  # :   Allows preprocessing of directory requests. There is no default
  #     callback.
  # :FancyIndexing
  # :   If true, show an index for directories. The default is true.
  # :FileCallback
  # :   Allows preprocessing of file requests. There is no default callback.
  # :HandlerCallback
  # :   Allows preprocessing of requests. There is no default callback.
  # :HandlerTable
  # :   Maps file suffixes to file handlers. DefaultFileHandler is used by
  #     default but any servlet can be used.
  # :NondisclosureName
  # :   Do not show files matching this array of globs. .ht\* and \*~ are
  #     excluded by default.
  # :UserDir
  # :   Directory inside ~user to serve content from for /~user requests. Only
  #     works if mounted on /. Disabled by default.
  FileHandler = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])

  # for GenericServer
  General = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])

  # for HTTPServer, HTTPRequest, HTTPResponse ...
  HTTP = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])

  LIBDIR = T.let(T.unsafe(nil), String)
end

# Processes HTTP cookies
class WEBrick::Cookie
  # The cookie comment
  sig {returns(T.untyped)}
  def comment(); end

  # The cookie comment
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def comment=(_); end

  # The cookie domain
  sig {returns(T.untyped)}
  def domain(); end

  # The cookie domain
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def domain=(_); end

  # Retrieves the expiration time as a
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html)
  sig {returns(T.untyped)}
  def expires(); end

  # Sets the cookie expiration to the time `t`. The expiration time may be a
  # false value to disable expiration or a
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) or HTTP format time
  # string to set the expiration date.
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
    .void
  end
  def initialize(name, value); end

  # The maximum age of the cookie
  sig {returns(T.untyped)}
  def max_age(); end

  # The maximum age of the cookie
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def max_age=(_); end

  # The cookie name
  sig {returns(T.untyped)}
  def name(); end

  # The cookie path
  sig {returns(T.untyped)}
  def path(); end

  # The cookie path
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def path=(_); end

  # Is this a secure cookie?
  sig {returns(T.untyped)}
  def secure(); end

  # Is this a secure cookie?
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def secure=(_); end

  # The cookie string suitable for use in an HTTP header
  sig {returns(T.untyped)}
  def to_s(); end

  # The cookie value
  sig {returns(T.untyped)}
  def value(); end

  # The cookie value
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def value=(_); end

  # The cookie version
  sig {returns(T.untyped)}
  def version(); end

  # The cookie version
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def version=(_); end

  # Parses a [`Cookie`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/Cookie.html)
  # field sent from the user-agent. Returns an array of cookies.
  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse(str); end

  # Parses the cookie in `str`
  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_set_cookie(str); end

  # Parses the cookies in `str`
  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_set_cookies(str); end
end

# A generic module for daemonizing a process
class WEBrick::Daemon
  # Performs the standard operations for daemonizing a process. Runs a block, if
  # given.
  sig {returns(T.untyped)}
  def self.start(); end
end

# Base TCP server class. You must subclass
# [`GenericServer`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/GenericServer.html)
# and provide a
# [`run`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/GenericServer.html#method-i-run)
# method.
class WEBrick::GenericServer
  # Retrieves `key` from the configuration
  sig do
    params(
      key: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](key); end

  # The server configuration
  sig {returns(T.untyped)}
  def config(); end

  sig do
    params(
      config: T.untyped,
      default: T.untyped,
    )
    .void
  end
  def initialize(config=T.unsafe(nil), default=T.unsafe(nil)); end

  # Adds listeners from `address` and `port` to the server. See
  # [`WEBrick::Utils::create_listeners`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/Utils.html#method-c-create_listeners)
  # for details.
  sig do
    params(
      address: T.untyped,
      port: T.untyped,
    )
    .returns(T.untyped)
  end
  def listen(address, port); end

  # Sockets listening for connections.
  sig {returns(T.untyped)}
  def listeners(); end

  # The server logger. This is independent from the HTTP access log.
  sig {returns(T.untyped)}
  def logger(); end

  # You must subclass
  # [`GenericServer`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/GenericServer.html)
  # and implement
  # [`run`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/GenericServer.html#method-i-run)
  # which accepts a TCP client socket
  sig do
    params(
      sock: T.untyped,
    )
    .returns(T.untyped)
  end
  def run(sock); end

  # Shuts down the server and all listening sockets. New listeners must be
  # provided to restart the server.
  sig {returns(T.untyped)}
  def shutdown(); end

  # Starts the server and runs the `block` for each connection. This method does
  # not return until the server is stopped from a signal handler or another
  # thread using
  # [`stop`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/GenericServer.html#method-i-stop)
  # or
  # [`shutdown`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/GenericServer.html#method-i-shutdown).
  #
  # If the block raises a subclass of
  # [`StandardError`](https://docs.ruby-lang.org/en/2.7.0/StandardError.html)
  # the exception is logged and ignored. If an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) or
  # Errno::EBADF exception is raised the exception is ignored. If an
  # [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) subclass
  # is raised the exception is logged and re-raised which stops the server.
  #
  # To completely shut down a server call
  # [`shutdown`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/GenericServer.html#method-i-shutdown)
  # from ensure:
  #
  # ```ruby
  # server = WEBrick::GenericServer.new
  # # or WEBrick::HTTPServer.new
  #
  # begin
  #   server.start
  # ensure
  #   server.shutdown
  # end
  # ```
  sig do
    returns(T.untyped)
  end
  def start(); end

  # The server status. One of :Stop, :Running or :Shutdown
  sig {returns(T.untyped)}
  def status(); end

  # Stops the server from accepting new connections.
  sig {returns(T.untyped)}
  def stop(); end

  # Tokens control the number of outstanding clients. The `:MaxClients`
  # configuration sets this.
  sig {returns(T.untyped)}
  def tokens(); end
end

module WEBrick::HTMLUtils
  # Escapes &, ", > and < in `string`
  sig do
    params(
      string: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.escape(string); end
end

# [`HTTPAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth.html)
# provides both basic and digest authentication.
#
# To enable authentication for requests in
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) you will need a
# user database and an authenticator. To start, here's an Htpasswd database for
# use with a DigestAuth authenticator:
#
# ```ruby
# config = { :Realm => 'DigestAuth example realm' }
#
# htpasswd = WEBrick::HTTPAuth::Htpasswd.new 'my_password_file'
# htpasswd.auth_type = WEBrick::HTTPAuth::DigestAuth
# htpasswd.set_passwd config[:Realm], 'username', 'password'
# htpasswd.flush
# ```
#
# The `:Realm` is used to provide different access to different groups across
# several resources on a server. Typically you'll need only one realm for a
# server.
#
# This database can be used to create an authenticator:
#
# ```ruby
# config[:UserDB] = htpasswd
#
# digest_auth = WEBrick::HTTPAuth::DigestAuth.new config
# ```
#
# To authenticate a request call authenticate with a request and response object
# in a servlet:
#
# ```ruby
# def do_GET req, res
#   @authenticator.authenticate req, res
# end
# ```
#
# For digest authentication the authenticator must not be created every request,
# it must be passed in as an option via
# [`WEBrick::HTTPServer#mount`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServer.html#method-i-mount).
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

# [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) providing generic
# support for both [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html)
# and Basic authentication schemes.
module WEBrick::HTTPAuth::Authenticator
  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) of
  # authentication, must be overridden by the including class
  AuthScheme = T.let(nil, T.untyped)
  RequestField = T.let(nil, T.untyped)
  ResponseField = T.let(nil, T.untyped)
  ResponseInfoField = T.let(nil, T.untyped)

  # The logger for this authenticator
  sig {returns(T.untyped)}
  def logger(); end

  # The realm this authenticator covers
  sig {returns(T.untyped)}
  def realm(); end

  # The user database for this authenticator
  sig {returns(T.untyped)}
  def userdb(); end
end

# Basic Authentication for
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html)
#
# Use this class to add basic authentication to a
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) servlet.
#
# Here is an example of how to set up a BasicAuth:
#
# ```ruby
# config = { :Realm => 'BasicAuth example realm' }
#
# htpasswd = WEBrick::HTTPAuth::Htpasswd.new 'my_password_file', password_hash: :bcrypt
# htpasswd.set_passwd config[:Realm], 'username', 'password'
# htpasswd.flush
#
# config[:UserDB] = htpasswd
#
# basic_auth = WEBrick::HTTPAuth::BasicAuth.new config
# ```
class WEBrick::HTTPAuth::BasicAuth
  include WEBrick::HTTPAuth::Authenticator
  AuthScheme = T.let(nil, T.untyped)

  # Authenticates a `req` and returns a 401 Unauthorized using `res` if the
  # authentication was not correct.
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def authenticate(req, res); end

  # Returns a challenge response which asks for authentication information
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
    .void
  end
  def initialize(config, default=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def logger(); end

  sig {returns(T.untyped)}
  def realm(); end

  sig {returns(T.untyped)}
  def userdb(); end

  # Used by UserDB to create a basic password entry
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

# RFC 2617 [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) Access
# Authentication for
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html)
#
# Use this class to add digest authentication to a
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) servlet.
#
# Here is an example of how to set up DigestAuth:
#
# ```ruby
# config = { :Realm => 'DigestAuth example realm' }
#
# htdigest = WEBrick::HTTPAuth::Htdigest.new 'my_password_file'
# htdigest.set_passwd config[:Realm], 'username', 'password'
# htdigest.flush
#
# config[:UserDB] = htdigest
#
# digest_auth = WEBrick::HTTPAuth::DigestAuth.new config
# ```
#
# When using this as with a servlet be sure not to create a new
# [`DigestAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/DigestAuth.html)
# object in the servlet's initialize. By default
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) creates a new
# servlet instance for every request and the
# [`DigestAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/DigestAuth.html)
# object must be used across requests.
class WEBrick::HTTPAuth::DigestAuth
  include WEBrick::HTTPAuth::Authenticator
  AuthScheme = T.let(nil, T.untyped)
  MustParams = T.let(nil, T.untyped)
  MustParamsAuth = T.let(nil, T.untyped)

  # [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) authentication
  # algorithm
  sig {returns(T.untyped)}
  def algorithm(); end

  # Authenticates a `req` and returns a 401 Unauthorized using `res` if the
  # authentication was not correct.
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def authenticate(req, res); end

  # Returns a challenge response which asks for authentication information
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
    .void
  end
  def initialize(config, default=T.unsafe(nil)); end

  # Quality of protection. RFC 2617 defines "auth" and "auth-int"
  sig {returns(T.untyped)}
  def qop(); end

  # Used by UserDB to create a digest password entry
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
  Elem = type_member(:out) {{fixed: T.untyped}}

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

# [`Htdigest`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/Htdigest.html)
# accesses apache-compatible digest password files. Passwords are matched to a
# realm where they are valid. For security, the path for a digest password
# database should be stored outside of the paths available to the HTTP server.
#
# [`Htdigest`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/Htdigest.html)
# is intended for use with
# [`WEBrick::HTTPAuth::DigestAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/DigestAuth.html)
# and stores passwords using cryptographic hashes.
#
# ```ruby
# htpasswd = WEBrick::HTTPAuth::Htdigest.new 'my_password_file'
# htpasswd.set_passwd 'my realm', 'username', 'password'
# htpasswd.flush
# ```
class WEBrick::HTTPAuth::Htdigest
  include WEBrick::HTTPAuth::UserDB
  # Removes a password from the database for `user` in `realm`.
  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
    )
    .returns(T.untyped)
  end
  def delete_passwd(realm, user); end

  # Iterate passwords in the database.
  sig {returns(T.untyped)}
  def each(); end

  # Flush the password database. If `output` is given the database will be
  # written there instead of to the original path.
  sig do
    params(
      output: T.untyped,
    )
    .returns(T.untyped)
  end
  def flush(output=T.unsafe(nil)); end

  # Retrieves a password from the database for `user` in `realm`. If `reload_db`
  # is true the database will be reloaded first.
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
    .void
  end
  def initialize(path); end

  # Reloads passwords from the database
  sig {returns(T.untyped)}
  def reload(); end

  # Sets a password in the database for `user` in `realm` to `pass`.
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

# [`Htgroup`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/Htgroup.html)
# accesses apache-compatible group files.
# [`Htgroup`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/Htgroup.html)
# can be used to provide group-based authentication for users. Currently
# [`Htgroup`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/Htgroup.html)
# is not directly integrated with any authenticators in
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html). For security,
# the path for a digest password database should be stored outside of the paths
# available to the HTTP server.
#
# Example:
#
# ```ruby
# htgroup = WEBrick::HTTPAuth::Htgroup.new 'my_group_file'
# htgroup.add 'superheroes', %w[spiderman batman]
#
# htgroup.members('superheroes').include? 'magneto' # => false
# ```
class WEBrick::HTTPAuth::Htgroup
  # Add an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # `members` to `group`
  sig do
    params(
      group: T.untyped,
      members: T.untyped,
    )
    .returns(T.untyped)
  end
  def add(group, members); end

  # Flush the group database. If `output` is given the database will be written
  # there instead of to the original path.
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
    .void
  end
  def initialize(path); end

  # Retrieve the list of members from `group`
  sig do
    params(
      group: T.untyped,
    )
    .returns(T.untyped)
  end
  def members(group); end

  # Reload groups from the database
  sig {returns(T.untyped)}
  def reload(); end
end

# [`Htpasswd`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/Htpasswd.html)
# accesses apache-compatible password files. Passwords are matched to a realm
# where they are valid. For security, the path for a password database should be
# stored outside of the paths available to the HTTP server.
#
# [`Htpasswd`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/Htpasswd.html)
# is intended for use with
# [`WEBrick::HTTPAuth::BasicAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/BasicAuth.html).
#
# To create an
# [`Htpasswd`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/Htpasswd.html)
# database with a single user:
#
# ```ruby
# htpasswd = WEBrick::HTTPAuth::Htpasswd.new 'my_password_file'
# htpasswd.set_passwd 'my realm', 'username', 'password'
# htpasswd.flush
# ```
class WEBrick::HTTPAuth::Htpasswd
  include WEBrick::HTTPAuth::UserDB
  # Removes a password from the database for `user` in `realm`.
  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
    )
    .returns(T.untyped)
  end
  def delete_passwd(realm, user); end

  # Iterate passwords in the database.
  sig {returns(T.untyped)}
  def each(); end

  # Flush the password database. If `output` is given the database will be
  # written there instead of to the original path.
  sig do
    params(
      output: T.untyped,
    )
    .returns(T.untyped)
  end
  def flush(output=T.unsafe(nil)); end

  # Retrieves a password from the database for `user` in `realm`. If `reload_db`
  # is true the database will be reloaded first.
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
      password_hash: T.untyped,
    )
    .void
  end
  def initialize(path, password_hash: T.unsafe(nil)); end

  # Reload passwords from the database
  sig {returns(T.untyped)}
  def reload(); end

  # Sets a password in the database for `user` in `realm` to `pass`.
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

# [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) providing generic
# support for both [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html)
# and Basic authentication schemes for proxies.
module WEBrick::HTTPAuth::ProxyAuthenticator
  InfoField = T.let(nil, T.untyped)
  RequestField = T.let(nil, T.untyped)
  ResponseField = T.let(nil, T.untyped)

end

# Basic authentication for proxy servers. See BasicAuth for details.
class WEBrick::HTTPAuth::ProxyBasicAuth < WEBrick::HTTPAuth::BasicAuth
  include WEBrick::HTTPAuth::ProxyAuthenticator
end

# [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) authentication for
# proxy servers. See DigestAuth for details.
class WEBrick::HTTPAuth::ProxyDigestAuth < WEBrick::HTTPAuth::DigestAuth
  include WEBrick::HTTPAuth::ProxyAuthenticator
end

# User database mixin for
# [`HTTPAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth.html). This
# mixin dispatches user record access to the underlying
# [`auth_type`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/UserDB.html#attribute-i-auth_type)
# for this database.
module WEBrick::HTTPAuth::UserDB
  # The authentication type.
  #
  # [`WEBrick::HTTPAuth::BasicAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/BasicAuth.html)
  # or
  # [`WEBrick::HTTPAuth::DigestAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/DigestAuth.html)
  # are built-in.
  sig {returns(T.untyped)}
  def auth_type(); end

  # The authentication type.
  #
  # [`WEBrick::HTTPAuth::BasicAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/BasicAuth.html)
  # or
  # [`WEBrick::HTTPAuth::DigestAuth`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/DigestAuth.html)
  # are built-in.
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def auth_type=(_); end

  # Retrieves a password in `realm` for `user` for the
  # [`auth_type`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/UserDB.html#attribute-i-auth_type)
  # of this database. `reload_db` is a dummy value.
  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      reload_db: T.untyped,
    )
    .returns(T.untyped)
  end
  def get_passwd(realm, user, reload_db=T.unsafe(nil)); end

  # Creates an obscured password in `realm` with `user` and `password` using the
  # [`auth_type`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/UserDB.html#attribute-i-auth_type)
  # of this database.
  sig do
    params(
      realm: T.untyped,
      user: T.untyped,
      pass: T.untyped,
    )
    .returns(T.untyped)
  end
  def make_passwd(realm, user, pass); end

  # Sets a password in `realm` with `user` and `password` for the
  # [`auth_type`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPAuth/UserDB.html#attribute-i-auth_type)
  # of this database.
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

# An HTTP request. This is consumed by service and do\_\* methods in
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) servlets
class WEBrick::HTTPRequest
  BODY_CONTAINABLE_METHODS = T.let(nil, T.untyped)
  MAX_URI_LENGTH = T.let(nil, T.untyped)
  PrivateNetworkRegexp = T.let(nil, T.untyped)

  # Retrieves `header_name`
  sig do
    params(
      header_name: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](header_name); end

  # The Accept header value
  sig {returns(T.untyped)}
  def accept(); end

  # The Accept-Charset header value
  sig {returns(T.untyped)}
  def accept_charset(); end

  # The Accept-Encoding header value
  sig {returns(T.untyped)}
  def accept_encoding(); end

  # The Accept-Language header value
  sig {returns(T.untyped)}
  def accept_language(); end

  # The socket address of the server
  sig {returns(T.untyped)}
  def addr(); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of request
  # attributes
  sig {returns(T.untyped)}
  def attributes(); end

  # Returns the request body.
  sig do
    returns(T.untyped)
  end
  def body(); end

  # The content-length header
  sig {returns(T.untyped)}
  def content_length(); end

  # The content-type header
  sig {returns(T.untyped)}
  def content_type(); end

  sig {returns(T.untyped)}
  def continue(); end

  # The parsed request cookies
  sig {returns(T.untyped)}
  def cookies(); end

  # Iterates over the request headers
  sig {returns(T.untyped)}
  def each(); end

  sig {returns(T.untyped)}
  def fixup(); end

  # The parsed header of the request
  sig {returns(T.untyped)}
  def header(); end

  # The host this request is for
  sig {returns(T.untyped)}
  def host(); end

  # The HTTP version of the request
  sig {returns(T.untyped)}
  def http_version(); end

  sig do
    params(
      config: T.untyped,
    )
    .void
  end
  def initialize(config); end

  # Is this a keep-alive connection?
  sig {returns(T.untyped)}
  def keep_alive(); end

  # Should the connection this request was made on be kept alive?
  sig {returns(T.untyped)}
  def keep_alive?(); end

  # This method provides the metavariables defined by the revision 3 of "The WWW
  # Common Gateway Interface Version 1.1" To browse the current document of
  # [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html) Version 1.1, see
  # below: http://tools.ietf.org/html/rfc3875
  sig {returns(T.untyped)}
  def meta_vars(); end

  # Parses a request from `socket`. This is called internally by
  # [`WEBrick::HTTPServer`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServer.html).
  sig do
    params(
      socket: T.untyped,
    )
    .returns(T.untyped)
  end
  def parse(socket=T.unsafe(nil)); end

  # The request path
  sig {returns(T.untyped)}
  def path(); end

  # The path info ([`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
  # variable)
  sig {returns(T.untyped)}
  def path_info(); end

  # The path info ([`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
  # variable)
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def path_info=(_); end

  # The socket address of the client
  sig {returns(T.untyped)}
  def peeraddr(); end

  # The port this request is for
  sig {returns(T.untyped)}
  def port(); end

  # Request query as a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  sig {returns(T.untyped)}
  def query(); end

  # The query from the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) of
  # the request
  sig {returns(T.untyped)}
  def query_string(); end

  # The query from the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) of
  # the request
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def query_string=(_); end

  # The raw header of the request
  sig {returns(T.untyped)}
  def raw_header(); end

  # The client's IP address
  sig {returns(T.untyped)}
  def remote_ip(); end

  # The complete request line such as:
  #
  # ```ruby
  # GET / HTTP/1.1
  # ```
  sig {returns(T.untyped)}
  def request_line(); end

  # The request method, GET, POST, PUT, etc.
  sig {returns(T.untyped)}
  def request_method(); end

  # The local time this request was received
  sig {returns(T.untyped)}
  def request_time(); end

  # The parsed [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) of the
  # request
  sig {returns(T.untyped)}
  def request_uri(); end

  # The script name ([`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
  # variable)
  sig {returns(T.untyped)}
  def script_name(); end

  # The script name ([`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
  # variable)
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def script_name=(_); end

  # The server name this request is for
  sig {returns(T.untyped)}
  def server_name(); end

  # Is this an SSL request?
  sig {returns(T.untyped)}
  def ssl?(); end

  sig {returns(T.untyped)}
  def to_s(); end

  # The unparsed [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) of the
  # request
  sig {returns(T.untyped)}
  def unparsed_uri(); end

  # The remote user ([`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
  # variable)
  sig {returns(T.untyped)}
  def user(); end

  # The remote user ([`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
  # variable)
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def user=(_); end
end

# An HTTP response. This is filled in by the service or do\_\* methods of a
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) HTTP Servlet.
class WEBrick::HTTPResponse
  # Retrieves the response header `field`
  sig do
    params(
      field: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](field); end

  # Sets the response header `field` to `value`
  sig do
    params(
      field: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def []=(field, value); end

  # Body may be:
  # *   a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html);
  # *   an IO-like object that responds to `#read` and `#readpartial`;
  # *   a Proc-like object that responds to `#call`.
  #
  #
  # In the latter case, either
  # [`chunked=`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPResponse.html#method-i-chunked-3D)
  # should be set to `true`, or `header['content-length']` explicitly provided.
  # Example:
  #
  # ```ruby
  # server.mount_proc '/' do |req, res|
  #   res.chunked = true
  #   # or
  #   # res.header['content-length'] = 10
  #   res.body = proc { |out| out.write(Time.now.to_s) }
  # end
  # ```
  sig {returns(T.untyped)}
  def body(); end

  # Body may be:
  # *   a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html);
  # *   an IO-like object that responds to `#read` and `#readpartial`;
  # *   a Proc-like object that responds to `#call`.
  #
  #
  # In the latter case, either
  # [`chunked=`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPResponse.html#method-i-chunked-3D)
  # should be set to `true`, or `header['content-length']` explicitly provided.
  # Example:
  #
  # ```ruby
  # server.mount_proc '/' do |req, res|
  #   res.chunked = true
  #   # or
  #   # res.header['content-length'] = 10
  #   res.body = proc { |out| out.write(Time.now.to_s) }
  # end
  # ```
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def body=(_); end

  # Enables chunked transfer encoding.
  sig do
    params(
      val: T.untyped,
    )
    .returns(T.untyped)
  end
  def chunked=(val); end

  # Will this response body be returned using chunked transfer-encoding?
  sig {returns(T.untyped)}
  def chunked?(); end

  # Configuration for this response
  sig {returns(T.untyped)}
  def config(); end

  # The content-length header
  sig {returns(T.untyped)}
  def content_length(); end

  # Sets the content-length header to `len`
  sig do
    params(
      len: T.untyped,
    )
    .returns(T.untyped)
  end
  def content_length=(len); end

  # The content-type header
  sig {returns(T.untyped)}
  def content_type(); end

  # Sets the content-type header to `type`
  sig do
    params(
      type: T.untyped,
    )
    .returns(T.untyped)
  end
  def content_type=(type); end

  # Response cookies
  sig {returns(T.untyped)}
  def cookies(); end

  # Iterates over each header in the response
  sig {returns(T.untyped)}
  def each(); end

  # Filename of the static file in this response. Only used by the FileHandler
  # servlet.
  sig {returns(T.untyped)}
  def filename(); end

  # Filename of the static file in this response. Only used by the FileHandler
  # servlet.
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def filename=(_); end

  # Response header
  sig {returns(T.untyped)}
  def header(); end

  # HTTP Response version
  sig {returns(T.untyped)}
  def http_version(); end

  sig do
    params(
      config: T.untyped,
    )
    .void
  end
  def initialize(config); end

  # Is this a keep-alive response?
  sig {returns(T.untyped)}
  def keep_alive(); end

  # Is this a keep-alive response?
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def keep_alive=(_); end

  # Will this response's connection be kept alive?
  sig {returns(T.untyped)}
  def keep_alive?(); end

  # Response reason phrase ("OK")
  sig {returns(T.untyped)}
  def reason_phrase(); end

  # Response reason phrase ("OK")
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def reason_phrase=(_); end

  # Request HTTP version for this response
  sig {returns(T.untyped)}
  def request_http_version(); end

  # Request HTTP version for this response
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def request_http_version=(_); end

  # Request method for this response
  sig {returns(T.untyped)}
  def request_method(); end

  # Request method for this response
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def request_method=(_); end

  # Request [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) for this
  # response
  sig {returns(T.untyped)}
  def request_uri(); end

  # Request [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) for this
  # response
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

  # Bytes sent in this response
  sig {returns(T.untyped)}
  def sent_size(); end

  # Creates an error page for exception `ex` with an optional `backtrace`
  sig do
    params(
      ex: T.untyped,
      backtrace: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_error(ex, backtrace=T.unsafe(nil)); end

  # Redirects to `url` with a
  # [`WEBrick::HTTPStatus::Redirect`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPStatus/Redirect.html)
  # `status`.
  #
  # Example:
  #
  # ```ruby
  # res.set_redirect WEBrick::HTTPStatus::TemporaryRedirect
  # ```
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

  # Response status code (200)
  sig {returns(T.untyped)}
  def status(); end

  # Sets the response's status to the `status` code
  sig do
    params(
      status: T.untyped,
    )
    .returns(T.untyped)
  end
  def status=(status); end

  # The response's HTTP status line
  sig {returns(T.untyped)}
  def status_line(); end

  sig {returns(T.untyped)}
  def to_s(); end
end

class WEBrick::HTTPResponse::InvalidHeader < ::StandardError; end

# An HTTP Server
class WEBrick::HTTPServer < WEBrick::GenericServer
  # Logs `req` and `res` in the access logs. `config` is used for the server
  # name.
  sig do
    params(
      config: T.untyped,
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def access_log(config, req, res); end

  # The default OPTIONS request handler says GET, HEAD, POST and OPTIONS
  # requests are allowed.
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
    .void
  end
  def initialize(config=T.unsafe(nil), default=T.unsafe(nil)); end

  # Finds the appropriate virtual host to handle `req`
  sig do
    params(
      req: T.untyped,
    )
    .returns(T.untyped)
  end
  def lookup_server(req); end

  # Mounts `servlet` on `dir` passing `options` to the servlet at creation time
  sig do
    params(
      dir: T.untyped,
      servlet: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def mount(dir, servlet, *options); end

  # Mounts `proc` or `block` on `dir` and calls it with a
  # [`WEBrick::HTTPRequest`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPRequest.html)
  # and
  # [`WEBrick::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPResponse.html)
  sig do
    params(
      dir: String,
      proc: T.nilable(T.proc.params(request: WEBrick::HTTPRequest, response: WEBrick::HTTPResponse).void),
      block: T.nilable(T.proc.params(request: WEBrick::HTTPRequest, response: WEBrick::HTTPResponse).void),
    )
    .void
  end
  def mount_proc(dir, proc = T.unsafe(nil), &block); end

  # Processes requests on `sock`
  sig do
    params(
      sock: T.untyped,
    )
    .returns(T.untyped)
  end
  def run(sock); end

  # Finds a servlet for `path`
  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def search_servlet(path); end

  # Services `req` and fills in `res`
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def service(req, res); end

  # Alias for:
  # [`unmount`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServer.html#method-i-unmount)
  sig do
    params(
      dir: T.untyped,
    )
    .returns(T.untyped)
  end
  def umount(dir); end

  # Unmounts `dir`
  #
  # Also aliased as:
  # [`umount`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServer.html#method-i-umount)
  sig do
    params(
      dir: T.untyped,
    )
    .returns(T.untyped)
  end
  def unmount(dir); end

  # Adds `server` as a virtual host.
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

  sig {void}
  def initialize(); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def scan(path); end
end

class WEBrick::HTTPServerError < ::WEBrick::ServerError; end

module WEBrick::HTTPServlet; end

# [`AbstractServlet`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/AbstractServlet.html)
# allows HTTP server modules to be reused across multiple servers and allows
# encapsulation of functionality.
#
# By default a servlet will respond to GET, HEAD (through an alias to GET) and
# OPTIONS requests.
#
# By default a new servlet is initialized for every request. A servlet instance
# can be reused by overriding
# [`::get_instance`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/AbstractServlet.html#method-c-get_instance)
# in the
# [`AbstractServlet`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/AbstractServlet.html)
# subclass.
#
# ## A Simple Servlet
#
# ```ruby
# class Simple < WEBrick::HTTPServlet::AbstractServlet
#   def do_GET request, response
#     status, content_type, body = do_stuff_with request
#
#     response.status = status
#     response['Content-Type'] = content_type
#     response.body = body
#   end
#
#   def do_stuff_with request
#     return 200, 'text/plain', 'you got a page'
#   end
# end
# ```
#
# This servlet can be mounted on a server at a given path:
#
# ```ruby
# server.mount '/simple', Simple
# ```
#
# ## Servlet Configuration
#
# Servlets can be configured via initialize. The first argument is the HTTP
# server the servlet is being initialized for.
#
# ```
# class Configurable < Simple
#   def initialize server, color, size
#     super server
#     @color = color
#     @size = size
#   end
#
#   def do_stuff_with request
#     content = "<p " \
#               %q{style="color: #{@color}; font-size: #{@size}"} \
#               ">Hello, World!"
#
#     return 200, "text/html", content
#   end
# end
# ```
#
# This servlet must be provided two arguments at mount time:
#
# ```ruby
# server.mount '/configurable', Configurable, 'red', '2em'
# ```
class WEBrick::HTTPServlet::AbstractServlet
  # Raises a NotFound exception
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_GET(req, res); end

  # Dispatches to
  # [`do_GET`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/AbstractServlet.html#method-i-do_GET)
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_HEAD(req, res); end

  # Returns the allowed HTTP request methods
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
    .void
  end
  def initialize(server, *options); end

  # Dispatches to a `do_` method based on `req` if such a method is available.
  # (`do_GET` for a GET request). Raises a MethodNotAllowed exception if the
  # method is not implemented.
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def service(req, res); end

  # Factory for servlet instances that will handle a request from `server` using
  # `options` from the mount point. By default a new servlet instance is created
  # for every call.
  sig do
    params(
      server: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.get_instance(server, *options); end
end

# Servlet for handling [`CGI`](https://docs.ruby-lang.org/en/2.7.0/CGI.html)
# scripts
#
# Example:
#
# ```ruby
# server.mount('/cgi/my_script', WEBrick::HTTPServlet::CGIHandler,
#              '/path/to/my_script')
# ```
class WEBrick::HTTPServlet::CGIHandler < WEBrick::HTTPServlet::AbstractServlet
  CGIRunner = T.let(nil, T.untyped)
  Ruby = T.let(nil, T.untyped)

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
    .void
  end
  def initialize(server, name); end
end

# Servlet for serving a single file. You probably want to use the FileHandler
# servlet instead as it handles directories and fancy indexes.
#
# Example:
#
# ```ruby
# server.mount('/my_page.txt', WEBrick::HTTPServlet::DefaultFileHandler,
#              '/path/to/my_page.txt')
# ```
#
# This servlet handles If-Modified-Since and
# [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) requests.
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
    .void
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

# [`ERBHandler`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/ERBHandler.html)
# evaluates an [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) file and
# returns the result. This handler is automatically used if there are .rhtml
# files in a directory served by the FileHandler.
#
# [`ERBHandler`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/ERBHandler.html)
# supports GET and POST methods.
#
# The [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) file is evaluated
# with the local variables `servlet_request` and `servlet_response` which are a
# [`WEBrick::HTTPRequest`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPRequest.html)
# and
# [`WEBrick::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPResponse.html)
# respectively.
#
# Example .rhtml file:
#
# ```
# Request to <%= servlet_request.request_uri %>
#
# Query params <%= servlet_request.query.inspect %>
# ```
class WEBrick::HTTPServlet::ERBHandler < WEBrick::HTTPServlet::AbstractServlet
  # Handles GET requests
  #
  # Also aliased as:
  # [`do_POST`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/ERBHandler.html#method-i-do_POST)
  sig do
    params(
      req: T.untyped,
      res: T.untyped,
    )
    .returns(T.untyped)
  end
  def do_GET(req, res); end

  # Handles POST requests
  #
  # Alias for:
  # [`do_GET`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServlet/ERBHandler.html#method-i-do_GET)
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
    .void
  end
  def initialize(server, name); end
end

# Serves a directory including fancy indexing and a variety of other options.
#
# Example:
#
# ```ruby
# server.mount('/assets', WEBrick::HTTPServlet::FileHandler,
#              '/path/to/assets')
# ```
class WEBrick::HTTPServlet::FileHandler < WEBrick::HTTPServlet::AbstractServlet
  HandlerTable = T.let(nil, T.untyped)

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
    .void
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

  # Allow custom handling of requests for files with `suffix` by class `handler`
  sig do
    params(
      suffix: T.untyped,
      handler: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.add_handler(suffix, handler); end

  # Remove custom handling of requests for files with `suffix`
  sig do
    params(
      suffix: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_handler(suffix); end
end

# Mounts a proc at a path that accepts a request and response.
#
# Instead of mounting this servlet with
# [`WEBrick::HTTPServer#mount`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServer.html#method-i-mount)
# use
# [`WEBrick::HTTPServer#mount_proc`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPServer.html#method-i-mount_proc):
#
# ```ruby
# server.mount_proc '/' do |req, res|
#   res.body = 'it worked!'
#   res.status = 200
# end
# ```
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
    .void
  end
  def initialize(proc); end
end

class WEBrick::HTTPServlet::HTTPServletError < ::StandardError; end

# This module is used to manager HTTP status codes.
#
# See http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html for more
# information.
module WEBrick::HTTPStatus
  CodeToError = T.let(nil, T.untyped)
  RC_ACCEPTED = T.let(nil, T.untyped)
  RC_BAD_GATEWAY = T.let(nil, T.untyped)
  RC_BAD_REQUEST = T.let(nil, T.untyped)
  RC_CONFLICT = T.let(nil, T.untyped)
  RC_CONTINUE = T.let(nil, T.untyped)
  RC_CREATED = T.let(nil, T.untyped)
  RC_EXPECTATION_FAILED = T.let(nil, T.untyped)
  RC_FAILED_DEPENDENCY = T.let(nil, T.untyped)
  RC_FORBIDDEN = T.let(nil, T.untyped)
  RC_FOUND = T.let(nil, T.untyped)
  RC_GATEWAY_TIMEOUT = T.let(nil, T.untyped)
  RC_GONE = T.let(nil, T.untyped)
  RC_HTTP_VERSION_NOT_SUPPORTED = T.let(nil, T.untyped)
  RC_INSUFFICIENT_STORAGE = T.let(nil, T.untyped)
  RC_INTERNAL_SERVER_ERROR = T.let(nil, T.untyped)
  RC_LENGTH_REQUIRED = T.let(nil, T.untyped)
  RC_LOCKED = T.let(nil, T.untyped)
  RC_METHOD_NOT_ALLOWED = T.let(nil, T.untyped)
  RC_MOVED_PERMANENTLY = T.let(nil, T.untyped)
  RC_MULTIPLE_CHOICES = T.let(nil, T.untyped)
  RC_MULTI_STATUS = T.let(nil, T.untyped)
  RC_NETWORK_AUTHENTICATION_REQUIRED = T.let(nil, T.untyped)
  RC_NON_AUTHORITATIVE_INFORMATION = T.let(nil, T.untyped)
  RC_NOT_ACCEPTABLE = T.let(nil, T.untyped)
  RC_NOT_FOUND = T.let(nil, T.untyped)
  RC_NOT_IMPLEMENTED = T.let(nil, T.untyped)
  RC_NOT_MODIFIED = T.let(nil, T.untyped)
  RC_NO_CONTENT = T.let(nil, T.untyped)
  RC_OK = T.let(nil, T.untyped)
  RC_PARTIAL_CONTENT = T.let(nil, T.untyped)
  RC_PAYMENT_REQUIRED = T.let(nil, T.untyped)
  RC_PRECONDITION_FAILED = T.let(nil, T.untyped)
  RC_PRECONDITION_REQUIRED = T.let(nil, T.untyped)
  RC_PROXY_AUTHENTICATION_REQUIRED = T.let(nil, T.untyped)
  RC_REQUEST_ENTITY_TOO_LARGE = T.let(nil, T.untyped)
  RC_REQUEST_HEADER_FIELDS_TOO_LARGE = T.let(nil, T.untyped)
  RC_REQUEST_RANGE_NOT_SATISFIABLE = T.let(nil, T.untyped)
  RC_REQUEST_TIMEOUT = T.let(nil, T.untyped)
  RC_REQUEST_URI_TOO_LARGE = T.let(nil, T.untyped)
  RC_RESET_CONTENT = T.let(nil, T.untyped)
  RC_SEE_OTHER = T.let(nil, T.untyped)
  RC_SERVICE_UNAVAILABLE = T.let(nil, T.untyped)
  RC_SWITCHING_PROTOCOLS = T.let(nil, T.untyped)
  RC_TEMPORARY_REDIRECT = T.let(nil, T.untyped)
  RC_TOO_MANY_REQUESTS = T.let(nil, T.untyped)
  RC_UNAUTHORIZED = T.let(nil, T.untyped)
  RC_UNAVAILABLE_FOR_LEGAL_REASONS = T.let(nil, T.untyped)
  RC_UNPROCESSABLE_ENTITY = T.let(nil, T.untyped)
  RC_UNSUPPORTED_MEDIA_TYPE = T.let(nil, T.untyped)
  RC_UPGRADE_REQUIRED = T.let(nil, T.untyped)
  RC_USE_PROXY = T.let(nil, T.untyped)
  StatusMessage = T.let(nil, T.untyped)

  # Returns the status class corresponding to `code`
  #
  # ```
  # WEBrick::HTTPStatus[302]
  # => WEBrick::HTTPStatus::NotFound
  # ```
  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](code); end

  # Is `code` a client error status?
  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.client_error?(code); end

  # Is `code` an error status?
  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.error?(code); end

  # Is `code` an informational status?
  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.info?(code); end

  # Returns the description corresponding to the HTTP status `code`
  #
  # ```
  # WEBrick::HTTPStatus.reason_phrase 404
  # => "Not Found"
  # ```
  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.reason_phrase(code); end

  # Is `code` a redirection status?
  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.redirect?(code); end

  # Is `code` a server error status?
  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.server_error?(code); end

  # Is `code` a successful status?
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

# Root of the HTTP client error statuses
class WEBrick::HTTPStatus::ClientError < WEBrick::HTTPStatus::Error
end

class WEBrick::HTTPStatus::Conflict < WEBrick::HTTPStatus::ClientError
end

class WEBrick::HTTPStatus::Continue < WEBrick::HTTPStatus::Info
end

class WEBrick::HTTPStatus::Created < WEBrick::HTTPStatus::Success
end

class WEBrick::HTTPStatus::EOFError < ::StandardError
end

# Root of the HTTP error statuses
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

# Root of the HTTP info statuses
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

# Root of the HTTP redirect statuses
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

# Root of the HTTP server error statuses
class WEBrick::HTTPStatus::ServerError < WEBrick::HTTPStatus::Error
end

class WEBrick::HTTPStatus::ServiceUnavailable < WEBrick::HTTPStatus::ServerError
end

# Root of the HTTP status class hierarchy
class WEBrick::HTTPStatus::Status < StandardError
  # Returns the HTTP status code
  #
  # Also aliased as:
  # [`to_i`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPStatus/Status.html#method-i-to_i)
  sig {returns(T.untyped)}
  def code(); end

  # Returns the HTTP status description
  sig {returns(T.untyped)}
  def reason_phrase(); end

  # Alias for:
  # [`code`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPStatus/Status.html#method-i-code)
  sig {returns(T.untyped)}
  def to_i(); end

  sig {returns(T.untyped)}
  def self.code(); end

  sig {returns(T.untyped)}
  def self.reason_phrase(); end
end

# Root of the HTTP success statuses
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

# [`HTTPUtils`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils.html)
# provides utility methods for working with the HTTP protocol.
#
# This module is generally used internally by
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html)
module WEBrick::HTTPUtils
  # Default mime types
  DefaultMimeTypes = T.let(nil, T.untyped)
  ESCAPED = T.let(nil, T.untyped)
  NONASCII = T.let(nil, T.untyped)
  UNESCAPED = T.let(nil, T.untyped)
  UNESCAPED_FORM = T.let(nil, T.untyped)
  UNESCAPED_PCHAR = T.let(nil, T.untyped)

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

  # Removes quotes and escapes from `str`
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

  # Loads Apache-compatible mime.types in `file`.
  sig do
    params(
      file: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.load_mime_types(file); end

  # Returns the mime type of `filename` from the list in `mime_tab`. If no mime
  # type was found application/octet-stream is returned.
  sig do
    params(
      filename: T.untyped,
      mime_tab: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mime_type(filename, mime_tab); end

  # Normalizes a request path. Raises an exception if the path cannot be
  # normalized.
  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.normalize_path(path); end

  # Parses form data in `io` with the given `boundary`
  sig do
    params(
      io: T.untyped,
      boundary: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_form_data(io, boundary); end

  # Parses an HTTP header `raw` into a hash of header fields with an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of values.
  sig do
    params(
      raw: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_header(raw); end

  # Parses the query component of a
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) in `str`
  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_query(str); end

  # Parses q values in `value` as used in Accept headers.
  sig do
    params(
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_qvalues(value); end

  # Parses a [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) header
  # value `ranges_specifier`
  sig do
    params(
      ranges_specifier: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_range_header(ranges_specifier); end

  # Quotes and escapes quotes in `str`
  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.quote(str); end

  # Splits a header value `str` according to HTTP specification.
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

# Stores multipart form data.
# [`FormData`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils/FormData.html)
# objects are created when
# [`WEBrick::HTTPUtils.parse_form_data`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils.html#method-c-parse_form_data)
# is called.
class WEBrick::HTTPUtils::FormData < String
  EmptyHeader = T.let(nil, T.untyped)
  EmptyRawHeader = T.let(nil, T.untyped)

  # Adds `str` to this
  # [`FormData`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils/FormData.html)
  # which may be the body, a header or a header entry.
  #
  # This is called by
  # [`WEBrick::HTTPUtils.parse_form_data`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils.html#method-c-parse_form_data)
  # for you
  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def <<(str); end

  # Retrieves the header at the first entry in `key`
  sig do
    params(
      key: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](*key); end

  # Adds `data` at the end of the chain of entries
  #
  # This is called by
  # [`WEBrick::HTTPUtils.parse_form_data`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils.html#method-c-parse_form_data)
  # for you.
  sig do
    params(
      data: T.untyped,
    )
    .returns(T.untyped)
  end
  def append_data(data); end

  # Yields each entry in this
  # [`FormData`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils/FormData.html)
  sig {returns(T.untyped)}
  def each_data(); end

  # The filename of the form data part
  sig {returns(T.untyped)}
  def filename(); end

  # The filename of the form data part
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
    .void
  end
  def initialize(*args); end

  # Returns all the
  # [`FormData`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils/FormData.html)
  # as an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  #
  # Also aliased as:
  # [`to_ary`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils/FormData.html#method-i-to_ary)
  sig {returns(T.untyped)}
  def list(); end

  # The name of the form data part
  sig {returns(T.untyped)}
  def name(); end

  # The name of the form data part
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

  # A
  # [`FormData`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils/FormData.html)
  # will behave like an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  #
  # Alias for:
  # [`list`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPUtils/FormData.html#method-i-list)
  sig {returns(T.untyped)}
  def to_ary(); end

  # This FormData's body
  sig {returns(T.untyped)}
  def to_s(); end
end

# Represents an HTTP protocol version
class WEBrick::HTTPVersion
  include Comparable
  # Compares this version with `other` according to the HTTP specification
  # rules.
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
    .void
  end
  def initialize(version); end

  # The major protocol version number
  sig {returns(T.untyped)}
  def major(); end

  # The major protocol version number
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def major=(_); end

  # The minor protocol version number
  sig {returns(T.untyped)}
  def minor(); end

  # The minor protocol version number
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def minor=(_); end

  # The HTTP version as show in the HTTP request and response. For example,
  # "1.1"
  sig {returns(T.untyped)}
  def to_s(); end

  # Converts `version` into an
  # [`HTTPVersion`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/HTTPVersion.html)
  sig do
    params(
      version: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.convert(version); end
end

# A logging class that prepends a timestamp to each message.
class WEBrick::Log < WEBrick::BasicLog
  sig do
    params(
      log_file: T.untyped,
      level: T.untyped,
    )
    .void
  end
  def initialize(log_file=T.unsafe(nil), level=T.unsafe(nil)); end

  # Same as BasicLog#log
  sig do
    params(
      level: T.untyped,
      data: T.untyped,
    )
    .returns(T.untyped)
  end
  def log(level, data); end

  # Format of the timestamp which is applied to each logged line. The default is
  # `"[%Y-%m-%d %H:%M:%S]"`
  sig {returns(T.untyped)}
  def time_format(); end

  # Format of the timestamp which is applied to each logged line. The default is
  # `"[%Y-%m-%d %H:%M:%S]"`
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def time_format=(_); end
end

# Server error exception
class WEBrick::ServerError < ::StandardError; end

# Base server class
class WEBrick::SimpleServer
  # A
  # [`SimpleServer`](https://docs.ruby-lang.org/en/2.7.0/WEBrick/SimpleServer.html)
  # only yields when you start it
  sig {returns(T.untyped)}
  def self.start(); end
end

module WEBrick::Utils
  # Characters used to generate random strings
  RAND_CHARS = T.let(nil, T.untyped)

  # Creates TCP server sockets bound to `address`:`port` and returns them.
  #
  # It will create IPV4 and IPV6 sockets on all interfaces.
  sig do
    params(
      address: T.untyped,
      port: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.create_listeners(address, port); end

  # The server hostname
  sig {returns(T.untyped)}
  def self.getservername(); end

  # Generates a random string of length `len`
  sig do
    params(
      len: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.random_string(len); end

  # Sets the close on exec flag for `io`
  sig do
    params(
      io: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.set_close_on_exec(io); end

  # Sets [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) operations on `io`
  # to be non-blocking
  sig do
    params(
      io: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.set_non_blocking(io); end

  # Changes the process's uid and gid to the ones of `user`
  sig do
    params(
      user: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.su(user); end

  # Executes the passed block and raises `exception` if execution takes more
  # than `seconds`.
  #
  # If `seconds` is zero or nil, simply executes the block
  sig do
    params(
      seconds: T.untyped,
      exception: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.timeout(seconds, exception=T.unsafe(nil)); end
end

# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) used to manage
# timeout handlers across multiple threads.
#
# [`Timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html) handlers should
# be managed by using the class methods which are synchronized.
#
# ```ruby
# id = TimeoutHandler.register(10, Timeout::Error)
# begin
#   sleep 20
#   puts 'foo'
# ensure
#   TimeoutHandler.cancel(id)
# end
# ```
#
# will raise
# [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html)
#
# ```ruby
# id = TimeoutHandler.register(10, Timeout::Error)
# begin
#   sleep 5
#   puts 'foo'
# ensure
#   TimeoutHandler.cancel(id)
# end
# ```
#
# will print 'foo'
class WEBrick::Utils::TimeoutHandler
  include Singleton
  extend Singleton::SingletonClassMethods
  TimeoutMutex = T.let(nil, T.untyped)

  sig do
    params(
      thread: T.untyped,
      id: T.untyped,
    )
    .returns(T.untyped)
  end
  def cancel(thread, id); end

  sig {void}
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

  # Cancels the timeout handler `id`
  sig do
    params(
      id: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cancel(id); end

  sig {returns(T.untyped)}
  def self.instance(); end

  # Registers a new timeout handler
  #
  # `time`
  # :   [`Timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html) in seconds
  # `exception`
  # :   [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) to
  #     raise when timeout elapsed
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
