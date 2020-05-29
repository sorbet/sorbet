# typed: __STDLIB_INTERNAL

# [`OpenURI`](https://docs.ruby-lang.org/en/2.6.0/OpenURI.html) is an
# easy-to-use wrapper for
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.6.0/Net/HTTP.html), Net::HTTPS
# and [`Net::FTP`](https://docs.ruby-lang.org/en/2.6.0/Net/FTP.html).
#
# ## Example
#
# It is possible to open an http, https or ftp URL as though it were a file:
#
# ```ruby
# open("http://www.ruby-lang.org/") {|f|
#   f.each_line {|line| p line}
# }
# ```
#
# The opened file has several getter methods for its meta-information, as
# follows, since it is extended by
# [`OpenURI::Meta`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/Meta.html).
#
# ```ruby
# open("http://www.ruby-lang.org/en") {|f|
#   f.each_line {|line| p line}
#   p f.base_uri         # <URI::HTTP:0x40e6ef2 URL:http://www.ruby-lang.org/en/>
#   p f.content_type     # "text/html"
#   p f.charset          # "iso-8859-1"
#   p f.content_encoding # []
#   p f.last_modified    # Thu Dec 05 02:45:02 UTC 2002
# }
# ```
#
# Additional header fields can be specified by an optional hash argument.
#
# ```ruby
# open("http://www.ruby-lang.org/en/",
#   "User-Agent" => "Ruby/#{RUBY_VERSION}",
#   "From" => "foo@bar.invalid",
#   "Referer" => "http://www.ruby-lang.org/") {|f|
#   # ...
# }
# ```
#
# The environment variables such as http\_proxy, https\_proxy and ftp\_proxy are
# in effect by default. Here we disable proxy:
#
# ```ruby
# open("http://www.ruby-lang.org/en/", :proxy => nil) {|f|
#   # ...
# }
# ```
#
# See
# [`OpenURI::OpenRead.open`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/OpenRead.html#method-i-open)
# and
# [`Kernel#open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open)
# for more on available options.
#
# [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) objects can be opened in
# a similar way.
#
# ```ruby
# uri = URI.parse("http://www.ruby-lang.org/en/")
# uri.open {|f|
#   # ...
# }
# ```
#
# [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) objects can be read
# directly. The returned string is also extended by
# [`OpenURI::Meta`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/Meta.html).
#
# ```ruby
# str = uri.read
# p str.base_uri
# ```
#
# Author
# :   Tanaka Akira <akr@m17n.org>
module OpenURI
  Options = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])
end

class OpenURI::HTTPError < ::StandardError
  def io; end

  def self.new(message, io); end
end

# Raised on redirection, only occurs when `redirect` option for HTTP is `false`.
class OpenURI::HTTPRedirect < ::OpenURI::HTTPError
  def uri; end

  def self.new(message, io, uri); end
end

# Mixin for holding meta-information.
module OpenURI::Meta
  # returns a [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) that is the
  # base of relative URIs in the data. It may differ from the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) supplied by a user due
  # to redirection.
  def base_uri; end

  # returns a [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) that is the
  # base of relative URIs in the data. It may differ from the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) supplied by a user due
  # to redirection.
  def base_uri=(_); end

  # returns a charset parameter in Content-Type field. It is downcased for
  # canonicalization.
  #
  # If charset parameter is not given but a block is given, the block is called
  # and its result is returned. It can be used to guess charset.
  #
  # If charset parameter and block is not given, nil is returned except text
  # type in HTTP. In that case, "iso-8859-1" is returned as defined by RFC2616
  # 3.7.1.
  def charset; end

  # Returns a list of encodings in Content-Encoding field as an array of
  # strings.
  #
  # The encodings are downcased for canonicalization.
  def content_encoding; end

  # returns "type/subtype" which is MIME Content-Type. It is downcased for
  # canonicalization. Content-Type parameters are stripped.
  def content_type; end

  # returns a [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) that
  # represents the Last-Modified field.
  def last_modified; end

  # returns a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) that
  # represents header fields. The
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) keys are downcased
  # for canonicalization. The
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) values are a field
  # body. If there are multiple field with same field name, the field values are
  # concatenated with a comma.
  def meta; end

  # returns a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) that
  # represents header fields. The
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) keys are downcased
  # for canonicalization. The
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) value are an array
  # of field values.
  def metas; end

  # returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) that
  # consists of status code and message.
  def status; end

  # returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) that
  # consists of status code and message.
  def status=(_); end
end


# Mixin for HTTP and FTP URIs.
module OpenURI::OpenRead
  # [`OpenURI::OpenRead#open`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/OpenRead.html#method-i-open)
  # provides 'open' for
  # [`URI::HTTP`](https://docs.ruby-lang.org/en/2.6.0/URI/HTTP.html) and
  # [`URI::FTP`](https://docs.ruby-lang.org/en/2.6.0/URI/FTP.html).
  #
  # [`OpenURI::OpenRead#open`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/OpenRead.html#method-i-open)
  # takes optional 3 arguments as:
  #
  # ```ruby
  # OpenURI::OpenRead#open([mode [, perm]] [, options]) [{|io| ... }]
  # ```
  #
  # [`OpenURI::OpenRead#open`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/OpenRead.html#method-i-open)
  # returns an IO-like object if block is not given. Otherwise it yields the
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object and return the
  # value of the block. The [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html)
  # object is extended with
  # [`OpenURI::Meta`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/Meta.html).
  #
  # `mode` and `perm` are the same as
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open).
  #
  # However, `mode` must be read mode because
  # [`OpenURI::OpenRead#open`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/OpenRead.html#method-i-open)
  # doesn't support write mode (yet). Also `perm` is ignored because it is
  # meaningful only for file creation.
  #
  # `options` must be a hash.
  #
  # Each option with a string key specifies an extra header field for HTTP.
  # I.e., it is ignored for FTP without HTTP proxy.
  #
  # The hash may include other options, where keys are symbols:
  #
  # :proxy
  # :   Synopsis:
  #
  # ```
  # :proxy => "http://proxy.foo.com:8000/"
  # :proxy => URI.parse("http://proxy.foo.com:8000/")
  # :proxy => true
  # :proxy => false
  # :proxy => nil
  # ```
  #
  #     If :proxy option is specified, the value should be
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html),
  #     [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html), boolean or nil.
  #
  #     When [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) or
  #     [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) is given, it is
  #     treated as proxy [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html).
  #
  #     When true is given or the option itself is not specified, environment
  #     variable 'scheme\_proxy' is examined. 'scheme' is replaced by 'http',
  #     'https' or 'ftp'.
  #
  #     When false or nil is given, the environment variables are ignored and
  #     connection will be made to a server directly.
  #
  # :proxy\_http\_basic\_authentication
  # :   Synopsis:
  #
  # ```
  # :proxy_http_basic_authentication =>
  #   ["http://proxy.foo.com:8000/", "proxy-user", "proxy-password"]
  # :proxy_http_basic_authentication =>
  #   [URI.parse("http://proxy.foo.com:8000/"),
  #    "proxy-user", "proxy-password"]
  # ```
  #
  #     If :proxy option is specified, the value should be an
  #     [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) with 3
  #     elements. It should contain a proxy
  #     [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html), a proxy user name
  #     and a proxy password. The proxy
  #     [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) should be a
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), an
  #     [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) or nil. The proxy
  #     user name and password should be a
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
  #
  #     If nil is given for the proxy
  #     [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html), this option is
  #     just ignored.
  #
  #     If :proxy and :proxy\_http\_basic\_authentication is specified,
  #     [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html)
  #     is raised.
  #
  # :http\_basic\_authentication
  # :   Synopsis:
  #
  # ```
  # :http_basic_authentication=>[user, password]
  # ```
  #
  #     If :http\_basic\_authentication is specified, the value should be an
  #     array which contains 2 strings: username and password. It is used for
  #     HTTP Basic authentication defined by RFC 2617.
  #
  # :content\_length\_proc
  # :   Synopsis:
  #
  # ```
  # :content_length_proc => lambda {|content_length| ... }
  # ```
  #
  #     If :content\_length\_proc option is specified, the option value
  #     procedure is called before actual transfer is started. It takes one
  #     argument, which is expected content length in bytes.
  #
  #     If two or more transfers are performed by HTTP redirection, the
  #     procedure is called only once for the last transfer.
  #
  #     When expected content length is unknown, the procedure is called with
  #     nil. This happens when the HTTP response has no Content-Length header.
  #
  # :progress\_proc
  # :   Synopsis:
  #
  # ```
  # :progress_proc => lambda {|size| ...}
  # ```
  #
  #     If :progress\_proc option is specified, the proc is called with one
  #     argument each time when 'open' gets content fragment from network. The
  #     argument `size` is the accumulated transferred size in bytes.
  #
  #     If two or more transfer is done by HTTP redirection, the procedure is
  #     called only one for a last transfer.
  #
  #     :progress\_proc and :content\_length\_proc are intended to be used for
  #     progress bar. For example, it can be implemented as follows using
  #     Ruby/ProgressBar.
  #
  # ```
  # pbar = nil
  # open("http://...",
  #   :content_length_proc => lambda {|t|
  #     if t && 0 < t
  #       pbar = ProgressBar.new("...", t)
  #       pbar.file_transfer_mode
  #     end
  #   },
  #   :progress_proc => lambda {|s|
  #     pbar.set s if pbar
  #   }) {|f| ... }
  # ```
  #
  # :read\_timeout
  # :   Synopsis:
  #
  # ```
  # :read_timeout=>nil     (no timeout)
  # :read_timeout=>10      (10 second)
  # ```
  #
  #     :read\_timeout option specifies a timeout of read for http connections.
  #
  # :open\_timeout
  # :   Synopsis:
  #
  # ```
  # :open_timeout=>nil     (no timeout)
  # :open_timeout=>10      (10 second)
  # ```
  #
  #     :open\_timeout option specifies a timeout of open for http connections.
  #
  # :ssl\_ca\_cert
  # :   Synopsis:
  #
  # ```
  # :ssl_ca_cert=>filename or an Array of filenames
  # ```
  #
  #     :ssl\_ca\_cert is used to specify CA certificate for SSL. If it is
  #     given, default certificates are not used.
  #
  # :ssl\_verify\_mode
  # :   Synopsis:
  #
  # ```
  # :ssl_verify_mode=>mode
  # ```
  #
  #     :ssl\_verify\_mode is used to specify openssl verify mode.
  #
  # :ftp\_active\_mode
  # :   Synopsis:
  #
  # ```
  # :ftp_active_mode=>bool
  # ```
  #
  #     `:ftp_active_mode => true` is used to make ftp active mode. Ruby 1.9
  #     uses passive mode by default. Note that the active mode is default in
  #     Ruby 1.8 or prior.
  #
  # :redirect
  # :   Synopsis:
  #
  # ```
  # :redirect=>bool
  # ```
  #
  #     `:redirect` is true by default. `:redirect => false` is used to disable
  #     all HTTP redirects.
  #
  #     [`OpenURI::HTTPRedirect`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/HTTPRedirect.html)
  #     exception raised on redirection. Using `true` also means that
  #     redirections between http and ftp are permitted.
  def open(*rest, &block); end

  # [`[OpenURI::OpenRead#read(](options)`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/OpenRead.html#method-i-read))
  # reads a content referenced by self and returns the content as string. The
  # string is extended with
  # [`OpenURI::Meta`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/Meta.html).
  # The argument `options` is same as
  # [`OpenURI::OpenRead#open`](https://docs.ruby-lang.org/en/2.6.0/OpenURI/OpenRead.html#method-i-open).
  def read(options = _); end
end
