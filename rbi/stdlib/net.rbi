# typed: __STDLIB_INTERNAL

module Net
end

class Net::BufferedIO
  BUFSIZE = ::T.unsafe(nil)

  def <<(str); end

  def close(); end

  def closed?(); end

  def continue_timeout(); end

  def continue_timeout=(continue_timeout); end

  def debug_output(); end

  def debug_output=(debug_output); end

  def eof?(); end

  def initialize(io, read_timeout: T.unsafe(nil), continue_timeout: T.unsafe(nil), debug_output: T.unsafe(nil)); end

  def inspect(); end

  def io(); end

  def read(len, dest=T.unsafe(nil), ignore_eof=T.unsafe(nil)); end

  def read_all(dest=T.unsafe(nil)); end

  def read_timeout(); end

  def read_timeout=(read_timeout); end

  def readline(); end

  def readuntil(terminator, ignore_eof=T.unsafe(nil)); end

  def write(str); end

  def writeline(str); end
end

module Net::DNS
  include ::Net::DNS::QueryClasses
  include ::Net::DNS::QueryTypes
  include ::Logger::Severity
  HFIXEDSZ = ::T.unsafe(nil)
  INT16SZ = ::T.unsafe(nil)
  INT32SZ = ::T.unsafe(nil)
  PACKETSZ = ::T.unsafe(nil)
  QFIXEDSZ = ::T.unsafe(nil)
  RRFIXEDSZ = ::T.unsafe(nil)
  VERSION = ::T.unsafe(nil)

end

class Net::DNS::Header
  IQUERY = ::T.unsafe(nil)
  OPARR = ::T.unsafe(nil)
  QUERY = ::T.unsafe(nil)
  STATUS = ::T.unsafe(nil)

  def aa=(val); end

  def ad=(val); end

  def anCount(); end

  def anCount=(val); end

  def arCount(); end

  def arCount=(val); end

  def auth?(); end

  def cd=(val); end

  def checking?(); end

  def data(); end

  def error?(); end

  def format(); end

  def id(); end

  def id=(val); end

  def initialize(arg=T.unsafe(nil)); end

  def inspect(); end

  def nsCount(); end

  def nsCount=(val); end

  def opCode(); end

  def opCode=(val); end

  def opCode_str(); end

  def qdCount(); end

  def qdCount=(val); end

  def qr=(val); end

  def query?(); end

  def rCode(); end

  def rCode=(val); end

  def rCode_str(); end

  def r_available?(); end

  def ra=(val); end

  def rd=(val); end

  def recursive=(val); end

  def recursive?(); end

  def response?(); end

  def tc=(val); end

  def truncated?(); end

  def verified?(); end

  def self.parse(arg); end
end

class Net::DNS::Header::Error < StandardError
end

class Net::DNS::Header::RCode
  FORMAT = ::T.unsafe(nil)
  NAME = ::T.unsafe(nil)
  NOERROR = ::T.unsafe(nil)
  NOTIMPLEMENTED = ::T.unsafe(nil)
  RCodeErrorString = ::T.unsafe(nil)
  RCodeType = ::T.unsafe(nil)
  REFUSED = ::T.unsafe(nil)
  SERVER = ::T.unsafe(nil)

  def code(); end

  def explanation(); end

  def initialize(code); end

  def to_s(); end

  def type(); end
end

class Net::DNS::Header::WrongCountError < ArgumentError
end

class Net::DNS::Header::WrongOpcodeError < ArgumentError
end

class Net::DNS::Header::WrongRecursiveError < ArgumentError
end

module Net::DNS::Names
  INT16SZ = ::T.unsafe(nil)

  def dn_comp(name, offset, compnames); end

  def dn_expand(packet, offset); end

  def names_array(name); end

  def pack_name(name); end

  def valid?(name); end
end

class Net::DNS::Names::Error < StandardError
end

class Net::DNS::Names::ExpandError < Net::DNS::Names::Error
end

class Net::DNS::Packet
  include ::Net::DNS::Names
  def additional(); end

  def additional=(object); end

  def answer(); end

  def answer=(object); end

  def answerfrom(); end

  def answersize(); end

  def authority(); end

  def authority=(object); end

  def data(); end

  def data_comp(); end

  def each_address(&block); end

  def each_cname(&block); end

  def each_mx(&block); end

  def each_nameserver(&block); end

  def each_ptr(&block); end

  def header(); end

  def header=(object); end

  def initialize(name, type=T.unsafe(nil), cls=T.unsafe(nil)); end

  def inspect(); end

  def nxdomain?(); end

  def query?(); end

  def question(); end

  def question=(object); end

  def size(); end

  def to_s(); end

  def truncated?(); end

  def self.parse(*args); end
end

class Net::DNS::Packet::Error < StandardError
end

class Net::DNS::Packet::PacketError < Net::DNS::Packet::Error
end

module Net::DNS::QueryClasses
  ANY = ::T.unsafe(nil)
  CH = ::T.unsafe(nil)
  HS = ::T.unsafe(nil)
  IN = ::T.unsafe(nil)
  NONE = ::T.unsafe(nil)

end

module Net::DNS::QueryTypes
  A = ::T.unsafe(nil)
  AAAA = ::T.unsafe(nil)
  AFSDB = ::T.unsafe(nil)
  ANY = ::T.unsafe(nil)
  ATMA = ::T.unsafe(nil)
  AXFR = ::T.unsafe(nil)
  CERT = ::T.unsafe(nil)
  CNAME = ::T.unsafe(nil)
  DNAME = ::T.unsafe(nil)
  DNSKEY = ::T.unsafe(nil)
  DS = ::T.unsafe(nil)
  EID = ::T.unsafe(nil)
  GID = ::T.unsafe(nil)
  GPOS = ::T.unsafe(nil)
  HINFO = ::T.unsafe(nil)
  ISDN = ::T.unsafe(nil)
  IXFR = ::T.unsafe(nil)
  KEY = ::T.unsafe(nil)
  KX = ::T.unsafe(nil)
  LOC = ::T.unsafe(nil)
  MAILA = ::T.unsafe(nil)
  MAILB = ::T.unsafe(nil)
  MB = ::T.unsafe(nil)
  MD = ::T.unsafe(nil)
  MF = ::T.unsafe(nil)
  MG = ::T.unsafe(nil)
  MINFO = ::T.unsafe(nil)
  MR = ::T.unsafe(nil)
  MX = ::T.unsafe(nil)
  NAPTR = ::T.unsafe(nil)
  NIMLOC = ::T.unsafe(nil)
  NS = ::T.unsafe(nil)
  NSAP = ::T.unsafe(nil)
  NSAPPTR = ::T.unsafe(nil)
  NSEC = ::T.unsafe(nil)
  NULL = ::T.unsafe(nil)
  NXT = ::T.unsafe(nil)
  OPT = ::T.unsafe(nil)
  PTR = ::T.unsafe(nil)
  PX = ::T.unsafe(nil)
  RP = ::T.unsafe(nil)
  RRSIG = ::T.unsafe(nil)
  RT = ::T.unsafe(nil)
  SIG = ::T.unsafe(nil)
  SIGZERO = ::T.unsafe(nil)
  SOA = ::T.unsafe(nil)
  SRV = ::T.unsafe(nil)
  SSHFP = ::T.unsafe(nil)
  TKEY = ::T.unsafe(nil)
  TSIG = ::T.unsafe(nil)
  TXT = ::T.unsafe(nil)
  UID = ::T.unsafe(nil)
  UINFO = ::T.unsafe(nil)
  UNSPEC = ::T.unsafe(nil)
  WKS = ::T.unsafe(nil)
  X25 = ::T.unsafe(nil)

end

class Net::DNS::Question
  include ::Net::DNS::Names
  def comp_data(); end

  def data(); end

  def initialize(name, type=T.unsafe(nil), cls=T.unsafe(nil)); end

  def inspect(); end

  def qClass(); end

  def qName(); end

  def qType(); end

  def to_s(); end

  def self.parse(arg); end
end

class Net::DNS::Question::Error < StandardError
end

class Net::DNS::Question::NameInvalid < Net::DNS::Question::Error
end

class Net::DNS::RR
  include ::Net::DNS::Names
  RRFIXEDSZ = ::T.unsafe(nil)
  RR_REGEXP = ::T.unsafe(nil)

  def cls(); end

  def comp_data(offset, compnames); end

  def data(); end

  def initialize(arg); end

  def inspect(); end

  def name(); end

  def rdata(); end

  def to_a(); end

  def to_s(); end

  def ttl(); end

  def type(); end

  def value(); end

  def self.new(*args); end

  def self.parse(data); end

  def self.parse_packet(data, offset); end
end

class Net::DNS::RR::A < Net::DNS::RR
  def address(); end

  def address=(string_or_ipaddr); end

  def value(); end
end

class Net::DNS::RR::AAAA < Net::DNS::RR
  def address(); end

  def address=(string_or_ipaddr); end

  def value(); end
end

class Net::DNS::RR::CNAME < Net::DNS::RR
  def cname(); end

  def value(); end
end

class Net::DNS::RR::Classes
  CLASSES = ::T.unsafe(nil)

  def initialize(cls); end

  def inspect(); end

  def to_i(); end

  def to_s(); end

  def self.default(); end

  def self.default=(str); end

  def self.regexp(); end

  def self.valid?(cls); end
end

class Net::DNS::RR::DataError < Net::DNS::RR::Error
end

class Net::DNS::RR::Error < StandardError
end

class Net::DNS::RR::HINFO < Net::DNS::RR
  def cpu(); end

  def os(); end

  def to_a(); end

  def value(); end
end

class Net::DNS::RR::MR < Net::DNS::RR
  def newname(); end

  def value(); end
end

class Net::DNS::RR::MX < Net::DNS::RR
  def exchange(); end

  def preference(); end

  def value(); end
end

class Net::DNS::RR::NS < Net::DNS::RR
  def nsdname(); end

  def value(); end
end

class Net::DNS::RR::PTR < Net::DNS::RR
  def ptr(); end

  def ptrdname(); end

  def value(); end
end

class Net::DNS::RR::SOA < Net::DNS::RR
  def expire(); end

  def minimum(); end

  def mname(); end

  def refresh(); end

  def retry(); end

  def rname(); end

  def serial(); end
end

class Net::DNS::RR::SRV < Net::DNS::RR
  def host(); end

  def port(); end

  def priority(); end

  def weight(); end
end

class Net::DNS::RR::TXT < Net::DNS::RR
  def txt(); end
end

class Net::DNS::RR::Types
  TYPES = ::T.unsafe(nil)

  def initialize(type); end

  def inspect(); end

  def to_i(); end

  def to_s(); end

  def to_str(); end

  def self.default(); end

  def self.default=(str); end

  def self.regexp(); end

  def self.to_str(type); end

  def self.valid?(type); end
end

class Net::DNS::Resolver
  Defaults = ::T.unsafe(nil)

  def axfr(name, cls=T.unsafe(nil)); end

  def defname(); end

  def defname=(bool); end

  def defname?(); end

  def dns_search(); end

  def dns_search=(bool); end

  def dnsrch(); end

  def dnsrch=(bool); end

  def domain(); end

  def domain=(name); end

  def ignore_truncated(); end

  def ignore_truncated=(bool); end

  def ignore_truncated?(); end

  def initialize(config=T.unsafe(nil)); end

  def inspect(); end

  def log_file=(log); end

  def log_level=(level); end

  def logger=(logger); end

  def mx(name, cls=T.unsafe(nil)); end

  def nameserver(); end

  def nameserver=(arg); end

  def nameservers(); end

  def nameservers=(arg); end

  def packet_size(); end

  def port(); end

  def port=(num); end

  def print(); end

  def query(argument, type=T.unsafe(nil), cls=T.unsafe(nil)); end

  def recurse(); end

  def recurse=(bool); end

  def recursive(); end

  def recursive=(bool); end

  def recursive?(); end

  def retrans(); end

  def retrans=(num); end

  def retry=(num); end

  def retry_interval(); end

  def retry_interval=(num); end

  def retry_number(); end

  def retry_number=(num); end

  def search(name, type=T.unsafe(nil), cls=T.unsafe(nil)); end

  def searchlist(); end

  def searchlist=(arg); end

  def source_address(); end

  def source_address=(addr); end

  def source_address_inet6(); end

  def source_port(); end

  def source_port=(num); end

  def srcaddr(); end

  def srcaddr=(addr); end

  def srcport(); end

  def srcport=(num); end

  def state(); end

  def tcp_timeout(); end

  def tcp_timeout=(secs); end

  def udp_timeout(); end

  def udp_timeout=(secs); end

  def use_tcp(); end

  def use_tcp=(bool); end

  def use_tcp?(); end

  def usevc(); end

  def usevc=(bool); end

  def self.platform_windows?(); end

  def self.start(*params); end
end

class Net::DNS::Resolver::DnsTimeout
  def initialize(seconds); end

  def pretty_to_s(); end

  def seconds(); end

  def timeout(&block); end

  def to_s(); end
end

class Net::DNS::Resolver::Error < StandardError
end

class Net::DNS::Resolver::NoResponseError < Net::DNS::Resolver::Error
end

class Net::DNS::Resolver::TcpTimeout < Net::DNS::Resolver::DnsTimeout
  def initialize(seconds); end
end

class Net::DNS::Resolver::UdpTimeout < Net::DNS::Resolver::DnsTimeout
  def initialize(seconds); end
end

# This class implements the
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) Transfer Protocol. If
# you have used a command-line
# [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) program, and are
# familiar with the commands, you will be able to use this class easily. Some
# extra features are included to take advantage of Ruby's style and strengths.
#
# ## Example
#
# ```ruby
# require 'net/ftp'
# ```
#
# ### Example 1
#
# ```ruby
# ftp = Net::FTP.new('example.com')
# ftp.login
# files = ftp.chdir('pub/lang/ruby/contrib')
# files = ftp.list('n*')
# ftp.getbinaryfile('nif.rb-0.91.gz', 'nif.gz', 1024)
# ftp.close
# ```
#
# ### Example 2
#
# ```ruby
# Net::FTP.open('example.com') do |ftp|
#   ftp.login
#   files = ftp.chdir('pub/lang/ruby/contrib')
#   files = ftp.list('n*')
#   ftp.getbinaryfile('nif.rb-0.91.gz', 'nif.gz', 1024)
# end
# ```
#
# ## Major Methods
#
# The following are the methods most likely to be useful to users:
# *   [`FTP.open`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-c-open)
# *   [`getbinaryfile`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-getbinaryfile)
# *   [`gettextfile`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-gettextfile)
# *   [`putbinaryfile`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-putbinaryfile)
# *   [`puttextfile`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-puttextfile)
# *   [`chdir`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-chdir)
# *   [`nlst`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-nlst)
# *   [`size`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-size)
# *   [`rename`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-rename)
# *   [`delete`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-delete)
class Net::FTP < Net::Protocol
  include ::OpenSSL::SSL
  include ::OpenSSL
  include ::MonitorMixin
  CASE_DEPENDENT_PARSER = ::T.unsafe(nil)
  CASE_INDEPENDENT_PARSER = ::T.unsafe(nil)
  CRLF = ::T.unsafe(nil)
  DECIMAL_PARSER = ::T.unsafe(nil)
  DEFAULT_BLOCKSIZE = ::T.unsafe(nil)
  FACT_PARSERS = ::T.unsafe(nil)
  FTP_PORT = ::T.unsafe(nil)
  OCTAL_PARSER = ::T.unsafe(nil)
  TIME_PARSER = ::T.unsafe(nil)

  # Aborts the previous command (ABOR command).
  def abort(); end

  # Sends the ACCT command.
  #
  # This is a less common
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) command, to send
  # account information if the destination host requires it.
  def acct(account); end

  # When `true`, transfers are performed in binary mode. Default: `true`.
  def binary(); end

  # A setter to toggle transfers in binary mode. `newmode` is either `true` or
  # `false`
  def binary=(newmode); end

  # Changes the (remote) directory.
  def chdir(dirname); end

  # Closes the connection. Further operations are impossible until you open a
  # new connection with
  # [`connect`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-connect).
  def close(); end

  # Returns `true` iff the connection is closed.
  def closed?(); end

  # Establishes an [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html)
  # connection to host, optionally overriding the default port. If the
  # environment variable `SOCKS_SERVER` is set, sets up the connection through a
  # SOCKS proxy. Raises an exception (typically `Errno::ECONNREFUSED`) if the
  # connection cannot be established.
  def connect(host, port=T.unsafe(nil)); end

  # When `true`, all traffic to and from the server is written to +$stdout+.
  # Default: `false`.
  def debug_mode(); end

  # When `true`, all traffic to and from the server is written to +$stdout+.
  # Default: `false`.
  def debug_mode=(debug_mode); end

  # Deletes a file on the server.
  def delete(filename); end

  # Alias for:
  # [`list`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-list)
  def dir(*args, &block); end

  # Retrieves `remotefile` in whatever mode the session is set (text or binary).
  # See
  # [`gettextfile`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-gettextfile)
  # and
  # [`getbinaryfile`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-getbinaryfile).
  def get(remotefile, localfile=T.unsafe(nil), blocksize=T.unsafe(nil), &block); end

  # Retrieves `remotefile` in binary mode, storing the result in `localfile`. If
  # `localfile` is nil, returns retrieved data. If a block is supplied, it is
  # passed the retrieved data in `blocksize` chunks.
  def getbinaryfile(remotefile, localfile=T.unsafe(nil), blocksize=T.unsafe(nil), &block); end

  # Alias for:
  # [`pwd`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-pwd)
  def getdir(); end

  # Retrieves `remotefile` in ASCII (text) mode, storing the result in
  # `localfile`. If `localfile` is nil, returns retrieved data. If a block is
  # supplied, it is passed the retrieved data one line at a time.
  def gettextfile(remotefile, localfile=T.unsafe(nil), &block); end

  # Issues the HELP command.
  def help(arg=T.unsafe(nil)); end

  def initialize(host=T.unsafe(nil), user_or_options=T.unsafe(nil), passwd=T.unsafe(nil), acct=T.unsafe(nil)); end

  # The server's last response.
  def last_response(); end

  # The server's last response code.
  def last_response_code(); end

  # The server's last response code.
  def lastresp(); end

  # Returns an array of file information in the directory (the output is like
  # `ls -l`). If a block is given, it iterates through the listing.
  #
  # Also aliased as:
  # [`ls`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-ls),
  # [`dir`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-dir)
  def list(*args, &block); end

  # Logs in to the remote host. The session must have been previously connected.
  # If `user` is the string "anonymous" and the `password` is `nil`,
  # "anonymous@" is used as a password. If the `acct` parameter is not `nil`, an
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) ACCT command is
  # sent following the successful login. Raises an exception on error (typically
  # `Net::FTPPermError`).
  def login(user=T.unsafe(nil), passwd=T.unsafe(nil), acct=T.unsafe(nil)); end

  # Alias for:
  # [`list`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-list)
  def ls(*args, &block); end

  # Returns the raw last modification time of the (remote) file in the format
  # "YYYYMMDDhhmmss" (MDTM command).
  #
  # Use `mtime` if you want a parsed
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) instance.
  def mdtm(filename); end

  # Creates a remote directory.
  def mkdir(dirname); end

  # Returns an array of the entries of the directory specified by `pathname`.
  # Each entry has the facts (e.g., size, last modification time, etc.) and the
  # pathname. If a block is given, it iterates through the listing. If
  # `pathname` is omitted, the current directory is assumed.
  def mlsd(pathname=T.unsafe(nil), &block); end

  # Returns data (e.g., size, last modification time, entry type, etc.) about
  # the file or directory specified by `pathname`. If `pathname` is omitted, the
  # current directory is assumed.
  def mlst(pathname=T.unsafe(nil)); end

  # Returns the last modification time of the (remote) file. If `local` is
  # `true`, it is returned as a local time, otherwise it's a UTC time.
  def mtime(filename, local=T.unsafe(nil)); end

  # Returns an array of filenames in the remote directory.
  def nlst(dir=T.unsafe(nil)); end

  # Issues a NOOP command.
  #
  # Does nothing except return a response.
  def noop(); end

  # Number of seconds to wait for the connection to open. Any number may be
  # used, including Floats for fractional seconds. If the
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) object cannot open
  # a connection in this many seconds, it raises a
  # [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # exception. The default value is `nil`.
  def open_timeout(); end

  # Number of seconds to wait for the connection to open. Any number may be
  # used, including Floats for fractional seconds. If the
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) object cannot open
  # a connection in this many seconds, it raises a
  # [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # exception. The default value is `nil`.
  def open_timeout=(open_timeout); end

  # When `true`, the connection is in passive mode. Default: `true`.
  def passive(); end

  # When `true`, the connection is in passive mode. Default: `true`.
  def passive=(passive); end

  # Transfers `localfile` to the server in whatever mode the session is set
  # (text or binary). See
  # [`puttextfile`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-puttextfile)
  # and
  # [`putbinaryfile`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-putbinaryfile).
  def put(localfile, remotefile=T.unsafe(nil), blocksize=T.unsafe(nil), &block); end

  # Transfers `localfile` to the server in binary mode, storing the result in
  # `remotefile`. If a block is supplied, calls it, passing in the transmitted
  # data in `blocksize` chunks.
  def putbinaryfile(localfile, remotefile=T.unsafe(nil), blocksize=T.unsafe(nil), &block); end

  # Transfers `localfile` to the server in ASCII (text) mode, storing the result
  # in `remotefile`. If callback or an associated block is supplied, calls it,
  # passing in the transmitted data one line at a time.
  def puttextfile(localfile, remotefile=T.unsafe(nil), &block); end

  # Returns the current remote directory.
  #
  # Also aliased as:
  # [`getdir`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#method-i-getdir)
  def pwd(); end

  # Exits the [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) session.
  def quit(); end

  # Number of seconds to wait for one block to be read (via one read(2) call).
  # Any number may be used, including Floats for fractional seconds. If the
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) object cannot read
  # data in this many seconds, it raises a
  # [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html)
  # exception. The default value is 60 seconds.
  def read_timeout(); end

  # Setter for the
  # [`read_timeout`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html#attribute-i-read_timeout)
  # attribute.
  def read_timeout=(sec); end

  # Renames a file on the server.
  def rename(fromname, toname); end

  # Sets or retrieves the `resume` status, which decides whether incomplete
  # transfers are resumed or restarted. Default: `false`.
  def resume(); end

  # Sets or retrieves the `resume` status, which decides whether incomplete
  # transfers are resumed or restarted. Default: `false`.
  def resume=(resume); end

  # Puts the connection into binary (image) mode, issues the given command, and
  # fetches the data returned, passing it to the associated block in chunks of
  # `blocksize` characters. Note that `cmd` is a server command (such as "RETR
  # myfile").
  def retrbinary(cmd, blocksize, rest_offset=T.unsafe(nil)); end

  # Puts the connection into ASCII (text) mode, issues the given command, and
  # passes the resulting data, one line at a time, to the associated block. If
  # no block is given, prints the lines. Note that `cmd` is a server command
  # (such as "RETR myfile").
  def retrlines(cmd); end

  def return_code(); end

  def return_code=(s); end

  # Removes a remote directory.
  def rmdir(dirname); end

  # Sends a command and returns the response.
  def sendcmd(cmd); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the socket used to
  # connect to the [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html)
  # server.
  #
  # May raise FTPReplyError if `get_greeting` is false.
  def set_socket(sock, get_greeting=T.unsafe(nil)); end

  # Issues a SITE command.
  def site(arg); end

  # Returns the size of the given (remote) filename.
  def size(filename); end

  # Number of seconds to wait for the TLS handshake. Any number may be used,
  # including Floats for fractional seconds. If the
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) object cannot
  # complete the TLS handshake in this many seconds, it raises a
  # [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # exception. The default value is `nil`. If `ssl_handshake_timeout` is `nil`,
  # `open_timeout` is used instead.
  def ssl_handshake_timeout(); end

  # Number of seconds to wait for the TLS handshake. Any number may be used,
  # including Floats for fractional seconds. If the
  # [`FTP`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP.html) object cannot
  # complete the TLS handshake in this many seconds, it raises a
  # [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # exception. The default value is `nil`. If `ssl_handshake_timeout` is `nil`,
  # `open_timeout` is used instead.
  def ssl_handshake_timeout=(ssl_handshake_timeout); end

  # Returns the status (STAT command).
  #
  # pathname
  # :   when stat is invoked with pathname as a parameter it acts like list but
  #     a lot faster and over the same tcp session.
  def status(pathname=T.unsafe(nil)); end

  # Puts the connection into binary (image) mode, issues the given server-side
  # command (such as "STOR myfile"), and sends the contents of the file named
  # `file` to the server. If the optional block is given, it also passes it the
  # data, in chunks of `blocksize` characters.
  def storbinary(cmd, file, blocksize, rest_offset=T.unsafe(nil)); end

  # Puts the connection into ASCII (text) mode, issues the given server-side
  # command (such as "STOR myfile"), and sends the contents of the file named
  # `file` to the server, one line at a time. If the optional block is given, it
  # also passes it the lines.
  def storlines(cmd, file); end

  # Returns system information.
  def system(); end

  # Sends a command and expect a response beginning with '2'.
  def voidcmd(cmd); end

  # The server's welcome message.
  def welcome(); end

  # When `true`, connections are in passive mode per default. Default: `true`.
  def self.default_passive(); end

  # When `true`, connections are in passive mode per default. Default: `true`.
  def self.default_passive=(value); end

  # A synonym for `FTP.new`, but with a mandatory host parameter.
  #
  # If a block is given, it is passed the `FTP` object, which will be closed
  # when the block finishes, or when an exception is raised.
  def self.open(host, *args); end
end

class Net::FTP::BufferedSSLSocket < Net::FTP::BufferedSocket
  def initialize(*args); end

  def send(mesg, flags, dest=T.unsafe(nil)); end

  def shutdown(*args); end
end

class Net::FTP::BufferedSocket < Net::BufferedIO
  def addr(*args); end

  def gets(); end

  def local_address(*args); end

  def peeraddr(*args); end

  def read(len=T.unsafe(nil)); end

  def readline(); end

  def remote_address(*args); end

  def send(*args); end

  def shutdown(*args); end
end

# [`MLSxEntry`](https://docs.ruby-lang.org/en/2.7.0/Net/FTP/MLSxEntry.html)
# represents an entry in responses of MLST/MLSD. Each entry has the facts (e.g.,
# size, last modification time, etc.) and the pathname.
class Net::FTP::MLSxEntry
  # Returns `true` if the APPE command may be applied to the file.
  def appendable?(); end

  def charset(); end

  # Returns `true` if files may be created in the directory by STOU, STOR, APPE,
  # and RNTO.
  def creatable?(); end

  def create(); end

  # Returns `true` if the file or directory may be deleted by DELE/RMD.
  def deletable?(); end

  # Returns `true` if the entry is a directory (i.e., the value of the type fact
  # is dir, cdir, or pdir).
  def directory?(); end

  # Returns `true` if the MKD command may be used to create a new directory
  # within the directory.
  def directory_makable?(); end

  # Returns `true` if the directory may be entered by CWD/CDUP.
  def enterable?(); end

  def facts(); end

  # Returns `true` if the entry is a file (i.e., the value of the type fact is
  # file).
  def file?(); end

  def initialize(facts, pathname); end

  def lang(); end

  # Returns `true` if the listing commands, LIST, NLST, and MLSD are applied to
  # the directory.
  def listable?(); end

  def media_type(); end

  def modify(); end

  def pathname(); end

  def perm(); end

  # Returns `true` if the objects in the directory may be deleted, or the
  # directory may be purged.
  def purgeable?(); end

  # Returns `true` if the RETR command may be applied to the file.
  def readable?(); end

  # Returns `true` if the file or directory may be renamed by RNFR.
  def renamable?(); end

  def size(); end

  def type(); end

  def unique(); end

  # Returns `true` if the STOR command may be applied to the file.
  def writable?(); end
end

class Net::FTP::NullSocket
  def close(); end

  def closed?(); end

  def method_missing(mid, *args); end

  def read_timeout=(sec); end
end

class Net::FTPConnectionError < Net::FTPError
end

class Net::FTPError < StandardError
end

class Net::FTPPermError < Net::FTPError
end

class Net::FTPProtoError < Net::FTPError
end

class Net::FTPReplyError < Net::FTPError
end

class Net::FTPTempError < Net::FTPError
end

# ## An [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) client API for Ruby.
#
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) provides a
# rich library which can be used to build
# [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) user-agents. For
# more details about [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
# see [RFC2616](http://www.ietf.org/rfc/rfc2616.txt).
#
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) is designed
# to work closely with [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html).
# [`URI::HTTP#host`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#attribute-i-host),
# [`URI::HTTP#port`](https://docs.ruby-lang.org/en/2.7.0/URI/Generic.html#attribute-i-port)
# and
# [`URI::HTTP#request_uri`](https://docs.ruby-lang.org/en/2.7.0/URI/HTTP.html#method-i-request_uri)
# are designed to work with
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html).
#
# If you are only performing a few GET requests you should try
# [`OpenURI`](https://docs.ruby-lang.org/en/2.7.0/OpenURI.html).
#
# ## Simple Examples
#
# All examples assume you have loaded
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) with:
#
# ```ruby
# require 'net/http'
# ```
#
# This will also require 'uri' so you don't need to require it separately.
#
# The [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) methods
# in the following section do not persist connections. They are not recommended
# if you are performing many
# [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) requests.
#
# ### GET
#
# ```ruby
# Net::HTTP.get('example.com', '/index.html') # => String
# ```
#
# ### GET by [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
#
# ```ruby
# uri = URI('http://example.com/index.html?count=10')
# Net::HTTP.get(uri) # => String
# ```
#
# ### GET with Dynamic Parameters
#
# ```ruby
# uri = URI('http://example.com/index.html')
# params = { :limit => 10, :page => 3 }
# uri.query = URI.encode_www_form(params)
#
# res = Net::HTTP.get_response(uri)
# puts res.body if res.is_a?(Net::HTTPSuccess)
# ```
#
# ### POST
#
# ```ruby
# uri = URI('http://www.example.com/search.cgi')
# res = Net::HTTP.post_form(uri, 'q' => 'ruby', 'max' => '50')
# puts res.body
# ```
#
# ### POST with Multiple Values
#
# ```ruby
# uri = URI('http://www.example.com/search.cgi')
# res = Net::HTTP.post_form(uri, 'q' => ['ruby', 'perl'], 'max' => '50')
# puts res.body
# ```
#
# ## How to use [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
#
# The following example code can be used as the basis of an
# [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) user-agent which
# can perform a variety of request types using persistent connections.
#
# ```ruby
# uri = URI('http://example.com/some_path?query=string')
#
# Net::HTTP.start(uri.host, uri.port) do |http|
#   request = Net::HTTP::Get.new uri
#
#   response = http.request request # Net::HTTPResponse object
# end
# ```
#
# [`Net::HTTP::start`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-c-start)
# immediately creates a connection to an
# [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) server which is
# kept open for the duration of the block. The connection will remain open for
# multiple requests in the block if the server indicates it supports persistent
# connections.
#
# If you wish to re-use a connection across multiple
# [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) requests without
# automatically closing it you can use
# [`::new`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-c-new) and
# then call
# [`start`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-start)
# and
# [`finish`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-finish)
# manually.
#
# The request types
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) supports are
# listed below in the section "HTTP Request Classes".
#
# For all the [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
# request objects and shortcut request methods you may supply either a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) for the request
# path or a [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) from which
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) will extract
# the request path.
#
# ### Response [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html)
#
# ```ruby
# uri = URI('http://example.com/index.html')
# res = Net::HTTP.get_response(uri)
#
# # Headers
# res['Set-Cookie']            # => String
# res.get_fields('set-cookie') # => Array
# res.to_hash['set-cookie']    # => Array
# puts "Headers: #{res.to_hash.inspect}"
#
# # Status
# puts res.code       # => '200'
# puts res.message    # => 'OK'
# puts res.class.name # => 'HTTPOK'
#
# # Body
# puts res.body if res.response_body_permitted?
# ```
#
# ### Following Redirection
#
# Each
# [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
# object belongs to a class for its response code.
#
# For example, all 2XX responses are instances of a
# [`Net::HTTPSuccess`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPSuccess.html)
# subclass, a 3XX response is an instance of a
# [`Net::HTTPRedirection`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPRedirection.html)
# subclass and a 200 response is an instance of the
# [`Net::HTTPOK`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPOK.html) class.
# For details of response classes, see the section "HTTP Response Classes"
# below.
#
# Using a case statement you can handle various types of responses properly:
#
# ```ruby
# def fetch(uri_str, limit = 10)
#   # You should choose a better exception.
#   raise ArgumentError, 'too many HTTP redirects' if limit == 0
#
#   response = Net::HTTP.get_response(URI(uri_str))
#
#   case response
#   when Net::HTTPSuccess then
#     response
#   when Net::HTTPRedirection then
#     location = response['location']
#     warn "redirected to #{location}"
#     fetch(location, limit - 1)
#   else
#     response.value
#   end
# end
#
# print fetch('http://www.ruby-lang.org')
# ```
#
# ### POST
#
# A POST can be made using the
# [`Net::HTTP::Post`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Post.html)
# request class. This example creates a URL encoded POST body:
#
# ```ruby
# uri = URI('http://www.example.com/todo.cgi')
# req = Net::HTTP::Post.new(uri)
# req.set_form_data('from' => '2005-01-01', 'to' => '2005-03-31')
#
# res = Net::HTTP.start(uri.hostname, uri.port) do |http|
#   http.request(req)
# end
#
# case res
# when Net::HTTPSuccess, Net::HTTPRedirection
#   # OK
# else
#   res.value
# end
# ```
#
# To send multipart/form-data use
# [`Net::HTTPHeader#set_form`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-set_form):
#
# ```ruby
# req = Net::HTTP::Post.new(uri)
# req.set_form([['upload', File.open('foo.bar')]], 'multipart/form-data')
# ```
#
# Other requests that can contain a body such as PUT can be created in the same
# way using the corresponding request class
# ([`Net::HTTP::Put`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Put.html)).
#
# ### Setting Headers
#
# The following example performs a conditional GET using the If-Modified-Since
# header. If the files has not been modified since the time in the header a Not
# Modified response will be returned. See RFC 2616 section 9.3 for further
# details.
#
# ```ruby
# uri = URI('http://example.com/cached_response')
# file = File.stat 'cached_response'
#
# req = Net::HTTP::Get.new(uri)
# req['If-Modified-Since'] = file.mtime.rfc2822
#
# res = Net::HTTP.start(uri.hostname, uri.port) {|http|
#   http.request(req)
# }
#
# open 'cached_response', 'w' do |io|
#   io.write res.body
# end if res.is_a?(Net::HTTPSuccess)
# ```
#
# ### Basic Authentication
#
# Basic authentication is performed according to
# [RFC2617](http://www.ietf.org/rfc/rfc2617.txt).
#
# ```ruby
# uri = URI('http://example.com/index.html?key=value')
#
# req = Net::HTTP::Get.new(uri)
# req.basic_auth 'user', 'pass'
#
# res = Net::HTTP.start(uri.hostname, uri.port) {|http|
#   http.request(req)
# }
# puts res.body
# ```
#
# ### Streaming Response Bodies
#
# By default [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
# reads an entire response into memory. If you are handling large files or wish
# to implement a progress bar you can instead stream the body directly to an
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
#
# ```ruby
# uri = URI('http://example.com/large_file')
#
# Net::HTTP.start(uri.host, uri.port) do |http|
#   request = Net::HTTP::Get.new uri
#
#   http.request request do |response|
#     open 'large_file', 'w' do |io|
#       response.read_body do |chunk|
#         io.write chunk
#       end
#     end
#   end
# end
# ```
#
# ### HTTPS
#
# HTTPS is enabled for an
# [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) connection by
# [`Net::HTTP#use_ssl=`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-use_ssl-3D).
#
# ```ruby
# uri = URI('https://secure.example.com/some_path?query=string')
#
# Net::HTTP.start(uri.host, uri.port, :use_ssl => true) do |http|
#   request = Net::HTTP::Get.new uri
#   response = http.request request # Net::HTTPResponse object
# end
# ```
#
# Or if you simply want to make a GET request, you may pass in an
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) object that has an HTTPS
# URL. [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
# automatically turns on TLS verification if the
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) object has a 'https'
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) scheme.
#
# ```ruby
# uri = URI('https://example.com/')
# Net::HTTP.get(uri) # => String
# ```
#
# In previous versions of Ruby you would need to require 'net/https' to use
# HTTPS. This is no longer true.
#
# ### Proxies
#
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) will
# automatically create a proxy from the `http_proxy` environment variable if it
# is present. To disable use of `http_proxy`, pass `nil` for the proxy address.
#
# You may also create a custom proxy:
#
# ```ruby
# proxy_addr = 'your.proxy.host'
# proxy_port = 8080
#
# Net::HTTP.new('example.com', nil, proxy_addr, proxy_port).start { |http|
#   # always proxy via your.proxy.addr:8080
# }
# ```
#
# See
# [`Net::HTTP.new`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-c-new)
# for further details and examples such as proxies that require a username and
# password.
#
# ### Compression
#
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) automatically
# adds Accept-Encoding for compression of response bodies and automatically
# decompresses gzip and deflate responses unless a
# [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) header was sent.
#
# Compression can be disabled through the Accept-Encoding: identity header.
#
# ## [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) Request Classes
#
# Here is the [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
# request class hierarchy.
#
# *   [`Net::HTTPRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPRequest.html)
#     *   [`Net::HTTP::Get`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Get.html)
#     *   [`Net::HTTP::Head`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Head.html)
#     *   [`Net::HTTP::Post`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Post.html)
#     *   [`Net::HTTP::Patch`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Patch.html)
#     *   [`Net::HTTP::Put`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Put.html)
#     *   [`Net::HTTP::Proppatch`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Proppatch.html)
#     *   [`Net::HTTP::Lock`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Lock.html)
#     *   [`Net::HTTP::Unlock`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Unlock.html)
#     *   [`Net::HTTP::Options`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Options.html)
#     *   [`Net::HTTP::Propfind`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Propfind.html)
#     *   [`Net::HTTP::Delete`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Delete.html)
#     *   [`Net::HTTP::Move`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Move.html)
#     *   [`Net::HTTP::Copy`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Copy.html)
#     *   [`Net::HTTP::Mkcol`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Mkcol.html)
#     *   [`Net::HTTP::Trace`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Trace.html)
#
#
#
# ## [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) Response Classes
#
# Here is [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) response
# class hierarchy. All classes are defined in
# [`Net`](https://docs.ruby-lang.org/en/2.7.0/Net.html) module and are
# subclasses of
# [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html).
#
# HTTPUnknownResponse
# :   For unhandled [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
#     extensions
# HTTPInformation
# :   1xx
# HTTPContinue
# :   100
# HTTPSwitchProtocol
# :   101
# HTTPSuccess
# :   2xx
# HTTPOK
# :   200
# HTTPCreated
# :   201
# HTTPAccepted
# :   202
# HTTPNonAuthoritativeInformation
# :   203
# HTTPNoContent
# :   204
# HTTPResetContent
# :   205
# HTTPPartialContent
# :   206
# HTTPMultiStatus
# :   207
# HTTPIMUsed
# :   226
# HTTPRedirection
# :   3xx
# HTTPMultipleChoices
# :   300
# HTTPMovedPermanently
# :   301
# HTTPFound
# :   302
# HTTPSeeOther
# :   303
# HTTPNotModified
# :   304
# HTTPUseProxy
# :   305
# HTTPTemporaryRedirect
# :   307
# HTTPClientError
# :   4xx
# HTTPBadRequest
# :   400
# HTTPUnauthorized
# :   401
# HTTPPaymentRequired
# :   402
# HTTPForbidden
# :   403
# HTTPNotFound
# :   404
# HTTPMethodNotAllowed
# :   405
# HTTPNotAcceptable
# :   406
# HTTPProxyAuthenticationRequired
# :   407
# [`HTTPRequestTimeOut`](https://docs.ruby-lang.org/en/2.7.0/HTTPRequestTimeOut.html)
# :   408
# HTTPConflict
# :   409
# HTTPGone
# :   410
# HTTPLengthRequired
# :   411
# HTTPPreconditionFailed
# :   412
# [`HTTPRequestEntityTooLarge`](https://docs.ruby-lang.org/en/2.7.0/HTTPRequestEntityTooLarge.html)
# :   413
# [`HTTPRequestURITooLong`](https://docs.ruby-lang.org/en/2.7.0/HTTPRequestURITooLong.html)
# :   414
# HTTPUnsupportedMediaType
# :   415
# [`HTTPRequestedRangeNotSatisfiable`](https://docs.ruby-lang.org/en/2.7.0/HTTPRequestedRangeNotSatisfiable.html)
# :   416
# HTTPExpectationFailed
# :   417
# HTTPUnprocessableEntity
# :   422
# HTTPLocked
# :   423
# HTTPFailedDependency
# :   424
# HTTPUpgradeRequired
# :   426
# HTTPPreconditionRequired
# :   428
# HTTPTooManyRequests
# :   429
# HTTPRequestHeaderFieldsTooLarge
# :   431
# HTTPUnavailableForLegalReasons
# :   451
# HTTPServerError
# :   5xx
# HTTPInternalServerError
# :   500
# HTTPNotImplemented
# :   501
# HTTPBadGateway
# :   502
# HTTPServiceUnavailable
# :   503
# [`HTTPGatewayTimeOut`](https://docs.ruby-lang.org/en/2.7.0/HTTPGatewayTimeOut.html)
# :   504
# HTTPVersionNotSupported
# :   505
# HTTPInsufficientStorage
# :   507
# HTTPNetworkAuthenticationRequired
# :   511
#
#
# There is also the
# [`Net::HTTPBadResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPBadResponse.html)
# exception which is raised when there is a protocol error.
class Net::HTTP < Net::Protocol
  HAVE_ZLIB = ::T.unsafe(nil)
  HTTPVersion = ::T.unsafe(nil)
  IDEMPOTENT_METHODS_ = ::T.unsafe(nil)
  Revision = ::T.unsafe(nil)
  SSL_ATTRIBUTES = ::T.unsafe(nil)
  SSL_IVNAMES = ::T.unsafe(nil)

  # Alias for:
  # [`started?`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-started-3F)
  def active?(); end

  # The DNS host name or IP address to connect to.
  def address(); end

  # Sets path of a CA certification file in PEM format.
  #
  # The file can contain several CA certificates.
  def ca_file(); end

  # Sets path of a CA certification file in PEM format.
  #
  # The file can contain several CA certificates.
  def ca_file=(ca_file); end

  # Sets path of a CA certification directory containing certifications in PEM
  # format.
  def ca_path(); end

  # Sets path of a CA certification directory containing certifications in PEM
  # format.
  def ca_path=(ca_path); end

  # Sets an
  # [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html)
  # object as client certificate. (This method is appeared in Michal Rokos's
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) extension).
  def cert(); end

  # Sets an
  # [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html)
  # object as client certificate. (This method is appeared in Michal Rokos's
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) extension).
  def cert=(cert); end

  # Sets the X509::Store to verify peer certificate.
  def cert_store(); end

  # Sets the X509::Store to verify peer certificate.
  def cert_store=(cert_store); end

  # Sets the available ciphers. See
  # [`OpenSSL::SSL::SSLContext#ciphers=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-ciphers-3D)
  def ciphers(); end

  # Sets the available ciphers. See
  # [`OpenSSL::SSL::SSLContext#ciphers=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-ciphers-3D)
  def ciphers=(ciphers); end

  def close_on_empty_response(); end

  def close_on_empty_response=(close_on_empty_response); end

  # Seconds to wait for 100 Continue response. If the
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object does not
  # receive a response in this many seconds it sends the request body. The
  # default value is `nil`.
  def continue_timeout(); end

  # Setter for the
  # [`continue_timeout`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-continue_timeout)
  # attribute.
  def continue_timeout=(sec); end

  # Sends a COPY request to the `path` and gets a response, as an HTTPResponse
  # object.
  def copy(path, initheader=T.unsafe(nil)); end

  # Sends a DELETE request to the `path` and gets a response, as an HTTPResponse
  # object.
  def delete(path, initheader=T.unsafe(nil)); end

  # Finishes the [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
  # session and closes the TCP connection. Raises
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) if the session
  # has not been started.
  def finish(); end

  # Retrieves data from `path` on the connected-to host which may be an absolute
  # path [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or a
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) to extract the path
  # from.
  #
  # `initheader` must be a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) like { 'Accept' =>
  # '**/**', ... }, and it defaults to an empty hash. If `initheader` doesn't
  # have the key 'accept-encoding', then a value of
  # "gzip;q=1.0,deflate;q=0.6,identity;q=0.3" is used, so that gzip compression
  # is used in preference to deflate compression, which is used in preference to
  # no compression. Ruby doesn't have libraries to support the compress
  # (Lempel-Ziv) compression, so that is not supported. The intent of this is to
  # reduce bandwidth by default.  If this routine sets up compression, then it
  # does the decompression also, removing the header as well to prevent
  # confusion. Otherwise it leaves the body as it found it.
  #
  # This method returns a
  # [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object.
  #
  # If called with a block, yields each fragment of the entity body in turn as a
  # string as it is read from the socket. Note that in this case, the returned
  # response object will **not** contain a (meaningful) body.
  #
  # `dest` argument is obsolete. It still works but you must not use it.
  #
  # This method never raises an exception.
  #
  # ```ruby
  # response = http.get('/index.html')
  #
  # # using block
  # File.open('result.txt', 'w') {|f|
  #   http.get('/~foo/') do |str|
  #     f.write str
  #   end
  # }
  # ```
  def get(path, initheader=T.unsafe(nil), dest=T.unsafe(nil), &block); end

  # Alias for:
  # [`request_get`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-request_get)
  def get2(path, initheader=T.unsafe(nil), &block); end

  # Gets only the header from `path` on the connected-to host. `header` is a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) like { 'Accept' =>
  # '**/**', ... }.
  #
  # This method returns a
  # [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object.
  #
  # This method never raises an exception.
  #
  # ```ruby
  # response = nil
  # Net::HTTP.start('some.www.server', 80) {|http|
  #   response = http.head('/index.html')
  # }
  # p response['content-type']
  # ```
  def head(path, initheader=T.unsafe(nil)); end

  # Alias for:
  # [`request_head`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-request_head)
  def head2(path, initheader=T.unsafe(nil), &block); end

  def initialize(address, port=T.unsafe(nil)); end

  def inspect(); end

  # Seconds to reuse the connection of the previous request. If the idle time is
  # less than this Keep-Alive
  # [`Timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html),
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) reuses the
  # TCP/IP socket used by the previous communication. The default value is 2
  # seconds.
  def keep_alive_timeout(); end

  # Seconds to reuse the connection of the previous request. If the idle time is
  # less than this Keep-Alive
  # [`Timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html),
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) reuses the
  # TCP/IP socket used by the previous communication. The default value is 2
  # seconds.
  def keep_alive_timeout=(keep_alive_timeout); end

  # Sets an
  # [`OpenSSL::PKey::RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html)
  # or
  # [`OpenSSL::PKey::DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html)
  # object. (This method is appeared in Michal Rokos's
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) extension.)
  def key(); end

  # Sets an
  # [`OpenSSL::PKey::RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html)
  # or
  # [`OpenSSL::PKey::DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html)
  # object. (This method is appeared in Michal Rokos's
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) extension.)
  def key=(key); end

  # The local host used to establish the connection.
  def local_host(); end

  # The local host used to establish the connection.
  def local_host=(local_host); end

  # The local port used to establish the connection.
  def local_port(); end

  # The local port used to establish the connection.
  def local_port=(local_port); end

  # Sends a LOCK request to the `path` and gets a response, as an HTTPResponse
  # object.
  def lock(path, body, initheader=T.unsafe(nil)); end

  # Sends a MKCOL request to the `path` and gets a response, as an HTTPResponse
  # object.
  def mkcol(path, body=T.unsafe(nil), initheader=T.unsafe(nil)); end

  # Sends a MOVE request to the `path` and gets a response, as an HTTPResponse
  # object.
  def move(path, initheader=T.unsafe(nil)); end

  # Number of seconds to wait for the connection to open. Any number may be
  # used, including Floats for fractional seconds. If the
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object cannot
  # open a connection in this many seconds, it raises a
  # [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # exception. The default value is 60 seconds.
  def open_timeout(); end

  # Number of seconds to wait for the connection to open. Any number may be
  # used, including Floats for fractional seconds. If the
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object cannot
  # open a connection in this many seconds, it raises a
  # [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # exception. The default value is 60 seconds.
  def open_timeout=(open_timeout); end

  # Sends a OPTIONS request to the `path` and gets a response, as an
  # HTTPResponse object.
  def options(path, initheader=T.unsafe(nil)); end

  # Sends a PATCH request to the `path` and gets a response, as an HTTPResponse
  # object.
  def patch(path, data, initheader=T.unsafe(nil), dest=T.unsafe(nil), &block); end

  # Returns the X.509 certificates the server presented.
  def peer_cert(); end

  # The port number to connect to.
  def port(); end

  # Posts `data` (must be a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)) to `path`.
  # `header` must be a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  # like { 'Accept' => '**/**', ... }.
  #
  # This method returns a
  # [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object.
  #
  # If called with a block, yields each fragment of the entity body in turn as a
  # string as it is read from the socket. Note that in this case, the returned
  # response object will **not** contain a (meaningful) body.
  #
  # `dest` argument is obsolete. It still works but you must not use it.
  #
  # This method never raises exception.
  #
  # ```ruby
  # response = http.post('/cgi-bin/search.rb', 'query=foo')
  #
  # # using block
  # File.open('result.txt', 'w') {|f|
  #   http.post('/cgi-bin/search.rb', 'query=foo') do |str|
  #     f.write str
  #   end
  # }
  # ```
  #
  # You should set Content-Type: header field for POST. If no Content-Type:
  # field given, this method uses "application/x-www-form-urlencoded" by
  # default.
  def post(path, data, initheader=T.unsafe(nil), dest=T.unsafe(nil), &block); end

  # Alias for:
  # [`request_post`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-request_post)
  def post2(path, data, initheader=T.unsafe(nil), &block); end

  # Sends a PROPFIND request to the `path` and gets a response, as an
  # HTTPResponse object.
  def propfind(path, body=T.unsafe(nil), initheader=T.unsafe(nil)); end

  # Sends a PROPPATCH request to the `path` and gets a response, as an
  # HTTPResponse object.
  def proppatch(path, body, initheader=T.unsafe(nil)); end

  # True if requests for this connection will be proxied
  def proxy?(); end

  def proxy_address(); end

  def proxy_address=(proxy_address); end

  def proxy_from_env=(proxy_from_env); end

  # True if the proxy for this connection is determined from the environment
  def proxy_from_env?(); end

  def proxy_pass(); end

  def proxy_pass=(proxy_pass); end

  def proxy_port(); end

  def proxy_port=(proxy_port); end

  def proxy_uri(); end

  def proxy_user(); end

  def proxy_user=(proxy_user); end

  # Alias for:
  # [`proxy_address`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-proxy_address)
  def proxyaddr(); end

  # Alias for:
  # [`proxy_port`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-proxy_port)
  def proxyport(); end

  def put(path, data, initheader=T.unsafe(nil)); end

  def put2(path, data, initheader=T.unsafe(nil), &block); end

  # Number of seconds to wait for one block to be read (via one read(2) call).
  # Any number may be used, including Floats for fractional seconds. If the
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object cannot
  # read data in this many seconds, it raises a
  # [`Net::ReadTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/ReadTimeout.html)
  # exception. The default value is 60 seconds.
  def read_timeout(); end

  # Setter for the
  # [`read_timeout`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-read_timeout)
  # attribute.
  def read_timeout=(sec); end

  # Sends an HTTPRequest object `req` to the
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) server.
  #
  # If `req` is a
  # [`Net::HTTP::Post`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Post.html)
  # or [`Net::HTTP::Put`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Put.html)
  # request containing data, the data is also sent. Providing data for a
  # [`Net::HTTP::Head`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Head.html)
  # or [`Net::HTTP::Get`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Get.html)
  # request results in an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html).
  #
  # Returns an HTTPResponse object.
  #
  # When called with a block, passes an HTTPResponse object to the block. The
  # body of the response will not have been read yet; the block can process it
  # using HTTPResponse#read\_body, if desired.
  #
  # This method never raises Net::\* exceptions.
  def request(req, body=T.unsafe(nil), &block); end

  # Sends a GET request to the `path`. Returns the response as a
  # [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object.
  #
  # When called with a block, passes an HTTPResponse object to the block. The
  # body of the response will not have been read yet; the block can process it
  # using HTTPResponse#read\_body, if desired.
  #
  # Returns the response.
  #
  # This method never raises Net::\* exceptions.
  #
  # ```ruby
  # response = http.request_get('/index.html')
  # # The entity body is already read in this case.
  # p response['content-type']
  # puts response.body
  #
  # # Using a block
  # http.request_get('/index.html') {|response|
  #   p response['content-type']
  #   response.read_body do |str|   # read body now
  #     print str
  #   end
  # }
  # ```
  #
  #
  # Also aliased as:
  # [`get2`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-get2)
  def request_get(path, initheader=T.unsafe(nil), &block); end

  # Sends a HEAD request to the `path` and returns the response as a
  # [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object.
  #
  # Returns the response.
  #
  # This method never raises Net::\* exceptions.
  #
  # ```ruby
  # response = http.request_head('/index.html')
  # p response['content-type']
  # ```
  #
  #
  # Also aliased as:
  # [`head2`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-head2)
  def request_head(path, initheader=T.unsafe(nil), &block); end

  # Sends a POST request to the `path`.
  #
  # Returns the response as a
  # [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object.
  #
  # When called with a block, the block is passed an HTTPResponse object. The
  # body of that response will not have been read yet; the block can process it
  # using HTTPResponse#read\_body, if desired.
  #
  # Returns the response.
  #
  # This method never raises Net::\* exceptions.
  #
  # ```ruby
  # # example
  # response = http.request_post('/cgi-bin/nice.rb', 'datadatadata...')
  # p response.status
  # puts response.body          # body is already read in this case
  #
  # # using block
  # http.request_post('/cgi-bin/nice.rb', 'datadatadata...') {|response|
  #   p response.status
  #   p response['content-type']
  #   response.read_body do |str|   # read body now
  #     print str
  #   end
  # }
  # ```
  #
  #
  # Also aliased as:
  # [`post2`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-post2)
  def request_post(path, data, initheader=T.unsafe(nil), &block); end

  def request_put(path, data, initheader=T.unsafe(nil), &block); end

  # Sends an [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) request
  # to the [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) server.
  # Also sends a DATA string if `data` is given.
  #
  # Returns a
  # [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object.
  #
  # This method never raises Net::\* exceptions.
  #
  # ```ruby
  # response = http.send_request('GET', '/index.html')
  # puts response.body
  # ```
  def send_request(name, path, data=T.unsafe(nil), header=T.unsafe(nil)); end

  # **WARNING** This method opens a serious security hole. Never use this method
  # in production code.
  #
  # Sets an output stream for debugging.
  #
  # ```
  # http = Net::HTTP.new(hostname)
  # http.set_debug_output $stderr
  # http.start { .... }
  # ```
  def set_debug_output(output); end

  # Sets the SSL timeout seconds.
  def ssl_timeout(); end

  # Sets the SSL timeout seconds.
  def ssl_timeout=(ssl_timeout); end

  # Sets the SSL version. See
  # [`OpenSSL::SSL::SSLContext#ssl_version=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-ssl_version-3D)
  def ssl_version(); end

  # Sets the SSL version. See
  # [`OpenSSL::SSL::SSLContext#ssl_version=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-ssl_version-3D)
  def ssl_version=(ssl_version); end

  # Opens a TCP connection and
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) session.
  #
  # When this method is called with a block, it passes the
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object to
  # the block, and closes the TCP connection and
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) session after
  # the block has been executed.
  #
  # When called with a block, it returns the return value of the block;
  # otherwise, it returns self.
  def start(); end

  # Returns true if the
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) session has been
  # started.
  #
  # Also aliased as:
  # [`active?`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-i-active-3F)
  def started?(); end

  # Sends a TRACE request to the `path` and gets a response, as an HTTPResponse
  # object.
  def trace(path, initheader=T.unsafe(nil)); end

  def transport_request(req); end

  # Sends a UNLOCK request to the `path` and gets a response, as an HTTPResponse
  # object.
  def unlock(path, body, initheader=T.unsafe(nil)); end

  # Turn on/off SSL. This flag must be set before starting session. If you
  # change use\_ssl value after session started, a
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object
  # raises [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html).
  def use_ssl=(flag); end

  # Returns true if SSL/TLS is being used with
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html).
  def use_ssl?(); end

  # Sets the verify callback for the server certification verification.
  def verify_callback(); end

  # Sets the verify callback for the server certification verification.
  def verify_callback=(verify_callback); end

  # Sets the maximum depth for the certificate chain verification.
  def verify_depth(); end

  # Sets the maximum depth for the certificate chain verification.
  def verify_depth=(verify_depth); end

  # Sets the flags for server the certification verification at beginning of
  # SSL/TLS session.
  #
  # OpenSSL::SSL::VERIFY\_NONE or OpenSSL::SSL::VERIFY\_PEER are acceptable.
  def verify_mode(); end

  # Sets the flags for server the certification verification at beginning of
  # SSL/TLS session.
  #
  # OpenSSL::SSL::VERIFY\_NONE or OpenSSL::SSL::VERIFY\_PEER are acceptable.
  def verify_mode=(verify_mode); end

  # Creates an [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) proxy
  # class which behaves like
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html), but
  # performs all access via the specified proxy.
  #
  # This class is obsolete. You may pass these same parameters directly to
  # [`Net::HTTP.new`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-c-new).
  # See
  # [`Net::HTTP.new`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-c-new)
  # for details of the arguments.
  def self.Proxy(p_addr=T.unsafe(nil), p_port=T.unsafe(nil), p_user=T.unsafe(nil), p_pass=T.unsafe(nil)); end

  # The default port to use for
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) requests;
  # defaults to 80.
  def self.default_port(); end

  # Sends a GET request to the target and returns the
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) response as a
  # string. The target can either be specified as (`uri`), or as (`host`,
  # `path`, `port` = 80); so:
  #
  # ```ruby
  # print Net::HTTP.get(URI('http://www.example.com/index.html'))
  # ```
  #
  # or:
  #
  # ```ruby
  # print Net::HTTP.get('www.example.com', '/index.html')
  # ```
  def self.get(uri_or_host, path=T.unsafe(nil), port=T.unsafe(nil)); end

  # Gets the body text from the target and outputs it to $stdout. The target can
  # either be specified as (`uri`), or as (`host`, `path`, `port` = 80); so:
  #
  # ```ruby
  # Net::HTTP.get_print URI('http://www.example.com/index.html')
  # ```
  #
  # or:
  #
  # ```ruby
  # Net::HTTP.get_print 'www.example.com', '/index.html'
  # ```
  def self.get_print(uri_or_host, path=T.unsafe(nil), port=T.unsafe(nil)); end

  # Sends a GET request to the target and returns the
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) response as a
  # [`Net::HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object. The target can either be specified as (`uri`), or as (`host`,
  # `path`, `port` = 80); so:
  #
  # ```ruby
  # res = Net::HTTP.get_response(URI('http://www.example.com/index.html'))
  # print res.body
  # ```
  #
  # or:
  #
  # ```ruby
  # res = Net::HTTP.get_response('www.example.com', '/index.html')
  # print res.body
  # ```
  def self.get_response(uri_or_host, path=T.unsafe(nil), port=T.unsafe(nil), &block); end

  # The default port to use for
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) requests;
  # defaults to 80.
  def self.http_default_port(); end

  # The default port to use for HTTPS requests; defaults to 443.
  def self.https_default_port(); end

  def self.is_version_1_1?(); end

  # Alias for:
  # [`version_1_2?`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-c-version_1_2-3F)
  def self.is_version_1_2?(); end

  # Creates a new
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object for
  # the specified server address, without opening the TCP connection or
  # initializing the [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html)
  # session. The `address` should be a DNS hostname or IP address.
  def self.new(address, port=T.unsafe(nil), p_addr=T.unsafe(nil), p_port=T.unsafe(nil), p_user=T.unsafe(nil), p_pass=T.unsafe(nil)); end

  # Alias for:
  # [`new`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-c-new)
  def self.newobj(*_); end

  # Posts data to the specified
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) object.
  #
  # Example:
  #
  # ```ruby
  # require 'net/http'
  # require 'uri'
  #
  # Net::HTTP.post URI('http://www.example.com/api/search'),
  #                { "q" => "ruby", "max" => "50" }.to_json,
  #                "Content-Type" => "application/json"
  # ```
  def self.post(url, data, header=T.unsafe(nil)); end

  # Posts HTML form data to the specified
  # [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) object. The form data
  # must be provided as a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) mapping from
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Example:
  #
  # ```ruby
  # { "cmd" => "search", "q" => "ruby", "max" => "50" }
  # ```
  #
  # This method also does Basic Authentication iff `url`.user exists. But
  # userinfo for authentication is deprecated (RFC3986). So this feature will be
  # removed.
  #
  # Example:
  #
  # ```ruby
  # require 'net/http'
  # require 'uri'
  #
  # Net::HTTP.post_form URI('http://www.example.com/search.cgi'),
  #                     { "q" => "ruby", "max" => "50" }
  # ```
  def self.post_form(url, params); end

  # Address of proxy host. If
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) does not
  # use a proxy, nil.
  def self.proxy_address(); end

  # returns true if self is a class which was created by HTTP::Proxy.
  def self.proxy_class?(); end

  # User password for accessing proxy. If
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) does not
  # use a proxy, nil.
  def self.proxy_pass(); end

  # Port number of proxy host. If
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) does not
  # use a proxy, nil.
  def self.proxy_port(); end

  # User name for accessing proxy. If
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) does not
  # use a proxy, nil.
  def self.proxy_user(); end

  def self.socket_type(); end

  # Creates a new
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object,
  # then additionally opens the TCP connection and
  # [`HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) session.
  #
  # Arguments are the following:
  # *address*
  # :   hostname or IP address of the server
  # *port*
  # :   port of the server
  # *p\_addr*
  # :   address of proxy
  # *p\_port*
  # :   port of proxy
  # *p\_user*
  # :   user of proxy
  # *p\_pass*
  # :   pass of proxy
  # *opt*
  # :   optional hash
  #
  #
  # *opt* sets following values by its accessor. The keys are ipaddr,
  # [`ca_file`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-ca_file),
  # [`ca_path`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-ca_path),
  # cert,
  # [`cert_store`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-cert_store),
  # ciphers,
  # [`close_on_empty_response`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-close_on_empty_response),
  # key,
  # [`open_timeout`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-open_timeout),
  # [`read_timeout`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-read_timeout),
  # [`write_timeout`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-write_timeout),
  # [`ssl_timeout`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-ssl_timeout),
  # [`ssl_version`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-ssl_version),
  # use\_ssl,
  # [`verify_callback`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-verify_callback),
  # [`verify_depth`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-verify_depth)
  # and verify\_mode. If you set :use\_ssl as true, you can use https and
  # default value of
  # [`verify_mode`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#attribute-i-verify_mode)
  # is set as OpenSSL::SSL::VERIFY\_PEER.
  #
  # If the optional block is given, the newly created
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object is
  # passed to it and closed when the block finishes. In this case, the return
  # value of this method is the return value of the block. If no block is given,
  # the return value of this method is the newly created
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) object
  # itself, and the caller is responsible for closing it upon completion using
  # the finish() method.
  def self.start(address, *arg, &block); end

  def self.version_1_1?(); end

  # Turns on net/http 1.2 (Ruby 1.8) features. Defaults to ON in Ruby 1.8 or
  # later.
  def self.version_1_2(); end

  # Returns true if net/http is in version 1.2 mode. Defaults to true.
  #
  # Also aliased as:
  # [`is_version_1_2?`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html#method-c-is_version_1_2-3F)
  def self.version_1_2?(); end
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Copy < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)

end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods. See
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) for usage
# examples.
class Net::HTTP::Delete < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)

end

class Net::HTTP::DigestAuth
  include ::MonitorMixin
  VERSION = ::T.unsafe(nil)

  def auth_header(uri, www_authenticate, method, iis=T.unsafe(nil)); end

  def initialize(ignored=T.unsafe(nil)); end

  def make_cnonce(); end

  def next_nonce(); end
end

class Net::HTTP::DigestAuth::Error < RuntimeError
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods. See
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) for usage
# examples.
class Net::HTTP::Get < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods. See
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) for usage
# examples.
class Net::HTTP::Head < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Lock < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Mkcol < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Move < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Options < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Patch < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods. See
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) for usage
# examples.
class Net::HTTP::Post < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Propfind < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Proppatch < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

module Net::HTTP::ProxyDelta
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods. See
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) for usage
# examples.
class Net::HTTP::Put < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)

end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Trace < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)
end

# See
# [`Net::HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# for attributes and methods.
class Net::HTTP::Unlock < Net::HTTPRequest
  METHOD = ::T.unsafe(nil)
  REQUEST_HAS_BODY = ::T.unsafe(nil)
  RESPONSE_HAS_BODY = ::T.unsafe(nil)

end

class Net::HTTPAccepted < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPBadGateway < Net::HTTPServerError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPBadRequest < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPBadResponse < StandardError
end

class Net::HTTPClientError < Net::HTTPResponse
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPConflict < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPContinue < Net::HTTPInformation
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPCreated < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPError < Net::ProtocolError
  include ::Net::HTTPExceptions
end

# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html) exception
# class. You cannot use
# [`Net::HTTPExceptions`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPExceptions.html)
# directly; instead, you must use its subclasses.
module Net::HTTPExceptions
  def data(); end

  def initialize(msg, res); end

  def response(); end
end

class Net::HTTPExpectationFailed < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPFailedDependency < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPFatalError < Net::ProtoFatalError
  include ::Net::HTTPExceptions
end

class Net::HTTPForbidden < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPFound < Net::HTTPRedirection
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPGatewayTimeout < Net::HTTPServerError
  HAS_BODY = ::T.unsafe(nil)
end

Net::HTTPGatewayTimeOut = Net::HTTPGatewayTimeout

# [`HTTPGenericRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPGenericRequest.html)
# is the parent of the
# [`Net::HTTPRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPRequest.html)
# class. Do not use this directly; use a subclass of
# [`Net::HTTPRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPRequest.html).
#
# Mixes in the
# [`Net::HTTPHeader`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html)
# module to provide easier access to HTTP headers.
class Net::HTTPGenericRequest
  include ::Net::HTTPHeader
  def []=(key, val); end

  def body(); end

  def body=(str); end

  def body_exist?(); end

  def body_stream(); end

  def body_stream=(input); end

  # Automatically set to false if the user sets the Accept-Encoding header. This
  # indicates they wish to handle Content-encoding in responses themselves.
  def decode_content(); end

  def exec(sock, ver, path); end

  def initialize(m, reqbody, resbody, uri_or_path, initheader=T.unsafe(nil)); end

  def inspect(); end

  def method(); end

  def path(); end

  def request_body_permitted?(); end

  def response_body_permitted?(); end

  def set_body_internal(str); end

  def update_uri(addr, port, ssl); end

  def uri(); end
end

class Net::HTTPGenericRequest::Chunker
  def finish(); end

  def initialize(sock); end

  def write(buf); end
end

class Net::HTTPGone < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

# The [`HTTPHeader`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html)
# module defines methods for reading and writing HTTP headers.
#
# It is used as a mixin by other classes, to provide hash-like access to HTTP
# header values. Unlike raw hash access,
# [`HTTPHeader`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html)
# provides access via case-insensitive keys. It also provides methods for
# accessing commonly-used HTTP header values in more convenient formats.
module Net::HTTPHeader
  # Returns the header field corresponding to the case-insensitive key. For
  # example, a key of "Content-Type" might return "text/html"
  def [](key); end

  # Sets the header field corresponding to the case-insensitive key.
  def []=(key, val); end

  # Ruby 1.8.3
  # :   Adds a value to a named header field, instead of replacing its value.
  #     Second argument `val` must be a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). See also
  #     [`[]=`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-5B-5D-3D),
  #     [`[]`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-5B-5D)
  #     and
  #     [`get_fields`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-get_fields).
  #
  # ```ruby
  # request.add_field 'X-My-Header', 'a'
  # p request['X-My-Header']              #=> "a"
  # p request.get_fields('X-My-Header')   #=> ["a"]
  # request.add_field 'X-My-Header', 'b'
  # p request['X-My-Header']              #=> "a, b"
  # p request.get_fields('X-My-Header')   #=> ["a", "b"]
  # request.add_field 'X-My-Header', 'c'
  # p request['X-My-Header']              #=> "a, b, c"
  # p request.get_fields('X-My-Header')   #=> ["a", "b", "c"]
  # ```
  def add_field(key, val); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the Authorization:
  # header for "Basic" authorization.
  def basic_auth(account, password); end

  # Alias for:
  # [`each_capitalized`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-each_capitalized)
  def canonical_each(); end

  # Returns "true" if the "transfer-encoding" header is present and set to
  # "chunked". This is an HTTP/1.1 feature, allowing the content to be sent in
  # "chunks" without at the outset stating the entire content length.
  def chunked?(); end

  def connection_close?(); end

  def connection_keep_alive?(); end

  # Returns an [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
  # object which represents the HTTP Content-Length: header field, or `nil` if
  # that field was not provided.
  def content_length(); end

  def content_length=(len); end

  # Returns a [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) object
  # which represents the value of the Content-Range: header field. For a partial
  # entity body, this indicates where this fragment fits inside the full entity
  # body, as range of byte offsets.
  def content_range(); end

  # Returns a content type string such as "text/html". This method returns nil
  # if Content-Type: header field does not exist.
  def content_type(); end

  # Alias for:
  # [`set_content_type`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-set_content_type)
  def content_type=(type, params=T.unsafe(nil)); end

  # Removes a header field, specified by case-insensitive key.
  def delete(key); end

  # Alias for:
  # [`each_header`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-each_header)
  def each(); end

  # As for
  # [`each_header`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-each_header),
  # except the keys are provided in capitalized form.
  #
  # Note that header names are capitalized systematically; capitalization may
  # not match that used by the remote HTTP server in its response.
  #
  # Returns an enumerator if no block is given.
  #
  # Also aliased as:
  # [`canonical_each`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-canonical_each)
  def each_capitalized(); end

  # Iterates through the header names in the header, passing capitalized header
  # names to the code block.
  #
  # Note that header names are capitalized systematically; capitalization may
  # not match that used by the remote HTTP server in its response.
  #
  # Returns an enumerator if no block is given.
  def each_capitalized_name(); end

  # Iterates through the header names and values, passing in the name and value
  # to the code block supplied.
  #
  # Returns an enumerator if no block is given.
  #
  # Example:
  #
  # ```ruby
  # response.header.each_header {|key,value| puts "#{key} = #{value}" }
  # ```
  #
  #
  # Also aliased as:
  # [`each`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-each)
  def each_header(); end

  # Alias for:
  # [`each_name`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-each_name)
  def each_key(&block); end

  # Iterates through the header names in the header, passing each header name to
  # the code block.
  #
  # Returns an enumerator if no block is given.
  #
  # Also aliased as:
  # [`each_key`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-each_key)
  def each_name(&block); end

  # Iterates through header values, passing each value to the code block.
  #
  # Returns an enumerator if no block is given.
  def each_value(); end

  # Returns the header field corresponding to the case-insensitive key. Returns
  # the default value `args`, or the result of the block, or raises an
  # [`IndexError`](https://docs.ruby-lang.org/en/2.7.0/IndexError.html) if
  # there's no header field named `key` See
  # [`Hash#fetch`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-fetch)
  def fetch(key, *args, &block); end

  # Alias for:
  # [`set_form_data`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-set_form_data)
  def form_data=(params, sep=T.unsafe(nil)); end

  # Ruby 1.8.3
  # :   Returns an array of header field strings corresponding to the
  #     case-insensitive `key`. This method allows you to get duplicated header
  #     fields without any processing. See also
  #     [`[]`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-5B-5D).
  #
  # ```
  # p response.get_fields('Set-Cookie')
  #   #=> ["session=al98axx; expires=Fri, 31-Dec-1999 23:58:23",
  #        "query=rubyscript; expires=Fri, 31-Dec-1999 23:58:23"]
  # p response['Set-Cookie']
  #   #=> "session=al98axx; expires=Fri, 31-Dec-1999 23:58:23, query=rubyscript; expires=Fri, 31-Dec-1999 23:58:23"
  # ```
  def get_fields(key); end

  def initialize_http_header(initheader); end

  # true if `key` header exists.
  def key?(key); end

  def length(); end

  # Returns a content type string such as "text". This method returns nil if
  # Content-Type: header field does not exist.
  def main_type(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) Proxy-Authorization:
  # header for "Basic" authorization.
  def proxy_basic_auth(account, password); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) objects which
  # represent the Range: HTTP header field, or `nil` if there is no such header.
  def range(); end

  # Alias for:
  # [`set_range`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-set_range)
  def range=(r, e=T.unsafe(nil)); end

  # The length of the range represented in Content-Range: header.
  def range_length(); end

  # Sets the content type in an HTTP header. The `type` should be a full HTTP
  # content type, e.g. "text/html". The `params` are an optional
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of parameters to add
  # after the content type, e.g. {'charset' => 'iso-8859-1'}
  #
  # Also aliased as:
  # [`content_type=`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-content_type-3D)
  def set_content_type(type, params=T.unsafe(nil)); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) an HTML form data set.
  # `params` is the form data set; it is an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Arrays or a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) +enctype is the type
  # to encode the form data set. It is application/x-www-form-urlencoded or
  # multipart/form-data. `formopt` is an optional hash to specify the detail.
  #
  # boundary
  # :   the boundary of the multipart message
  # charset
  # :   the charset of the message. All names and the values of non-file fields
  #     are encoded as the charset.
  #
  #
  # Each item of params is an array and contains following items:
  # `name`
  # :   the name of the field
  # `value`
  # :   the value of the field, it should be a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or a
  #     [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
  # `opt`
  # :   an optional hash to specify additional information
  #
  #
  # Each item is a file field or a normal field. If `value` is a
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) object or the `opt`
  # have a filename key, the item is treated as a file field.
  #
  # If Transfer-Encoding is set as chunked, this send the request in chunked
  # encoding. Because chunked encoding is HTTP/1.1 feature, you must confirm the
  # server to support HTTP/1.1 before sending it.
  #
  # Example:
  #
  # ```ruby
  # http.set_form([["q", "ruby"], ["lang", "en"]])
  # ```
  #
  # See also RFC 2388, RFC 2616, HTML 4.01, and HTML5
  def set_form(params, enctype=T.unsafe(nil), formopt=T.unsafe(nil)); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) header fields and a
  # body from HTML form data. `params` should be an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Arrays or a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) containing HTML form
  # data. Optional argument `sep` means data record separator.
  #
  # Values are URL encoded as necessary and the content-type is set to
  # application/x-www-form-urlencoded
  #
  # Example:
  #
  # ```ruby
  # http.form_data = {"q" => "ruby", "lang" => "en"}
  # http.form_data = {"q" => ["ruby", "perl"], "lang" => "en"}
  # http.set_form_data({"q" => "ruby", "lang" => "en"}, ';')
  # ```
  #
  #
  # Also aliased as:
  # [`form_data=`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-form_data-3D)
  def set_form_data(params, sep=T.unsafe(nil)); end

  # Sets the HTTP Range: header. Accepts either a
  # [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) object as a single
  # argument, or a beginning index and a length from that index. Example:
  #
  # ```ruby
  # req.range = (0..1023)
  # req.set_range 0, 1023
  # ```
  #
  #
  # Also aliased as:
  # [`range=`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-range-3D)
  def set_range(r, e=T.unsafe(nil)); end

  def size(); end

  # Returns a content type string such as "html". This method returns nil if
  # Content-Type: header field does not exist or sub-type is not given (e.g.
  # "Content-Type: text").
  def sub_type(); end

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) consisting
  # of header names and array of values. e.g. {"cache-control" => ["private"],
  #
  # ```
  # "content-type" => ["text/html"],
  # "date" => ["Wed, 22 Jun 2005 22:11:50 GMT"]}
  # ```
  def to_hash(); end

  # Any parameters specified for the content type, returned as a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html). For example, a
  # header of Content-Type: text/html; charset=EUC-JP would result in
  # [`type_params`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPHeader.html#method-i-type_params)
  # returning {'charset' => 'EUC-JP'}
  def type_params(); end
end

class Net::HTTPHeaderSyntaxError < StandardError
end

class Net::HTTPIMUsed < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPInformation < Net::HTTPResponse
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPInsufficientStorage < Net::HTTPServerError
  HAS_BODY = ::T.unsafe(nil)
end

# 444 No Response - Nginx 449 Retry With - Microsoft 450 Blocked by Windows
# Parental Controls - Microsoft 499 Client Closed Request - Nginx
class Net::HTTPInternalServerError < Net::HTTPServerError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPLengthRequired < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPLocked < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPMethodNotAllowed < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPMovedPermanently < Net::HTTPRedirection
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPMultiStatus < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPMultipleChoices < Net::HTTPRedirection
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPNetworkAuthenticationRequired < Net::HTTPServerError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPNoContent < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPNonAuthoritativeInformation < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPNotAcceptable < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPNotFound < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPNotImplemented < Net::HTTPServerError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPNotModified < Net::HTTPRedirection
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPOK < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPPartialContent < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPPaymentRequired < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPPermanentRedirect < Net::HTTPRedirection
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPPreconditionFailed < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPPreconditionRequired < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPProxyAuthenticationRequired < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPRedirection < Net::HTTPResponse
  HAS_BODY = ::T.unsafe(nil)
end

# HTTP request class. This class wraps together the request header and the
# request path. You cannot use this class directly. Instead, you should use one
# of its subclasses:
# [`Net::HTTP::Get`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Get.html),
# [`Net::HTTP::Post`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Post.html),
# [`Net::HTTP::Head`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP/Head.html).
class Net::HTTPRequest < Net::HTTPGenericRequest
  def initialize(path, initheader=T.unsafe(nil)); end
end

class Net::HTTPRequestEntityTooLarge < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPRequestHeaderFieldsTooLarge < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPRequestTimeout < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

Net::HTTPRequestTimeOut = Net::HTTPRequestTimeout

class Net::HTTPRequestURITooLong < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPRequestedRangeNotSatisfiable < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPResetContent < Net::HTTPSuccess
  HAS_BODY = ::T.unsafe(nil)
end

# HTTP response class.
#
# This class wraps together the response header and the response body (the
# entity requested).
#
# It mixes in the HTTPHeader module, which provides access to response header
# values both via hash-like methods and via individual readers.
#
# Note that each possible HTTP response code defines its own
# [`HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
# subclass. All classes are defined under the
# [`Net`](https://docs.ruby-lang.org/en/2.7.0/Net.html) module. Indentation
# indicates inheritance. For a list of the classes see
# [`Net::HTTP`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTP.html).
#
# Correspondence `HTTP code => class` is stored in
# [`CODE_TO_OBJ`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html#CODE_TO_OBJ)
# constant:
#
# ```ruby
# Net::HTTPResponse::CODE_TO_OBJ['404'] #=> Net::HTTPNotFound
# ```
class Net::HTTPResponse
  include ::Net::HTTPHeader
  CODE_CLASS_TO_OBJ = ::T.unsafe(nil)
  CODE_TO_OBJ = ::T.unsafe(nil)

  # Returns the full entity body.
  #
  # Calling this method a second or subsequent time will return the string
  # already read.
  #
  # ```ruby
  # http.request_get('/index.html') {|res|
  #   puts res.body
  # }
  #
  # http.request_get('/index.html') {|res|
  #   p res.body.object_id   # 538149362
  #   p res.body.object_id   # 538149362
  # }
  # ```
  #
  #
  # Also aliased as:
  # [`entity`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html#method-i-entity)
  def body(); end

  # Because it may be necessary to modify the body, Eg, decompression this
  # method facilitates that.
  def body=(value); end

  # The HTTP result code string. For example, '302'. You can also determine the
  # response type by examining which response subclass the response object is an
  # instance of.
  def code(); end

  def code_type(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) to true automatically
  # when the request did not contain an Accept-Encoding header from the user.
  def decode_content(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) to true automatically
  # when the request did not contain an Accept-Encoding header from the user.
  def decode_content=(decode_content); end

  # Alias for:
  # [`body`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html#method-i-body)
  def entity(); end

  def error!(); end

  def error_type(); end

  def header(); end

  # The HTTP version supported by the server.
  def http_version(); end

  def initialize(httpv, code, msg); end

  def inspect(); end

  # The HTTP result message sent by the server. For example, 'Not Found'.
  def message(); end

  # The HTTP result message sent by the server. For example, 'Not Found'.
  def msg(); end

  # Gets the entity body returned by the remote HTTP server.
  #
  # If a block is given, the body is passed to the block, and the body is
  # provided in fragments, as it is read in from the socket.
  #
  # If `dest` argument is given, response is read into that variable, with
  # `dest#<<` method (it could be
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html), or any other object
  # responding to `<<`).
  #
  # Calling this method a second or subsequent time for the same
  # [`HTTPResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/HTTPResponse.html)
  # object will return the value already read.
  #
  # ```ruby
  # http.request_get('/index.html') {|res|
  #   puts res.read_body
  # }
  #
  # http.request_get('/index.html') {|res|
  #   p res.read_body.object_id   # 538149362
  #   p res.read_body.object_id   # 538149362
  # }
  #
  # # using iterator
  # http.request_get('/index.html') {|res|
  #   res.read_body do |segment|
  #     print segment
  #   end
  # }
  # ```
  def read_body(dest=T.unsafe(nil), &block); end

  def read_header(); end

  def reading_body(sock, reqmethodallowbody); end

  def response(); end

  # The [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) used to fetch this
  # response. The response [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
  # is only available if a [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
  # was used to create the request.
  def uri(); end

  # The [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) used to fetch this
  # response. The response [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
  # is only available if a [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
  # was used to create the request.
  def uri=(uri); end

  # Raises an HTTP error if the response is not 2xx (success).
  def value(); end

  # true if the response has a body.
  def self.body_permitted?(); end

  def self.exception_type(); end

  def self.read_new(sock); end
end

class Net::HTTPResponse::Inflater
  def finish(); end

  def inflate_adapter(dest); end

  def initialize(socket); end

  def read(clen, dest, ignore_eof=T.unsafe(nil)); end

  def read_all(dest); end
end

class Net::HTTPRetriableError < Net::ProtoRetriableError
  include ::Net::HTTPExceptions
end

class Net::HTTPSeeOther < Net::HTTPRedirection
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPServerError < Net::HTTPResponse
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPServerException < Net::ProtoServerError
  include ::Net::HTTPExceptions
end
# for compatibility
Net::HTTPClientException = Net::HTTPServerException

class Net::HTTPServiceUnavailable < Net::HTTPServerError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPSuccess < Net::HTTPResponse
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPSwitchProtocol < Net::HTTPInformation
  HAS_BODY = ::T.unsafe(nil)
end

# 306 Switch Proxy - no longer unused
class Net::HTTPTemporaryRedirect < Net::HTTPRedirection
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPTooManyRequests < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPUnauthorized < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPUnavailableForLegalReasons < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

# https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
class Net::HTTPUnknownResponse < Net::HTTPResponse
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPUnprocessableEntity < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPUnsupportedMediaType < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

# 425 Unordered Collection - existed only in draft
class Net::HTTPUpgradeRequired < Net::HTTPClientError
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPUseProxy < Net::HTTPRedirection
  HAS_BODY = ::T.unsafe(nil)
end

class Net::HTTPVersionNotSupported < Net::HTTPServerError
  HAS_BODY = ::T.unsafe(nil)
end

# [`Net::IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) implements
# Internet Message Access Protocol
# ([`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html)) client
# functionality. The protocol is described in [IMAP].
#
# ## [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) Overview
#
# An [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) client connects
# to a server, and then authenticates itself using either
# [`authenticate()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-authenticate)
# or
# [`login()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-login).
# Having authenticated itself, there is a range of commands available to it.
# Most work with mailboxes, which may be arranged in an hierarchical namespace,
# and each of which contains zero or more messages. How this is implemented on
# the server is implementation-dependent; on a UNIX server, it will frequently
# be implemented as files in mailbox format within a hierarchy of directories.
#
# To work on the messages within a mailbox, the client must first select that
# mailbox, using either
# [`select()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-select)
# or (for read-only access)
# [`examine()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-examine).
# Once the client has successfully selected a mailbox, they enter *selected*
# state, and that mailbox becomes the *current* mailbox, on which mail-item
# related commands implicitly operate.
#
# Messages have two sorts of identifiers: message sequence numbers and UIDs.
#
# Message sequence numbers number messages within a mailbox from 1 up to the
# number of items in the mailbox. If a new message arrives during a session, it
# receives a sequence number equal to the new size of the mailbox. If messages
# are expunged from the mailbox, remaining messages have their sequence numbers
# "shuffled down" to fill the gaps.
#
# UIDs, on the other hand, are permanently guaranteed not to identify another
# message within the same mailbox, even if the existing message is deleted. UIDs
# are required to be assigned in ascending (but not necessarily sequential)
# order within a mailbox; this means that if a non-IMAP client rearranges the
# order of mailitems within a mailbox, the UIDs have to be reassigned. An
# [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) client thus cannot
# rearrange message orders.
#
# ## Examples of Usage
#
# ### List sender and subject of all recent messages in the default mailbox
#
# ```ruby
# imap = Net::IMAP.new('mail.example.com')
# imap.authenticate('LOGIN', 'joe_user', 'joes_password')
# imap.examine('INBOX')
# imap.search(["RECENT"]).each do |message_id|
#   envelope = imap.fetch(message_id, "ENVELOPE")[0].attr["ENVELOPE"]
#   puts "#{envelope.from[0].name}: \t#{envelope.subject}"
# end
# ```
#
# ### Move all messages from April 2003 from "Mail/sent-mail" to "Mail/sent-apr03"
#
# ```ruby
# imap = Net::IMAP.new('mail.example.com')
# imap.authenticate('LOGIN', 'joe_user', 'joes_password')
# imap.select('Mail/sent-mail')
# if not imap.list('Mail/', 'sent-apr03')
#   imap.create('Mail/sent-apr03')
# end
# imap.search(["BEFORE", "30-Apr-2003", "SINCE", "1-Apr-2003"]).each do |message_id|
#   imap.copy(message_id, "Mail/sent-apr03")
#   imap.store(message_id, "+FLAGS", [:Deleted])
# end
# imap.expunge
# ```
#
# ## [`Thread`](https://docs.ruby-lang.org/en/2.7.0/Thread.html) Safety
#
# [`Net::IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) supports
# concurrent threads. For example,
#
# ```ruby
# imap = Net::IMAP.new("imap.foo.net", "imap2")
# imap.authenticate("cram-md5", "bar", "password")
# imap.select("inbox")
# fetch_thread = Thread.start { imap.fetch(1..-1, "UID") }
# search_result = imap.search(["BODY", "hello"])
# fetch_result = fetch_thread.value
# imap.disconnect
# ```
#
# This script invokes the FETCH command and the SEARCH command concurrently.
#
# ## Errors
#
# An [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) server can send
# three different types of responses to indicate failure:
#
# NO
# :   the attempted command could not be successfully completed. For instance,
#     the username/password used for logging in are incorrect; the selected
#     mailbox does not exist; etc.
#
# BAD
# :   the request from the client does not follow the server's understanding of
#     the [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) protocol.
#     This includes attempting commands from the wrong client state; for
#     instance, attempting to perform a SEARCH command without having SELECTed a
#     current mailbox. It can also signal an internal server failure (such as a
#     disk crash) has occurred.
#
# BYE
# :   the server is saying goodbye. This can be part of a normal logout
#     sequence, and can be used as part of a login sequence to indicate that the
#     server is (for some reason) unwilling to accept your connection. As a
#     response to any other command, it indicates either that the server is
#     shutting down, or that the server is timing out the client connection due
#     to inactivity.
#
#
# These three error response are represented by the errors
# [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html),
# [`Net::IMAP::BadResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BadResponseError.html),
# and
# [`Net::IMAP::ByeResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/ByeResponseError.html),
# all of which are subclasses of
# [`Net::IMAP::ResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/ResponseError.html).
# Essentially, all methods that involve sending a request to the server can
# generate one of these errors. Only the most pertinent instances have been
# documented below.
#
# Because the [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) class
# uses Sockets for communication, its methods are also susceptible to the
# various errors that can occur when working with sockets. These are generally
# represented as [`Errno`](https://docs.ruby-lang.org/en/2.7.0/Errno.html)
# errors. For instance, any method that involves sending a request to the server
# and/or receiving a response from it could raise an Errno::EPIPE error if the
# network connection unexpectedly goes down. See the socket(7), ip(7), tcp(7),
# socket(2), connect(2), and associated man pages.
#
# Finally, a
# [`Net::IMAP::DataFormatError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/DataFormatError.html)
# is thrown if low-level data is found to be in an incorrect format (for
# instance, when converting between UTF-8 and UTF-16), and
# [`Net::IMAP::ResponseParseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/ResponseParseError.html)
# is thrown if a server response is non-parseable.
#
# ## References
#
#     1.  Crispin, "INTERNET MESSAGE ACCESS PROTOCOL - VERSION 4rev1",
#
#     RFC 2060, December 1996. (Note: since obsoleted by RFC 3501)
#
# [LANGUAGE-TAGS]
# :   Alvestrand, H., "Tags for the Identification of Languages", RFC 1766,
#     March 1995.
#
# [MD5]
# :   Myers, J., and M. Rose, "The Content-MD5 Header Field", RFC 1864, October
#     1995.
#
# [MIME-IMB]
# :   Freed, N., and N. Borenstein, "MIME (Multipurpose Internet Mail
#     Extensions) Part One: Format of Internet Message Bodies", RFC 2045,
#     November 1996.
#
# [RFC-822]
# :   Crocker, D., "Standard for the Format of ARPA Internet Text Messages", STD
#     11, RFC 822, University of Delaware, August 1982.
#
# [RFC-2087]
# :   Myers, J., "IMAP4 QUOTA extension", RFC 2087, January 1997.
#
# [RFC-2086]
# :   Myers, J., "IMAP4 [`ACL`](https://docs.ruby-lang.org/en/2.7.0/ACL.html)
#     extension", RFC 2086, January 1997.
#
# [RFC-2195]
# :   Klensin, J., Catoe, R., and Krumviede, P., "IMAP/POP AUTHorize Extension
#     for Simple Challenge/Response", RFC 2195, September 1997.
#
# [SORT-THREAD-EXT]
# :   Crispin, M., "INTERNET MESSAGE ACCESS PROTOCOL - SORT and THREAD
#     Extensions", draft-ietf-imapext-sort, May 2003.
#
# [OSSL]
# :   http://www.openssl.org
#
# [RSSL]
# :   http://savannah.gnu.org/projects/rubypki
#
# [UTF7]
# :   Goldsmith, D. and Davis, M., "UTF-7: A Mail-Safe Transformation Format of
#     Unicode", RFC 2152, May 1997.
class Net::IMAP < Net::Protocol
  include ::OpenSSL::SSL
  include ::OpenSSL
  include ::MonitorMixin
  # Flag indicating a message has been answered.
  ANSWERED = ::T.unsafe(nil)
  CRLF = ::T.unsafe(nil)
  DATE_MONTH = ::T.unsafe(nil)
  # Flag indicating a message has been marked for deletion. This will occur when
  # the mailbox is closed or expunged.
  DELETED = ::T.unsafe(nil)
  # Flag indicating a message is only a draft or work-in-progress version.
  DRAFT = ::T.unsafe(nil)
  # Flag indicating a message has been flagged for special or urgent attention.
  FLAGGED = ::T.unsafe(nil)
  # Flag indicating that a mailbox has been marked "interesting" by the server;
  # this commonly indicates that the mailbox contains new messages.
  MARKED = ::T.unsafe(nil)
  # Flag indicating that a mailbox context name cannot contain children.
  NOINFERIORS = ::T.unsafe(nil)
  # Flag indicating that a mailbox is not selected.
  NOSELECT = ::T.unsafe(nil)
  PORT = ::T.unsafe(nil)
  # Flag indicating that the message is "recent," meaning that this session is
  # the first session in which the client has been notified of this message.
  RECENT = ::T.unsafe(nil)
  # Flag indicating a message has been seen.
  SEEN = ::T.unsafe(nil)
  SSL_PORT = ::T.unsafe(nil)
  # Flag indicating that the mailbox does not contains new messages.
  UNMARKED = ::T.unsafe(nil)

  # Adds a response handler. For example, to detect when the server sends a new
  # EXISTS response (which normally indicates new messages being added to the
  # mailbox), add the following handler after selecting the mailbox:
  #
  # ```ruby
  # imap.add_response_handler { |resp|
  #   if resp.kind_of?(Net::IMAP::UntaggedResponse) and resp.name == "EXISTS"
  #     puts "Mailbox now has #{resp.data} messages"
  #   end
  # }
  # ```
  def add_response_handler(handler=T.unsafe(nil)); end

  # Sends a APPEND command to append the `message` to the end of the `mailbox`.
  # The optional `flags` argument is an array of flags initially passed to the
  # new message. The optional `date_time` argument specifies the creation time
  # to assign to the new message; it defaults to the current time. For example:
  #
  # ```ruby
  # imap.append("inbox", <<EOF.gsub(/\n/, "\r\n"), [:Seen], Time.now)
  # Subject: hello
  # From: shugo@ruby-lang.org
  # To: shugo@ruby-lang.org
  #
  # hello world
  # EOF
  # ```
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if the mailbox does not exist (it is not created automatically),
  # or if the flags, date\_time, or message arguments contain errors.
  def append(mailbox, message, flags=T.unsafe(nil), date_time=T.unsafe(nil)); end

  # Sends an AUTHENTICATE command to authenticate the client. The `auth_type`
  # parameter is a string that represents the authentication mechanism to be
  # used. Currently
  # [`Net::IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) supports
  # the authentication mechanisms:
  #
  # ```
  # LOGIN:: login using cleartext user and password.
  # CRAM-MD5:: login with cleartext user and encrypted password
  #            (see [RFC-2195] for a full description).  This
  #            mechanism requires that the server have the user's
  #            password stored in clear-text password.
  # ```
  #
  # For both of these mechanisms, there should be two `args`: username and
  # (cleartext) password. A server may not support one or the other of these
  # mechanisms; check
  # [`capability()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-capability)
  # for a capability of the form "AUTH=LOGIN" or "AUTH=CRAM-MD5".
  #
  # Authentication is done using the appropriate authenticator object: see
  # @@authenticators for more information on plugging in your own authenticator.
  #
  # For example:
  #
  # ```ruby
  # imap.authenticate('LOGIN', user, password)
  # ```
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if authentication fails.
  def authenticate(auth_type, *args); end

  # Sends a CAPABILITY command, and returns an array of capabilities that the
  # server supports. Each capability is a string. See [IMAP] for a list of
  # possible capabilities.
  #
  # Note that the
  # [`Net::IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) class does
  # not modify its behaviour according to the capabilities of the server; it is
  # up to the user of the class to ensure that a certain capability is supported
  # by a server before using it.
  def capability(); end

  # Sends a CHECK command to request a checkpoint of the currently selected
  # mailbox. This performs implementation-specific housekeeping; for instance,
  # reconciling the mailbox's in-memory and on-disk state.
  def check(); end

  # The thread to receive exceptions.
  def client_thread(); end

  # The thread to receive exceptions.
  def client_thread=(client_thread); end

  # Sends a CLOSE command to close the currently selected mailbox. The CLOSE
  # command permanently removes from the mailbox all messages that have the
  # Deleted flag set.
  def close(); end

  # Sends a COPY command to copy the specified message(s) to the end of the
  # specified destination `mailbox`. The `set` parameter is a number, an array
  # of numbers, or a [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html)
  # object. The number is a message sequence number.
  def copy(set, mailbox); end

  # Sends a CREATE command to create a new `mailbox`.
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if a mailbox with that name cannot be created.
  def create(mailbox); end

  # Sends a DELETE command to remove the `mailbox`.
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if a mailbox with that name cannot be deleted, either because it
  # does not exist or because the client does not have permission to delete it.
  def delete(mailbox); end

  # Disconnects from the server.
  def disconnect(); end

  # Returns true if disconnected from the server.
  def disconnected?(); end

  # Sends a EXAMINE command to select a `mailbox` so that messages in the
  # `mailbox` can be accessed. Behaves the same as
  # [`select()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-select),
  # except that the selected `mailbox` is identified as read-only.
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if the mailbox does not exist or is for some reason
  # non-examinable.
  def examine(mailbox); end

  # Sends a EXPUNGE command to permanently remove from the currently selected
  # mailbox all messages that have the Deleted flag set.
  def expunge(); end

  # Sends a FETCH command to retrieve data associated with a message in the
  # mailbox.
  #
  # The `set` parameter is a number or a range between two numbers, or an array
  # of those. The number is a message sequence number, where -1 represents a
  # '\*' for use in range notation like 100..-1 being interpreted as '100:\*'.
  # Beware that the `exclude_end?` property of a
  # [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) object is ignored,
  # and the contents of a range are independent of the order of the range
  # endpoints as per the protocol specification, so 1...5, 5..1 and 5...1 are
  # all equivalent to 1..5.
  #
  # `attr` is a list of attributes to fetch; see the documentation for
  # [`Net::IMAP::FetchData`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#FetchData)
  # for a list of valid attributes.
  #
  # The return value is an array of
  # [`Net::IMAP::FetchData`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#FetchData)
  # or nil (instead of an empty array) if there is no matching message.
  #
  # For example:
  #
  # ```ruby
  # p imap.fetch(6..8, "UID")
  # #=> [#<Net::IMAP::FetchData seqno=6, attr={"UID"=>98}>, \\
  #      #<Net::IMAP::FetchData seqno=7, attr={"UID"=>99}>, \\
  #      #<Net::IMAP::FetchData seqno=8, attr={"UID"=>100}>]
  # p imap.fetch(6, "BODY[HEADER.FIELDS (SUBJECT)]")
  # #=> [#<Net::IMAP::FetchData seqno=6, attr={"BODY[HEADER.FIELDS (SUBJECT)]"=>"Subject: test\r\n\r\n"}>]
  # data = imap.uid_fetch(98, ["RFC822.SIZE", "INTERNALDATE"])[0]
  # p data.seqno
  # #=> 6
  # p data.attr["RFC822.SIZE"]
  # #=> 611
  # p data.attr["INTERNALDATE"]
  # #=> "12-Oct-2000 22:40:59 +0900"
  # p data.attr["UID"]
  # #=> 98
  # ```
  def fetch(set, attr); end

  # Send the GETACL command along with a specified `mailbox`. If this mailbox
  # exists, an array containing objects of
  # [`Net::IMAP::MailboxACLItem`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxACLItem)
  # will be returned.
  def getacl(mailbox); end

  # Sends the GETQUOTA command along with specified `mailbox`. If this mailbox
  # exists, then an array containing a
  # [`Net::IMAP::MailboxQuota`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxQuota)
  # object is returned. This command is generally only available to server
  # admin.
  def getquota(mailbox); end

  # Sends the GETQUOTAROOT command along with the specified `mailbox`. This
  # command is generally available to both admin and user. If this mailbox
  # exists, it returns an array containing objects of type
  # [`Net::IMAP::MailboxQuotaRoot`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxQuotaRoot)
  # and
  # [`Net::IMAP::MailboxQuota`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxQuota).
  def getquotaroot(mailbox); end

  # Returns an initial greeting response from the server.
  def greeting(); end

  # Sends an IDLE command that waits for notifications of new or expunged
  # messages. Yields responses from the server during the IDLE.
  #
  # Use
  # [`idle_done()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-idle_done)
  # to leave IDLE.
  #
  # If `timeout` is given, this method returns after `timeout` seconds passed.
  # `timeout` can be used for keep-alive. For example, the following code checks
  # the connection for each 60 seconds.
  #
  # ```
  # loop do
  #   imap.idle(60) do |res|
  #     ...
  #   end
  # end
  # ```
  def idle(timeout=T.unsafe(nil), &response_handler); end

  # Leaves IDLE.
  def idle_done(); end

  def initialize(host, port_or_options=T.unsafe(nil), usessl=T.unsafe(nil), certs=T.unsafe(nil), verify=T.unsafe(nil)); end

  # Sends a LIST command, and returns a subset of names from the complete set of
  # all names available to the client. `refname` provides a context (for
  # instance, a base directory in a directory-based mailbox hierarchy).
  # `mailbox` specifies a mailbox or (via wildcards) mailboxes under that
  # context. Two wildcards may be used in `mailbox`: '\*', which matches all
  # characters **including** the hierarchy delimiter (for instance, '/' on a
  # UNIX-hosted directory-based mailbox hierarchy); and '%', which matches all
  # characters **except** the hierarchy delimiter.
  #
  # If `refname` is empty, `mailbox` is used directly to determine which
  # mailboxes to match. If `mailbox` is empty, the root name of `refname` and
  # the hierarchy delimiter are returned.
  #
  # The return value is an array of `Net::IMAP::MailboxList`. For example:
  #
  # ```ruby
  # imap.create("foo/bar")
  # imap.create("foo/baz")
  # p imap.list("", "foo/%")
  # #=> [#<Net::IMAP::MailboxList attr=[:Noselect], delim="/", name="foo/">, \\
  #      #<Net::IMAP::MailboxList attr=[:Noinferiors, :Marked], delim="/", name="foo/bar">, \\
  #      #<Net::IMAP::MailboxList attr=[:Noinferiors], delim="/", name="foo/baz">]
  # ```
  def list(refname, mailbox); end

  # Sends a LOGIN command to identify the client and carries the plaintext
  # `password` authenticating this `user`. Note that, unlike calling
  # [`authenticate()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-authenticate)
  # with an `auth_type` of "LOGIN",
  # [`login()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-login)
  # does **not** use the login authenticator.
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if authentication fails.
  def login(user, password); end

  # Sends a LOGOUT command to inform the server that the client is done with the
  # connection.
  def logout(); end

  # Sends a LSUB command, and returns a subset of names from the set of names
  # that the user has declared as being "active" or "subscribed."  `refname` and
  # `mailbox` are interpreted as for
  # [`list()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-list).
  # The return value is an array of `Net::IMAP::MailboxList`.
  def lsub(refname, mailbox); end

  # Sends a MOVE command to move the specified message(s) to the end of the
  # specified destination `mailbox`. The `set` parameter is a number, an array
  # of numbers, or a [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html)
  # object. The number is a message sequence number. The
  # [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) MOVE extension
  # is described in [RFC-6851].
  def move(set, mailbox); end

  # Sends a NOOP command to the server. It does nothing.
  def noop(); end

  # Removes the response handler.
  def remove_response_handler(handler); end

  # Sends a RENAME command to change the name of the `mailbox` to `newname`.
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if a mailbox with the name `mailbox` cannot be renamed to
  # `newname` for whatever reason; for instance, because `mailbox` does not
  # exist, or because there is already a mailbox with the name `newname`.
  def rename(mailbox, newname); end

  # Returns all response handlers.
  def response_handlers(); end

  # Returns recorded untagged responses. For example:
  #
  # ```ruby
  # imap.select("inbox")
  # p imap.responses["EXISTS"][-1]
  # #=> 2
  # p imap.responses["UIDVALIDITY"][-1]
  # #=> 968263756
  # ```
  def responses(); end

  # Sends a SEARCH command to search the mailbox for messages that match the
  # given searching criteria, and returns message sequence numbers. `keys` can
  # either be a string holding the entire search string, or a single-dimension
  # array of search keywords and arguments. The following are some common search
  # criteria; see [IMAP] section 6.4.4 for a full list.
  #
  # <message set>
  # :   a set of message sequence numbers. ',' indicates an interval, ':'
  #     indicates a range. For instance, '2,10:12,15' means "2,10,11,12,15".
  #
  # BEFORE <date>
  # :   messages with an internal date strictly before <date>. The date argument
  #     has a format similar to 8-Aug-2002.
  #
  # BODY <string>
  # :   messages that contain <string> within their body.
  #
  # CC <string>
  # :   messages containing <string> in their CC field.
  #
  # FROM <string>
  # :   messages that contain <string> in their FROM field.
  #
  # NEW
  # :   messages with the Recent, but not the Seen, flag set.
  #
  # NOT <search-key>
  # :   negate the following search key.
  #
  # OR <search-key> <search-key>
  # :   "or" two search keys together.
  #
  # ON <date>
  # :   messages with an internal date exactly equal to <date>, which has a
  #     format similar to 8-Aug-2002.
  #
  # SINCE <date>
  # :   messages with an internal date on or after <date>.
  #
  # SUBJECT <string>
  # :   messages with <string> in their subject.
  #
  # TO <string>
  # :   messages with <string> in their TO field.
  #
  #
  # For example:
  #
  # ```ruby
  # p imap.search(["SUBJECT", "hello", "NOT", "NEW"])
  # #=> [1, 6, 7, 8]
  # ```
  def search(keys, charset=T.unsafe(nil)); end

  # Sends a SELECT command to select a `mailbox` so that messages in the
  # `mailbox` can be accessed.
  #
  # After you have selected a mailbox, you may retrieve the number of items in
  # that mailbox from @[responses]("EXISTS")[-1], and the number of recent
  # messages from @[responses]("RECENT")[-1]. Note that these values can change
  # if new messages arrive during a session; see
  # [`add_response_handler()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-add_response_handler)
  # for a way of detecting this event.
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if the mailbox does not exist or is for some reason
  # non-selectable.
  def select(mailbox); end

  # Sends the SETACL command along with `mailbox`, `user` and the `rights` that
  # user is to have on that mailbox. If `rights` is nil, then that user will be
  # stripped of any rights to that mailbox. The
  # [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html)
  # [`ACL`](https://docs.ruby-lang.org/en/2.7.0/ACL.html) commands are described
  # in [RFC-2086].
  def setacl(mailbox, user, rights); end

  # Sends a SETQUOTA command along with the specified `mailbox` and `quota`. If
  # `quota` is nil, then `quota` will be unset for that mailbox. Typically one
  # needs to be logged in as a server admin for this to work. The
  # [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) quota commands
  # are described in [RFC-2087].
  def setquota(mailbox, quota); end

  # Sends a SORT command to sort messages in the mailbox. Returns an array of
  # message sequence numbers. For example:
  #
  # ```ruby
  # p imap.sort(["FROM"], ["ALL"], "US-ASCII")
  # #=> [1, 2, 3, 5, 6, 7, 8, 4, 9]
  # p imap.sort(["DATE"], ["SUBJECT", "hello"], "US-ASCII")
  # #=> [6, 7, 8, 1]
  # ```
  #
  # See [SORT-THREAD-EXT] for more details.
  def sort(sort_keys, search_keys, charset); end

  # Sends a STARTTLS command to start TLS session.
  def starttls(options=T.unsafe(nil), verify=T.unsafe(nil)); end

  # Sends a STATUS command, and returns the status of the indicated `mailbox`.
  # `attr` is a list of one or more attributes whose statuses are to be
  # requested. Supported attributes include:
  #
  # ```
  # MESSAGES:: the number of messages in the mailbox.
  # RECENT:: the number of recent messages in the mailbox.
  # UNSEEN:: the number of unseen messages in the mailbox.
  # ```
  #
  # The return value is a hash of attributes. For example:
  #
  # ```ruby
  # p imap.status("inbox", ["MESSAGES", "RECENT"])
  # #=> {"RECENT"=>0, "MESSAGES"=>44}
  # ```
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if status values for `mailbox` cannot be returned; for instance,
  # because it does not exist.
  def status(mailbox, attr); end

  # Sends a STORE command to alter data associated with messages in the mailbox,
  # in particular their flags. The `set` parameter is a number, an array of
  # numbers, or a [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html)
  # object. Each number is a message sequence number. `attr` is the name of a
  # data item to store: 'FLAGS' will replace the message's flag list with the
  # provided one, '+FLAGS' will add the provided flags, and '-FLAGS' will remove
  # them. `flags` is a list of flags.
  #
  # The return value is an array of
  # [`Net::IMAP::FetchData`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#FetchData).
  # For example:
  #
  # ```ruby
  # p imap.store(6..8, "+FLAGS", [:Deleted])
  # #=> [#<Net::IMAP::FetchData seqno=6, attr={"FLAGS"=>[:Seen, :Deleted]}>, \\
  #      #<Net::IMAP::FetchData seqno=7, attr={"FLAGS"=>[:Seen, :Deleted]}>, \\
  #      #<Net::IMAP::FetchData seqno=8, attr={"FLAGS"=>[:Seen, :Deleted]}>]
  # ```
  def store(set, attr, flags); end

  # Sends a SUBSCRIBE command to add the specified `mailbox` name to the
  # server's set of "active" or "subscribed" mailboxes as returned by
  # [`lsub()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-lsub).
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if `mailbox` cannot be subscribed to; for instance, because it
  # does not exist.
  def subscribe(mailbox); end

  # Similar to
  # [`search()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-search),
  # but returns message sequence numbers in threaded format, as a
  # [`Net::IMAP::ThreadMember`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#ThreadMember)
  # tree. The supported algorithms are:
  #
  # ORDEREDSUBJECT
  # :   split into single-level threads according to subject, ordered by date.
  # REFERENCES
  # :   split into threads by parent/child relationships determined by which
  #     message is a reply to which.
  #
  #
  # Unlike
  # [`search()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-search),
  # `charset` is a required argument. US-ASCII and UTF-8 are sample values.
  #
  # See [SORT-THREAD-EXT] for more details.
  def thread(algorithm, search_keys, charset); end

  # Similar to
  # [`copy()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-copy),
  # but `set` contains unique identifiers.
  def uid_copy(set, mailbox); end

  # Similar to
  # [`fetch()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-fetch),
  # but `set` contains unique identifiers.
  def uid_fetch(set, attr); end

  # Similar to
  # [`move()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-move),
  # but `set` contains unique identifiers.
  def uid_move(set, mailbox); end

  # Similar to
  # [`search()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-search),
  # but returns unique identifiers.
  def uid_search(keys, charset=T.unsafe(nil)); end

  # Similar to
  # [`sort()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-sort),
  # but returns an array of unique identifiers.
  def uid_sort(sort_keys, search_keys, charset); end

  # Similar to
  # [`store()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-store),
  # but `set` contains unique identifiers.
  def uid_store(set, attr, flags); end

  # Similar to
  # [`thread()`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-thread),
  # but returns unique identifiers instead of message sequence numbers.
  def uid_thread(algorithm, search_keys, charset); end

  # Sends a UNSUBSCRIBE command to remove the specified `mailbox` name from the
  # server's set of "active" or "subscribed" mailboxes.
  #
  # A
  # [`Net::IMAP::NoResponseError`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/NoResponseError.html)
  # is raised if `mailbox` cannot be unsubscribed from; for instance, because
  # the client is not currently subscribed to it.
  def unsubscribe(mailbox); end

  # Sends a XLIST command, and returns a subset of names from the complete set
  # of all names available to the client. `refname` provides a context (for
  # instance, a base directory in a directory-based mailbox hierarchy).
  # `mailbox` specifies a mailbox or (via wildcards) mailboxes under that
  # context. Two wildcards may be used in `mailbox`: '\*', which matches all
  # characters **including** the hierarchy delimiter (for instance, '/' on a
  # UNIX-hosted directory-based mailbox hierarchy); and '%', which matches all
  # characters **except** the hierarchy delimiter.
  #
  # If `refname` is empty, `mailbox` is used directly to determine which
  # mailboxes to match. If `mailbox` is empty, the root name of `refname` and
  # the hierarchy delimiter are returned.
  #
  # The XLIST command is like the LIST command except that the flags returned
  # refer to the function of the folder/mailbox, e.g. :Sent
  #
  # The return value is an array of `Net::IMAP::MailboxList`. For example:
  #
  # ```ruby
  # imap.create("foo/bar")
  # imap.create("foo/baz")
  # p imap.xlist("", "foo/%")
  # #=> [#<Net::IMAP::MailboxList attr=[:Noselect], delim="/", name="foo/">, \\
  #      #<Net::IMAP::MailboxList attr=[:Noinferiors, :Marked], delim="/", name="foo/bar">, \\
  #      #<Net::IMAP::MailboxList attr=[:Noinferiors], delim="/", name="foo/baz">]
  # ```
  def xlist(refname, mailbox); end

  # Adds an authenticator for
  # [`Net::IMAP#authenticate`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-authenticate).
  # `auth_type` is the type of authentication this authenticator supports (for
  # instance, "LOGIN"). The `authenticator` is an object which defines a
  # process() method to handle authentication with the server. See
  # [`Net::IMAP::LoginAuthenticator`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/LoginAuthenticator.html),
  # [`Net::IMAP::CramMD5Authenticator`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/CramMD5Authenticator.html),
  # and
  # [`Net::IMAP::DigestMD5Authenticator`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/DigestMD5Authenticator.html)
  # for examples.
  #
  # If `auth_type` refers to an existing authenticator, it will be replaced by
  # the new one.
  def self.add_authenticator(auth_type, authenticator); end

  # Returns the debug mode.
  def self.debug(); end

  # Sets the debug mode.
  def self.debug=(val); end

  # Decode a string from modified UTF-7 format to UTF-8.
  #
  # UTF-7 is a 7-bit encoding of Unicode [UTF7].
  # [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) uses a slightly
  # modified version of this to encode mailbox names containing non-ASCII
  # characters; see [IMAP] section 5.1.3.
  #
  # [`Net::IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) does *not*
  # automatically encode and decode mailbox names to and from UTF-7.
  def self.decode_utf7(s); end

  # Alias for:
  # [`default_port`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-c-default_port)
  def self.default_imap_port(); end

  # Alias for:
  # [`default_tls_port`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-c-default_tls_port)
  def self.default_imaps_port(); end

  # The default port for
  # [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) connections,
  # port 143
  #
  # Also aliased as:
  # [`default_imap_port`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-c-default_imap_port)
  def self.default_port(); end

  # Alias for:
  # [`default_tls_port`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-c-default_tls_port)
  def self.default_ssl_port(); end

  # The default port for IMAPS connections, port 993
  #
  # Also aliased as:
  # [`default_imaps_port`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-c-default_imaps_port),
  # [`default_ssl_port`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-c-default_ssl_port)
  def self.default_tls_port(); end

  # Encode a string from UTF-8 format to modified UTF-7.
  def self.encode_utf7(s); end

  # Formats `time` as an IMAP-style date.
  def self.format_date(time); end

  # Formats `time` as an IMAP-style date-time.
  def self.format_datetime(time); end

  # Returns the max number of flags interned to symbols.
  def self.max_flag_count(); end

  # Sets the max number of flags interned to symbols.
  def self.max_flag_count=(count); end
end

# [`Net::IMAP::Address`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Address)
# represents electronic mail addresses.
#
# #### Fields:
#
# name
# :   Returns the phrase from [RFC-822] mailbox.
#
# route
# :   Returns the route from [RFC-822] route-addr.
#
# mailbox
# :   nil indicates end of [RFC-822] group. If non-nil and host is nil, returns
#     [RFC-822] group name. Otherwise, returns [RFC-822] local-part.
#
# host
# :   nil indicates [RFC-822] group syntax. Otherwise, returns [RFC-822] domain
#     name.
class Net::IMAP::Address < Struct
  Elem = type_member {{fixed: T.untyped}}

  def host(); end

  def host=(_); end

  def mailbox(); end

  def mailbox=(_); end

  def name(); end

  def name=(_); end

  def route(); end

  def route=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

class Net::IMAP::Atom
  def initialize(data); end

  def send_data(imap); end

  def validate(); end
end

# [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) raised upon a "BAD"
# response from the server, indicating that the client command violated the
# [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html) protocol, or an
# internal server failure has occurred.
class Net::IMAP::BadResponseError < Net::IMAP::ResponseError
end

# [`Net::IMAP::BodyTypeAttachment`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeAttachment.html)
# represents attachment body structures of messages.
#
# #### Fields:
#
# media\_type
# :   Returns the content media type name.
#
# subtype
# :   Returns `nil`.
#
# param
# :   Returns a hash that represents parameters.
#
# multipart?
# :   Returns false.
class Net::IMAP::BodyTypeAttachment < Struct
  Elem = type_member {{fixed: T.untyped}}

  def media_type(); end

  def media_type=(_); end

  def multipart?(); end

  def param(); end

  def param=(_); end

  def subtype(); end

  def subtype=(_); end
end

# [`Net::IMAP::BodyTypeBasic`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeBasic.html)
# represents basic body structures of messages.
#
# #### Fields:
#
# media\_type
# :   Returns the content media type name as defined in [MIME-IMB].
#
# subtype
# :   Returns the content subtype name as defined in [MIME-IMB].
#
# param
# :   Returns a hash that represents parameters as defined in [MIME-IMB].
#
# content\_id
# :   Returns a string giving the content id as defined in [MIME-IMB].
#
# description
# :   Returns a string giving the content description as defined in [MIME-IMB].
#
# encoding
# :   Returns a string giving the content transfer encoding as defined in
#     [MIME-IMB].
#
# size
# :   Returns a number giving the size of the body in octets.
#
# md5
# :   Returns a string giving the body MD5 value as defined in [MD5].
#
# disposition
# :   Returns a Net::IMAP::ContentDisposition object giving the content
#     disposition.
#
# language
# :   Returns a string or an array of strings giving the body language value as
#     defined in [LANGUAGE-TAGS].
#
# extension
# :   Returns extension data.
#
# multipart?
# :   Returns false.
class Net::IMAP::BodyTypeBasic < Struct
  Elem = type_member {{fixed: T.untyped}}

  def content_id(); end

  def content_id=(_); end

  def description(); end

  def description=(_); end

  def disposition(); end

  def disposition=(_); end

  def encoding(); end

  def encoding=(_); end

  def extension(); end

  def extension=(_); end

  def language(); end

  def language=(_); end

  def md5(); end

  def md5=(_); end

  # Obsolete: use `subtype` instead. Calling this will generate a warning
  # message to `stderr`, then return the value of `subtype`.
  def media_subtype(); end

  def media_type(); end

  def media_type=(_); end

  def multipart?(); end

  def param(); end

  def param=(_); end

  def size(); end

  def size=(_); end

  def subtype(); end

  def subtype=(_); end
end

class Net::IMAP::BodyTypeExtension < Struct
  Elem = type_member {{fixed: T.untyped}}

  def content_id(); end

  def content_id=(_); end

  def description(); end

  def description=(_); end

  def encoding(); end

  def encoding=(_); end

  def media_type(); end

  def media_type=(_); end

  def multipart?(); end

  def params(); end

  def params=(_); end

  def size(); end

  def size=(_); end

  def subtype(); end

  def subtype=(_); end
end

# [`Net::IMAP::BodyTypeMessage`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeMessage.html)
# represents MESSAGE/RFC822 body structures of messages.
#
# #### Fields:
#
# envelope
# :   Returns a Net::IMAP::Envelope giving the envelope structure.
#
# body
# :   Returns an object giving the body structure.
#
#
# And
# [`Net::IMAP::BodyTypeMessage`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeMessage.html)
# has all methods of
# [`Net::IMAP::BodyTypeText`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeText.html).
class Net::IMAP::BodyTypeMessage < Struct
  Elem = type_member {{fixed: T.untyped}}

  def body(); end

  def body=(_); end

  def content_id(); end

  def content_id=(_); end

  def description(); end

  def description=(_); end

  def disposition(); end

  def disposition=(_); end

  def encoding(); end

  def encoding=(_); end

  def envelope(); end

  def envelope=(_); end

  def extension(); end

  def extension=(_); end

  def language(); end

  def language=(_); end

  def lines(); end

  def lines=(_); end

  def md5(); end

  def md5=(_); end

  # Obsolete: use `subtype` instead. Calling this will generate a warning
  # message to `stderr`, then return the value of `subtype`.
  def media_subtype(); end

  def media_type(); end

  def media_type=(_); end

  def multipart?(); end

  def param(); end

  def param=(_); end

  def size(); end

  def size=(_); end

  def subtype(); end

  def subtype=(_); end
end

# [`Net::IMAP::BodyTypeMultipart`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeMultipart.html)
# represents multipart body structures of messages.
#
# #### Fields:
#
# media\_type
# :   Returns the content media type name as defined in [MIME-IMB].
#
# subtype
# :   Returns the content subtype name as defined in [MIME-IMB].
#
# parts
# :   Returns multiple parts.
#
# param
# :   Returns a hash that represents parameters as defined in [MIME-IMB].
#
# disposition
# :   Returns a Net::IMAP::ContentDisposition object giving the content
#     disposition.
#
# language
# :   Returns a string or an array of strings giving the body language value as
#     defined in [LANGUAGE-TAGS].
#
# extension
# :   Returns extension data.
#
# multipart?
# :   Returns true.
class Net::IMAP::BodyTypeMultipart < Struct
  Elem = type_member {{fixed: T.untyped}}

  def disposition(); end

  def disposition=(_); end

  def extension(); end

  def extension=(_); end

  def language(); end

  def language=(_); end

  # Obsolete: use `subtype` instead. Calling this will generate a warning
  # message to `stderr`, then return the value of `subtype`.
  def media_subtype(); end

  def media_type(); end

  def media_type=(_); end

  def multipart?(); end

  def param(); end

  def param=(_); end

  def parts(); end

  def parts=(_); end

  def subtype(); end

  def subtype=(_); end
end

# [`Net::IMAP::BodyTypeText`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeText.html)
# represents TEXT body structures of messages.
#
# #### Fields:
#
# lines
# :   Returns the size of the body in text lines.
#
#
# And
# [`Net::IMAP::BodyTypeText`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeText.html)
# has all fields of
# [`Net::IMAP::BodyTypeBasic`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeBasic.html).
class Net::IMAP::BodyTypeText < Struct
  Elem = type_member {{fixed: T.untyped}}

  def content_id(); end

  def content_id=(_); end

  def description(); end

  def description=(_); end

  def disposition(); end

  def disposition=(_); end

  def encoding(); end

  def encoding=(_); end

  def extension(); end

  def extension=(_); end

  def language(); end

  def language=(_); end

  def lines(); end

  def lines=(_); end

  def md5(); end

  def md5=(_); end

  # Obsolete: use `subtype` instead. Calling this will generate a warning
  # message to `stderr`, then return the value of `subtype`.
  def media_subtype(); end

  def media_type(); end

  def media_type=(_); end

  def multipart?(); end

  def param(); end

  def param=(_); end

  def size(); end

  def size=(_); end

  def subtype(); end

  def subtype=(_); end
end

# [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) raised upon a "BYE"
# response from the server, indicating that the client is not being allowed to
# login, or has been timed out due to inactivity.
class Net::IMAP::ByeResponseError < Net::IMAP::ResponseError
end

# [`Net::IMAP::ContentDisposition`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#ContentDisposition)
# represents Content-Disposition fields.
#
# #### Fields:
#
# dsp\_type
# :   Returns the disposition type.
#
# param
# :   Returns a hash that represents parameters of the Content-Disposition
#     field.
class Net::IMAP::ContentDisposition < Struct
  Elem = type_member {{fixed: T.untyped}}

  def dsp_type(); end

  def dsp_type=(_); end

  def param(); end

  def param=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::ContinuationRequest`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#ContinuationRequest)
# represents command continuation requests.
#
# The command continuation request response is indicated by a "+" token instead
# of a tag. This form of response indicates that the server is ready to accept
# the continuation of a command from the client. The remainder of this response
# is a line of text.
#
# ```
# continue_req    ::= "+" SPACE (resp_text / base64)
# ```
#
# #### Fields:
#
# data
# :   Returns the data
#     ([`Net::IMAP::ResponseText`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#ResponseText)).
#
# raw\_data
# :   Returns the raw data string.
class Net::IMAP::ContinuationRequest < Struct
  Elem = type_member {{fixed: T.untyped}}

  def data(); end

  def data=(_); end

  def raw_data(); end

  def raw_data=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# Authenticator for the "CRAM-MD5" authentication type. See authenticate().
class Net::IMAP::CramMD5Authenticator
  def initialize(user, password); end

  def process(challenge); end
end

# [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) raised when data is
# in the incorrect format.
class Net::IMAP::DataFormatError < Net::IMAP::Error
end

# Authenticator for the "DIGEST-MD5" authentication type. See authenticate().
class Net::IMAP::DigestMD5Authenticator
  STAGE_ONE = ::T.unsafe(nil)
  STAGE_TWO = ::T.unsafe(nil)

  def initialize(user, password, authname=T.unsafe(nil)); end

  def process(challenge); end
end

# [`Net::IMAP::Envelope`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Envelope)
# represents envelope structures of messages.
#
# #### Fields:
#
# date
# :   Returns a string that represents the date.
#
# subject
# :   Returns a string that represents the subject.
#
# from
# :   Returns an array of
#     [`Net::IMAP::Address`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Address)
#     that represents the from.
#
# sender
# :   Returns an array of
#     [`Net::IMAP::Address`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Address)
#     that represents the sender.
#
# reply\_to
# :   Returns an array of
#     [`Net::IMAP::Address`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Address)
#     that represents the reply-to.
#
# to
# :   Returns an array of
#     [`Net::IMAP::Address`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Address)
#     that represents the to.
#
# cc
# :   Returns an array of
#     [`Net::IMAP::Address`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Address)
#     that represents the cc.
#
# bcc
# :   Returns an array of
#     [`Net::IMAP::Address`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Address)
#     that represents the bcc.
#
# in\_reply\_to
# :   Returns a string that represents the in-reply-to.
#
# message\_id
# :   Returns a string that represents the message-id.
class Net::IMAP::Envelope < Struct
  Elem = type_member {{fixed: T.untyped}}

  def bcc(); end

  def bcc=(_); end

  def cc(); end

  def cc=(_); end

  def date(); end

  def date=(_); end

  def from(); end

  def from=(_); end

  def in_reply_to(); end

  def in_reply_to=(_); end

  def message_id(); end

  def message_id=(_); end

  def reply_to(); end

  def reply_to=(_); end

  def sender(); end

  def sender=(_); end

  def subject(); end

  def subject=(_); end

  def to(); end

  def to=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# Superclass of [`IMAP`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html)
# errors.
class Net::IMAP::Error < StandardError
end

# [`Net::IMAP::FetchData`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#FetchData)
# represents the contents of the FETCH response.
#
# #### Fields:
#
# seqno
# :   Returns the message sequence number. (Note: not the unique identifier,
#     even for the UID command response.)
#
# attr
# :   Returns a hash. Each key is a data item name, and each value is its value.
#
#     The current data items are:
#
#     BODY
# :       A form of BODYSTRUCTURE without extension data.
#     [BODY](<section>)<<origin\_octet>>
# :       A string expressing the body contents of the specified section.
#     BODYSTRUCTURE
# :       An object that describes the [MIME-IMB] body structure of a message.
#         See
#         [`Net::IMAP::BodyTypeBasic`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeBasic.html),
#         [`Net::IMAP::BodyTypeText`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeText.html),
#         [`Net::IMAP::BodyTypeMessage`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeMessage.html),
#         [`Net::IMAP::BodyTypeMultipart`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP/BodyTypeMultipart.html).
#     ENVELOPE
# :       A
#         [`Net::IMAP::Envelope`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#Envelope)
#         object that describes the envelope structure of a message.
#     FLAGS
# :       A array of flag symbols that are set for this message. Flag symbols
#         are capitalized by
#         [`String#capitalize`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-capitalize).
#     INTERNALDATE
# :       A string representing the internal date of the message.
#     RFC822
# :       Equivalent to BODY[].
#     RFC822.HEADER
# :       Equivalent to [BODY.PEEK](HEADER).
#     RFC822.SIZE
# :       A number expressing the [RFC-822] size of the message.
#     RFC822.TEXT
# :       Equivalent to [BODY](TEXT).
#     UID
# :       A number expressing the unique identifier of the message.
class Net::IMAP::FetchData < Struct
  Elem = type_member {{fixed: T.untyped}}

  def attr(); end

  def attr=(_); end

  def seqno(); end

  def seqno=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) raised when too many
# flags are interned to symbols.
class Net::IMAP::FlagCountError < Net::IMAP::Error
end

class Net::IMAP::Literal
  def initialize(data); end

  def send_data(imap); end

  def validate(); end
end

# Authenticator for the "LOGIN" authentication type. See authenticate().
class Net::IMAP::LoginAuthenticator
  STATE_PASSWORD = ::T.unsafe(nil)
  STATE_USER = ::T.unsafe(nil)

  def initialize(user, password); end

  def process(data); end
end

# [`Net::IMAP::MailboxACLItem`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxACLItem)
# represents the response from GETACL.
#
# ```
# acl_data        ::= "ACL" SPACE mailbox *(SPACE identifier SPACE rights)
#
# identifier      ::= astring
#
# rights          ::= astring
# ```
#
# #### Fields:
#
# user
# :   Login name that has certain rights to the mailbox that was specified with
#     the getacl command.
#
# rights
# :   The access rights the indicated user has to the mailbox.
class Net::IMAP::MailboxACLItem < Struct
  Elem = type_member {{fixed: T.untyped}}

  def mailbox(); end

  def mailbox=(_); end

  def rights(); end

  def rights=(_); end

  def user(); end

  def user=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::MailboxList`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxList)
# represents contents of the LIST response.
#
# ```
# mailbox_list    ::= "(" #("\Marked" / "\Noinferiors" /
#                     "\Noselect" / "\Unmarked" / flag_extension) ")"
#                     SPACE (<"> QUOTED_CHAR <"> / nil) SPACE mailbox
# ```
#
# #### Fields:
#
# attr
# :   Returns the name attributes. Each name attribute is a symbol capitalized
#     by
#     [`String#capitalize`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-capitalize),
#     such as :Noselect (not :NoSelect).
#
# delim
# :   Returns the hierarchy delimiter.
#
# name
# :   Returns the mailbox name.
class Net::IMAP::MailboxList < Struct
  Elem = type_member {{fixed: T.untyped}}

  def attr(); end

  def attr=(_); end

  def delim(); end

  def delim=(_); end

  def name(); end

  def name=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::MailboxQuota`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxQuota)
# represents contents of GETQUOTA response. This object can also be a response
# to GETQUOTAROOT. In the syntax specification below, the delimiter used with
# the "#" construct is a single space (SPACE).
#
# ```
# quota_list      ::= "(" #quota_resource ")"
#
# quota_resource  ::= atom SPACE number SPACE number
#
# quota_response  ::= "QUOTA" SPACE astring SPACE quota_list
# ```
#
# #### Fields:
#
# mailbox
# :   The mailbox with the associated quota.
#
# usage
# :   Current storage usage of the mailbox.
#
# quota
# :   Quota limit imposed on the mailbox.
class Net::IMAP::MailboxQuota < Struct
  Elem = type_member {{fixed: T.untyped}}

  def mailbox(); end

  def mailbox=(_); end

  def quota(); end

  def quota=(_); end

  def usage(); end

  def usage=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::MailboxQuotaRoot`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxQuotaRoot)
# represents part of the GETQUOTAROOT response. (GETQUOTAROOT can also return
# [`Net::IMAP::MailboxQuota`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#MailboxQuota).)
#
# ```
# quotaroot_response ::= "QUOTAROOT" SPACE astring *(SPACE astring)
# ```
#
# #### Fields:
#
# mailbox
# :   The mailbox with the associated quota.
#
# quotaroots
# :   Zero or more quotaroots that affect the quota on the specified mailbox.
class Net::IMAP::MailboxQuotaRoot < Struct
  Elem = type_member {{fixed: T.untyped}}

  def mailbox(); end

  def mailbox=(_); end

  def quotaroots(); end

  def quotaroots=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

class Net::IMAP::MessageSet
  def initialize(data); end

  def send_data(imap); end

  def validate(); end
end

# [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) raised upon a "NO"
# response from the server, indicating that the client command could not be
# completed successfully.
class Net::IMAP::NoResponseError < Net::IMAP::ResponseError
end

# Common validators of number and nz\_number types
module Net::IMAP::NumValidator
  # Ensure argument is 'number' or raise DataFormatError
  def self.ensure_number(num); end

  # Ensure argument is 'nz\_number' or raise DataFormatError
  def self.ensure_nz_number(num); end

  # Check is passed argument valid 'number' in RFC 3501 terminology
  def self.valid_number?(num); end

  # Check is passed argument valid 'nz\_number' in RFC 3501 terminology
  def self.valid_nz_number?(num); end
end

# Authenticator for the "PLAIN" authentication type. See authenticate().
class Net::IMAP::PlainAuthenticator
  def initialize(user, password); end

  def process(data); end
end

class Net::IMAP::QuotedString
  def initialize(data); end

  def send_data(imap); end

  def validate(); end
end

class Net::IMAP::RawData
  def initialize(data); end

  def send_data(imap); end

  def validate(); end
end

# [`Net::IMAP::ResponseCode`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#ResponseCode)
# represents response codes.
#
# ```
# resp_text_code  ::= "ALERT" / "PARSE" /
#                     "PERMANENTFLAGS" SPACE "(" #(flag / "\*") ")" /
#                     "READ-ONLY" / "READ-WRITE" / "TRYCREATE" /
#                     "UIDVALIDITY" SPACE nz_number /
#                     "UNSEEN" SPACE nz_number /
#                     atom [SPACE 1*<any TEXT_CHAR except "]">]
# ```
#
# #### Fields:
#
# name
# :   Returns the name, such as "ALERT", "PERMANENTFLAGS", or "UIDVALIDITY".
#
# data
# :   Returns the data, if it exists.
class Net::IMAP::ResponseCode < Struct
  Elem = type_member {{fixed: T.untyped}}

  def data(); end

  def data=(_); end

  def name(); end

  def name=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# Superclass of all errors used to encapsulate "fail" responses from the server.
class Net::IMAP::ResponseError < Net::IMAP::Error
  def initialize(response); end

  # The response that caused this error
  def response(); end

  # The response that caused this error
  def response=(response); end
end

# [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) raised when a
# response from the server is non-parseable.
class Net::IMAP::ResponseParseError < Net::IMAP::Error
end

class Net::IMAP::ResponseParser
  ADDRESS_REGEXP = ::T.unsafe(nil)
  ATOM_TOKENS = ::T.unsafe(nil)
  BEG_REGEXP = ::T.unsafe(nil)
  CTEXT_REGEXP = ::T.unsafe(nil)
  DATA_REGEXP = ::T.unsafe(nil)
  EXPR_BEG = ::T.unsafe(nil)
  EXPR_CTEXT = ::T.unsafe(nil)
  EXPR_DATA = ::T.unsafe(nil)
  EXPR_RTEXT = ::T.unsafe(nil)
  EXPR_TEXT = ::T.unsafe(nil)
  FLAG_REGEXP = ::T.unsafe(nil)
  RTEXT_REGEXP = ::T.unsafe(nil)
  STRING_TOKENS = ::T.unsafe(nil)
  TEXT_REGEXP = ::T.unsafe(nil)
  T_ATOM = ::T.unsafe(nil)
  T_BSLASH = ::T.unsafe(nil)
  T_CRLF = ::T.unsafe(nil)
  T_EOF = ::T.unsafe(nil)
  T_LBRA = ::T.unsafe(nil)
  T_LITERAL = ::T.unsafe(nil)
  T_LPAR = ::T.unsafe(nil)
  T_NIL = ::T.unsafe(nil)
  T_NUMBER = ::T.unsafe(nil)
  T_PERCENT = ::T.unsafe(nil)
  T_PLUS = ::T.unsafe(nil)
  T_QUOTED = ::T.unsafe(nil)
  T_RBRA = ::T.unsafe(nil)
  T_RPAR = ::T.unsafe(nil)
  T_SPACE = ::T.unsafe(nil)
  T_STAR = ::T.unsafe(nil)
  T_TEXT = ::T.unsafe(nil)

  def initialize(); end

  def parse(str); end
end

class Net::IMAP::ResponseParser::Token < Struct
  Elem = type_member {{fixed: T.untyped}}

  def symbol(); end

  def symbol=(_); end

  def value(); end

  def value=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::ResponseText`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#ResponseText)
# represents texts of responses. The text may be prefixed by the response code.
#
# ```
# resp_text       ::= ["[" resp_text_code "]" SPACE] (text_mime2 / text)
#                     ;; text SHOULD NOT begin with "[" or "="
# ```
#
# #### Fields:
#
# code
# :   Returns the response code. See ((<Net::IMAP::ResponseCode>)).
#
# text
# :   Returns the text.
class Net::IMAP::ResponseText < Struct
  Elem = type_member {{fixed: T.untyped}}

  def code(); end

  def code=(_); end

  def text(); end

  def text=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::StatusData`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#StatusData)
# represents the contents of the STATUS response.
#
# #### Fields:
#
# mailbox
# :   Returns the mailbox name.
#
# attr
# :   Returns a hash. Each key is one of "MESSAGES", "RECENT", "UIDNEXT",
#     "UIDVALIDITY", "UNSEEN". Each value is a number.
class Net::IMAP::StatusData < Struct
  Elem = type_member {{fixed: T.untyped}}

  def attr(); end

  def attr=(_); end

  def mailbox(); end

  def mailbox=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::TaggedResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#TaggedResponse)
# represents tagged responses.
#
# The server completion result response indicates the success or failure of the
# operation. It is tagged with the same tag as the client command which began
# the operation.
#
# ```
# response_tagged ::= tag SPACE resp_cond_state CRLF
#
# tag             ::= 1*<any ATOM_CHAR except "+">
#
# resp_cond_state ::= ("OK" / "NO" / "BAD") SPACE resp_text
# ```
#
# #### Fields:
#
# tag
# :   Returns the tag.
#
# name
# :   Returns the name, one of "OK", "NO", or "BAD".
#
# data
# :   Returns the data. See ((<Net::IMAP::ResponseText>)).
#
# raw\_data
# :   Returns the raw data string.
class Net::IMAP::TaggedResponse < Struct
  Elem = type_member {{fixed: T.untyped}}

  def data(); end

  def data=(_); end

  def name(); end

  def name=(_); end

  def raw_data(); end

  def raw_data=(_); end

  def tag(); end

  def tag=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::ThreadMember`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#ThreadMember)
# represents a thread-node returned by
# [`Net::IMAP#thread`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#method-i-thread).
#
# #### Fields:
#
# seqno
# :   The sequence number of this message.
#
# children
# :   An array of
#     [`Net::IMAP::ThreadMember`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#ThreadMember)
#     objects for mail items that are children of this in the thread.
class Net::IMAP::ThreadMember < Struct
  Elem = type_member {{fixed: T.untyped}}

  def children(); end

  def children=(_); end

  def seqno(); end

  def seqno=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

# [`Net::IMAP::UntaggedResponse`](https://docs.ruby-lang.org/en/2.7.0/Net/IMAP.html#UntaggedResponse)
# represents untagged responses.
#
# [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) transmitted by the
# server to the client and status responses that do not indicate command
# completion are prefixed with the token "\*", and are called untagged
# responses.
#
# ```
# response_data   ::= "*" SPACE (resp_cond_state / resp_cond_bye /
#                     mailbox_data / message_data / capability_data)
# ```
#
# #### Fields:
#
# name
# :   Returns the name, such as "FLAGS", "LIST", or "FETCH".
#
# data
# :   Returns the data such as an array of flag symbols, a
#     ((<Net::IMAP::MailboxList>)) object.
#
# raw\_data
# :   Returns the raw data string.
class Net::IMAP::UntaggedResponse < Struct
  Elem = type_member {{fixed: T.untyped}}

  def data(); end

  def data=(_); end

  def name(); end

  def name=(_); end

  def raw_data(); end

  def raw_data=(_); end

  def self.[](*_); end

  def self.members(); end

  def self.new(*_); end
end

class Net::InternetMessageIO < Net::BufferedIO
  def each_list_item(); end

  def each_message_chunk(); end

  def initialize(*_); end

  def write_message(src); end

  def write_message_0(src); end

  def write_message_by_block(&block); end
end

module Net::NTLM
  BLOB_SIGN = ::T.unsafe(nil)
  DEFAULT_FLAGS = ::T.unsafe(nil)
  FLAGS = ::T.unsafe(nil)
  FLAG_KEYS = ::T.unsafe(nil)
  LM_MAGIC = ::T.unsafe(nil)
  MAX64 = ::T.unsafe(nil)
  SSP_SIGN = ::T.unsafe(nil)
  TIME_OFFSET = ::T.unsafe(nil)

  def self.apply_des(plain, keys); end

  def self.gen_keys(str); end

  def self.is_ntlm_hash?(data); end

  def self.lm_hash(password); end

  def self.lm_response(arg); end

  def self.lmv2_response(arg, opt=T.unsafe(nil)); end

  def self.ntlm2_session(arg, opt=T.unsafe(nil)); end

  def self.ntlm_hash(password, opt=T.unsafe(nil)); end

  def self.ntlm_response(arg); end

  def self.ntlmv2_hash(user, password, target, opt=T.unsafe(nil)); end

  def self.ntlmv2_response(arg, opt=T.unsafe(nil)); end

  def self.pack_int64le(val); end

  def self.split7(str); end
end

class Net::NTLM::Blob < Net::NTLM::FieldSet
  def blob_signature(); end

  def blob_signature=(val); end

  def challenge(); end

  def challenge=(val); end

  def parse(str, offset=T.unsafe(nil)); end

  def reserved(); end

  def reserved=(val); end

  def target_info(); end

  def target_info=(val); end

  def timestamp(); end

  def timestamp=(val); end

  def unknown1(); end

  def unknown1=(val); end

  def unknown2(); end

  def unknown2=(val); end
end

class Net::NTLM::ChannelBinding
  def acceptor_address_length(); end

  def acceptor_addrtype(); end

  def application_data(); end

  def channel(); end

  def channel_binding_token(); end

  def channel_hash(); end

  def gss_channel_bindings_struct(); end

  def initialize(outer_channel); end

  def initiator_address_length(); end

  def initiator_addtype(); end

  def unique_prefix(); end

  def self.create(outer_channel); end
end

class Net::NTLM::Client
  DEFAULT_FLAGS = ::T.unsafe(nil)

  def domain(); end

  def flags(); end

  def init_context(resp=T.unsafe(nil), channel_binding=T.unsafe(nil)); end

  def initialize(username, password, opts=T.unsafe(nil)); end

  def password(); end

  def session(); end

  def session_key(); end

  def username(); end

  def workstation(); end
end

class Net::NTLM::Client::Session
  CLIENT_TO_SERVER_SEALING = ::T.unsafe(nil)
  CLIENT_TO_SERVER_SIGNING = ::T.unsafe(nil)
  MAX64 = ::T.unsafe(nil)
  SERVER_TO_CLIENT_SEALING = ::T.unsafe(nil)
  SERVER_TO_CLIENT_SIGNING = ::T.unsafe(nil)
  TIME_OFFSET = ::T.unsafe(nil)
  VERSION_MAGIC = ::T.unsafe(nil)

  def authenticate!(); end

  def challenge_message(); end

  def channel_binding(); end

  def client(); end

  def exported_session_key(); end

  def initialize(client, challenge_message, channel_binding=T.unsafe(nil)); end

  def seal_message(message); end

  def sign_message(message); end

  def unseal_message(emessage); end

  def verify_signature(signature, message); end
end

class Net::NTLM::EncodeUtil
  def self.decode_utf16le(str); end

  def self.encode_utf16le(str); end
end

class Net::NTLM::Field
  def active(); end

  def active=(active); end

  def initialize(opts); end

  def parse(str, offset=T.unsafe(nil)); end

  def serialize(); end

  def size(); end

  def value(); end

  def value=(value); end
end

class Net::NTLM::FieldSet
  def [](name); end

  def []=(name, val); end

  def disable(name); end

  def enable(name); end

  def has_disabled_fields?(); end

  def initialize(); end

  def parse(str, offset=T.unsafe(nil)); end

  def serialize(); end

  def size(); end

  def self.int16LE(name, opts); end

  def self.int32LE(name, opts); end

  def self.int64LE(name, opts); end

  def self.names(); end

  def self.opts(); end

  def self.prototypes(); end

  def self.security_buffer(name, opts); end

  def self.string(name, opts); end

  def self.types(); end
end

class Net::NTLM::Int16LE < Net::NTLM::Field
  def initialize(opt); end

  def parse(str, offset=T.unsafe(nil)); end

  def serialize(); end
end

class Net::NTLM::Int32LE < Net::NTLM::Field
  def initialize(opt); end

  def parse(str, offset=T.unsafe(nil)); end

  def serialize(); end
end

class Net::NTLM::Int64LE < Net::NTLM::Field
  def initialize(opt); end

  def parse(str, offset=T.unsafe(nil)); end

  def serialize(); end
end

class Net::NTLM::InvalidTargetDataError < Net::NTLM::NtlmError
  def data(); end

  def initialize(msg, data); end
end

class Net::NTLM::Message < Net::NTLM::FieldSet
  def data_edge(); end

  def data_size(); end

  def decode64(str); end

  def deflag(); end

  def dump_flags(); end

  def encode64(); end

  def has_flag?(flag); end

  def head_size(); end

  def parse(str); end

  def security_buffers(); end

  def serialize(); end

  def set_flag(flag); end

  def size(); end

  def self.decode64(str); end

  def self.parse(str); end
end

class Net::NTLM::Message::Type0 < Net::NTLM::Message
  def sign(); end

  def sign=(val); end

  def type(); end

  def type=(val); end
end

class Net::NTLM::Message::Type1 < Net::NTLM::Message
  def domain(); end

  def domain=(val); end

  def flag(); end

  def flag=(val); end

  def os_version(); end

  def os_version=(val); end

  def sign(); end

  def sign=(val); end

  def type(); end

  def type=(val); end

  def workstation(); end

  def workstation=(val); end
end

class Net::NTLM::Message::Type2 < Net::NTLM::Message
  def challenge(); end

  def challenge=(val); end

  def context(); end

  def context=(val); end

  def flag(); end

  def flag=(val); end

  def os_version(); end

  def os_version=(val); end

  def response(arg, opt=T.unsafe(nil)); end

  def sign(); end

  def sign=(val); end

  def target_info(); end

  def target_info=(val); end

  def target_name(); end

  def target_name=(val); end

  def type(); end

  def type=(val); end
end

class Net::NTLM::Message::Type3 < Net::NTLM::Message
  def blank_password?(server_challenge); end

  def domain(); end

  def domain=(val); end

  def flag(); end

  def flag=(val); end

  def lm_response(); end

  def lm_response=(val); end

  def ntlm_response(); end

  def ntlm_response=(val); end

  def ntlm_version(); end

  def os_version(); end

  def os_version=(val); end

  def password?(password, server_challenge); end

  def session_key(); end

  def session_key=(val); end

  def sign(); end

  def sign=(val); end

  def type(); end

  def type=(val); end

  def user(); end

  def user=(val); end

  def workstation(); end

  def workstation=(val); end

  def self.create(arg, opt=T.unsafe(nil)); end
end

class Net::NTLM::NtlmError < StandardError
end

class Net::NTLM::SecurityBuffer < Net::NTLM::FieldSet
  def active(); end

  def active=(active); end

  def allocated(); end

  def allocated=(val); end

  def data_size(); end

  def initialize(opts=T.unsafe(nil)); end

  def length(); end

  def length=(val); end

  def offset(); end

  def offset=(val); end

  def parse(str, offset=T.unsafe(nil)); end

  def serialize(); end

  def value(); end

  def value=(val); end
end

class Net::NTLM::String < Net::NTLM::Field
  def initialize(opts); end

  def parse(str, offset=T.unsafe(nil)); end

  def serialize(); end

  def value=(val); end
end

class Net::NTLM::TargetInfo
  MSV_AV_CHANNEL_BINDINGS = ::T.unsafe(nil)
  MSV_AV_DNS_COMPUTER_NAME = ::T.unsafe(nil)
  MSV_AV_DNS_DOMAIN_NAME = ::T.unsafe(nil)
  MSV_AV_DNS_TREE_NAME = ::T.unsafe(nil)
  MSV_AV_EOL = ::T.unsafe(nil)
  MSV_AV_FLAGS = ::T.unsafe(nil)
  MSV_AV_NB_COMPUTER_NAME = ::T.unsafe(nil)
  MSV_AV_NB_DOMAIN_NAME = ::T.unsafe(nil)
  MSV_AV_SINGLE_HOST = ::T.unsafe(nil)
  MSV_AV_TARGET_NAME = ::T.unsafe(nil)
  MSV_AV_TIMESTAMP = ::T.unsafe(nil)
  VALID_PAIR_ID = ::T.unsafe(nil)

  def av_pairs(); end

  def initialize(av_pair_sequence); end

  def to_s(); end
end

module Net::NetPrivate
end

# [`OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html), a
# subclass of
# [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html), is
# raised if a connection cannot be created within the open\_timeout.
class Net::OpenTimeout < Timeout::Error
end

class Net::ProtoAuthError < Net::ProtocolError
end

class Net::ProtoCommandError < Net::ProtocolError
end

class Net::ProtoFatalError < Net::ProtocolError
end

class Net::ProtoRetriableError < Net::ProtocolError
end

class Net::ProtoServerError < Net::ProtocolError
end

class Net::ProtoSyntaxError < Net::ProtocolError
end

class Net::ProtoUnknownError < Net::ProtocolError
end

class Net::Protocol
  def self.protocol_param(name, val); end
end

class Net::ProtocolError < StandardError
end

class Net::ReadAdapter
  def <<(str); end

  def initialize(block); end

  def inspect(); end
end

# [`ReadTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/ReadTimeout.html), a
# subclass of
# [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html), is
# raised if a chunk of the response cannot be read within the read\_timeout.
class Net::ReadTimeout < Timeout::Error
end

# ## What is This Library?
#
# This library provides functionality to send internet mail via
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html), the Simple Mail
# Transfer Protocol. For details of
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) itself, see
# [RFC2821] (http://www.ietf.org/rfc/rfc2821.txt).
#
# ## What is This Library NOT?
#
# This library does NOT provide functions to compose internet mails. You must
# create them by yourself. If you want better mail support, try RubyMail or
# TMail or search for alternatives in [RubyGems.org](https://rubygems.org/) or
# [The Ruby Toolbox](https://www.ruby-toolbox.com/).
#
# FYI: the official documentation on internet mail is: [RFC2822]
# (http://www.ietf.org/rfc/rfc2822.txt).
#
# ## Examples
#
# ### Sending Messages
#
# You must open a connection to an
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) server before
# sending messages. The first argument is the address of your
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) server, and the
# second argument is the port number. Using
# [`SMTP.start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-c-start)
# with a block is the simplest way to do this. This way, the
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) connection is
# closed automatically after the block is executed.
#
# ```ruby
# require 'net/smtp'
# Net::SMTP.start('your.smtp.server', 25) do |smtp|
#   # Use the SMTP object smtp only in this block.
# end
# ```
#
# Replace 'your.smtp.server' with your
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) server. Normally
# your system manager or internet provider supplies a server for you.
#
# Then you can send messages.
#
# ```ruby
# msgstr = <<END_OF_MESSAGE
# From: Your Name <your@mail.address>
# To: Destination Address <someone@example.com>
# Subject: test message
# Date: Sat, 23 Jun 2001 16:26:43 +0900
# Message-Id: <unique.message.id.string@example.com>
#
# This is a test message.
# END_OF_MESSAGE
#
# require 'net/smtp'
# Net::SMTP.start('your.smtp.server', 25) do |smtp|
#   smtp.send_message msgstr,
#                     'your@mail.address',
#                     'his_address@example.com'
# end
# ```
#
# ### Closing the Session
#
# You MUST close the [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html)
# session after sending messages, by calling the
# [`finish`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-finish)
# method:
#
# ```ruby
# # using SMTP#finish
# smtp = Net::SMTP.start('your.smtp.server', 25)
# smtp.send_message msgstr, 'from@address', 'to@address'
# smtp.finish
# ```
#
# You can also use the block form of
# [`SMTP.start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-c-start)/SMTP#start.
# This closes the [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html)
# session automatically:
#
# ```ruby
# # using block form of SMTP.start
# Net::SMTP.start('your.smtp.server', 25) do |smtp|
#   smtp.send_message msgstr, 'from@address', 'to@address'
# end
# ```
#
# I strongly recommend this scheme. This form is simpler and more robust.
#
# ### HELO domain
#
# In almost all situations, you must provide a third argument to
# [`SMTP.start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-c-start)/SMTP#start.
# This is the domain name which you are on (the host to send mail from). It is
# called the "HELO domain". The
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) server will judge
# whether it should send or reject the
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) session by
# inspecting the HELO domain.
#
# ```
# Net::SMTP.start('your.smtp.server', 25,
#                 'mail.from.domain') { |smtp| ... }
# ```
#
# ### [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) Authentication
#
# The [`Net::SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) class
# supports three authentication schemes; PLAIN, LOGIN and CRAM MD5.
# ([`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) Authentication:
# [RFC2554]) To use [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html)
# authentication, pass extra arguments to
# [`SMTP.start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-c-start)/SMTP#start.
#
# ```ruby
# # PLAIN
# Net::SMTP.start('your.smtp.server', 25, 'mail.from.domain',
#                 'Your Account', 'Your Password', :plain)
# # LOGIN
# Net::SMTP.start('your.smtp.server', 25, 'mail.from.domain',
#                 'Your Account', 'Your Password', :login)
#
# # CRAM MD5
# Net::SMTP.start('your.smtp.server', 25, 'mail.from.domain',
#                 'Your Account', 'Your Password', :cram_md5)
# ```
class Net::SMTP < Net::Protocol
  CRAM_BUFSIZE = ::T.unsafe(nil)
  # Authentication
  DEFAULT_AUTH_TYPE = ::T.unsafe(nil)
  IMASK = ::T.unsafe(nil)
  OMASK = ::T.unsafe(nil)
  Revision = ::T.unsafe(nil)

  # The address of the
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) server to
  # connect to.
  def address(); end

  def auth_cram_md5(user, secret); end

  def auth_login(user, secret); end

  def auth_plain(user, secret); end

  def authenticate(user, secret, authtype=T.unsafe(nil)); end

  # Returns supported authentication methods on this server. You cannot get
  # valid value before opening
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) session.
  def capable_auth_types(); end

  # true if server advertises AUTH CRAM-MD5. You cannot get valid value before
  # opening [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) session.
  def capable_cram_md5_auth?(); end

  # true if server advertises AUTH LOGIN. You cannot get valid value before
  # opening [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) session.
  def capable_login_auth?(); end

  # true if server advertises AUTH PLAIN. You cannot get valid value before
  # opening [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) session.
  def capable_plain_auth?(); end

  # true if server advertises STARTTLS. You cannot get valid value before
  # opening [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) session.
  def capable_starttls?(); end

  # This method sends a message. If `msgstr` is given, sends it as a message. If
  # block is given, yield a message writer stream. You must write message before
  # the block is closed.
  #
  # ```ruby
  # # Example 1 (by string)
  # smtp.data(<<EndMessage)
  # From: john@example.com
  # To: betty@example.com
  # Subject: I found a bug
  #
  # Check vm.c:58879.
  # EndMessage
  #
  # # Example 2 (by block)
  # smtp.data {|f|
  #   f.puts "From: john@example.com"
  #   f.puts "To: betty@example.com"
  #   f.puts "Subject: I found a bug"
  #   f.puts ""
  #   f.puts "Check vm.c:58879."
  # }
  # ```
  def data(msgstr=T.unsafe(nil), &block); end

  # WARNING: This method causes serious security holes. Use this method for only
  # debugging.
  #
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) an output stream for
  # debug logging. You must call this before
  # [`start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-start).
  #
  # ```
  # # example
  # smtp = Net::SMTP.new(addr, port)
  # smtp.set_debug_output $stderr
  # smtp.start do |smtp|
  #   ....
  # end
  # ```
  #
  #
  # Also aliased as:
  # [`set_debug_output`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-set_debug_output)
  def debug_output=(arg); end

  # Alias for:
  # [`disable_tls`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-disable_tls)
  def disable_ssl(); end

  # Disables SMTP/TLS (STARTTLS) for this object. Must be called before the
  # connection is established to have any effect.
  def disable_starttls(); end

  # Disables SMTP/TLS for this object. Must be called before the connection is
  # established to have any effect.
  #
  # Also aliased as:
  # [`disable_ssl`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-disable_ssl)
  def disable_tls(); end

  def ehlo(domain); end

  # Alias for:
  # [`enable_tls`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-enable_tls)
  def enable_ssl(context=T.unsafe(nil)); end

  # Enables SMTP/TLS (STARTTLS) for this object. `context` is a
  # [`OpenSSL::SSL::SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
  # object.
  def enable_starttls(context=T.unsafe(nil)); end

  # Enables SMTP/TLS (STARTTLS) for this object if server accepts. `context` is
  # a
  # [`OpenSSL::SSL::SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
  # object.
  def enable_starttls_auto(context=T.unsafe(nil)); end

  # Enables SMTP/TLS (SMTPS:
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) over direct TLS
  # connection) for this object. Must be called before the connection is
  # established to have any effect. `context` is a
  # [`OpenSSL::SSL::SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
  # object.
  #
  # Also aliased as:
  # [`enable_ssl`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-enable_ssl)
  def enable_tls(context=T.unsafe(nil)); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) whether to use ESMTP
  # or not. This should be done before calling
  # [`start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-start).
  # Note that if
  # [`start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-start)
  # is called in ESMTP mode, and the connection fails due to a ProtocolError,
  # the [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) object will
  # automatically switch to plain
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) mode and retry
  # (but not vice versa).
  def esmtp(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) whether to use ESMTP
  # or not. This should be done before calling
  # [`start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-start).
  # Note that if
  # [`start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-start)
  # is called in ESMTP mode, and the connection fails due to a ProtocolError,
  # the [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) object will
  # automatically switch to plain
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) mode and retry
  # (but not vice versa).
  def esmtp=(esmtp); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) whether to use ESMTP
  # or not. This should be done before calling
  # [`start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-start).
  # Note that if
  # [`start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-start)
  # is called in ESMTP mode, and the connection fails due to a ProtocolError,
  # the [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) object will
  # automatically switch to plain
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) mode and retry
  # (but not vice versa).
  def esmtp?(); end

  # Finishes the [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html)
  # session and closes TCP connection. Raises
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) if not
  # started.
  def finish(); end

  def helo(domain); end

  def initialize(address, port=T.unsafe(nil)); end

  # Provide human-readable stringification of class state.
  def inspect(); end

  def mailfrom(from_addr); end

  # Opens a message writer stream and gives it to the block. The stream is valid
  # only in the block, and has these methods:
  #
  # puts(str = '')
  # :   outputs STR and CR LF.
  # print(str)
  # :   outputs STR.
  # printf(fmt, \*args)
  # :   outputs sprintf(fmt,\*args).
  # write(str)
  # :   outputs STR and returns the length of written bytes.
  # <<(str)
  # :   outputs STR and returns self.
  #
  #
  # If a single CR ("r") or LF ("n") is found in the message, it is converted to
  # the CR LF pair. You cannot send a binary message with this method.
  #
  # ### Parameters
  #
  # `from_addr` is a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # representing the source mail address.
  #
  # `to_addr` is a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # or Strings or [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # Strings, representing the destination mail address or addresses.
  #
  # ### Example
  #
  # ```ruby
  # Net::SMTP.start('smtp.example.com', 25) do |smtp|
  #   smtp.open_message_stream('from@example.com', ['dest@example.com']) do |f|
  #     f.puts 'From: from@example.com'
  #     f.puts 'To: dest@example.com'
  #     f.puts 'Subject: test message'
  #     f.puts
  #     f.puts 'This is a test message.'
  #   end
  # end
  # ```
  #
  # ### Errors
  #
  # This method may raise:
  #
  # *   [`Net::SMTPServerBusy`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPServerBusy.html)
  # *   [`Net::SMTPSyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPSyntaxError.html)
  # *   [`Net::SMTPFatalError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPFatalError.html)
  # *   [`Net::SMTPUnknownError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPUnknownError.html)
  # *   [`Net::ReadTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/ReadTimeout.html)
  # *   [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html)
  #
  #
  # Also aliased as:
  # [`ready`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-ready)
  def open_message_stream(from_addr, *to_addrs, &block); end

  # Seconds to wait while attempting to open a connection. If the connection
  # cannot be opened within this time, a
  # [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # is raised. The default value is 30 seconds.
  def open_timeout(); end

  # Seconds to wait while attempting to open a connection. If the connection
  # cannot be opened within this time, a
  # [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # is raised. The default value is 30 seconds.
  def open_timeout=(open_timeout); end

  # The port number of the
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) server to
  # connect to.
  def port(); end

  def quit(); end

  def rcptto(to_addr); end

  def rcptto_list(to_addrs); end

  # Seconds to wait while reading one block (by one read(2) call). If the
  # read(2) call does not complete within this time, a
  # [`Net::ReadTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/ReadTimeout.html)
  # is raised. The default value is 60 seconds.
  def read_timeout(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the number of seconds
  # to wait until timing-out a read(2) call.
  def read_timeout=(sec); end

  # Alias for:
  # [`open_message_stream`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-open_message_stream)
  def ready(from_addr, *to_addrs, &block); end

  # Aborts the current mail transaction
  def rset(); end

  # Alias for:
  # [`send_message`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-send_message)
  def send_mail(msgstr, from_addr, *to_addrs); end

  # Sends `msgstr` as a message. Single CR ("r") and LF ("n") found in the
  # `msgstr`, are converted into the CR LF pair. You cannot send a binary
  # message with this method. `msgstr` should include both the message headers
  # and body.
  #
  # `from_addr` is a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # representing the source mail address.
  #
  # `to_addr` is a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # or Strings or [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # Strings, representing the destination mail address or addresses.
  #
  # ### Example
  #
  # ```ruby
  # Net::SMTP.start('smtp.example.com') do |smtp|
  #   smtp.send_message msgstr,
  #                     'from@example.com',
  #                     ['dest@example.com', 'dest2@example.com']
  # end
  # ```
  #
  # ### Errors
  #
  # This method may raise:
  #
  # *   [`Net::SMTPServerBusy`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPServerBusy.html)
  # *   [`Net::SMTPSyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPSyntaxError.html)
  # *   [`Net::SMTPFatalError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPFatalError.html)
  # *   [`Net::SMTPUnknownError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPUnknownError.html)
  # *   [`Net::ReadTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/ReadTimeout.html)
  # *   [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html)
  #
  #
  # Also aliased as:
  # [`send_mail`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-send_mail),
  # [`sendmail`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-sendmail)
  def send_message(msgstr, from_addr, *to_addrs); end

  # Alias for:
  # [`send_message`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-send_message)
  def sendmail(msgstr, from_addr, *to_addrs); end

  # Alias for:
  # [`debug_output=`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-debug_output-3D)
  def set_debug_output(arg); end

  # Alias for:
  # [`tls?`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-tls-3F)
  def ssl?(); end

  # Opens a TCP connection and starts the
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) session.
  #
  # ### Parameters
  #
  # `helo` is the *HELO* *domain* that you'll dispatch mails from; see the
  # discussion in the overview notes.
  #
  # If both of `user` and `secret` are given,
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) authentication
  # will be attempted using the AUTH command. `authtype` specifies the type of
  # authentication to attempt; it must be one of :login, :plain, and :cram\_md5.
  # See the notes on [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html)
  # Authentication in the overview.
  #
  # ### Block Usage
  #
  # When this methods is called with a block, the newly-started
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) object is
  # yielded to the block, and automatically closed after the block call
  # finishes. Otherwise, it is the caller's responsibility to close the session
  # when finished.
  #
  # ### Example
  #
  # This is very similar to the class method
  # [`SMTP.start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-c-start).
  #
  # ```ruby
  # require 'net/smtp'
  # smtp = Net::SMTP.new('smtp.mail.server', 25)
  # smtp.start(helo_domain, account, password, authtype) do |smtp|
  #   smtp.send_message msgstr, 'from@example.com', ['dest@example.com']
  # end
  # ```
  #
  # The primary use of this method (as opposed to
  # [`SMTP.start`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-c-start))
  # is probably to set debugging
  # ([`set_debug_output`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-set_debug_output))
  # or ESMTP
  # ([`esmtp=`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#attribute-i-esmtp)),
  # which must be done before the session is started.
  #
  # ### Errors
  #
  # If session has already been started, an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) will be
  # raised.
  #
  # This method may raise:
  #
  # *   [`Net::SMTPAuthenticationError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPAuthenticationError.html)
  # *   [`Net::SMTPServerBusy`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPServerBusy.html)
  # *   [`Net::SMTPSyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPSyntaxError.html)
  # *   [`Net::SMTPFatalError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPFatalError.html)
  # *   [`Net::SMTPUnknownError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPUnknownError.html)
  # *   [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # *   [`Net::ReadTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/ReadTimeout.html)
  # *   [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html)
  def start(helo=T.unsafe(nil), user=T.unsafe(nil), secret=T.unsafe(nil), authtype=T.unsafe(nil)); end

  # `true` if the [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html)
  # session has been started.
  def started?(); end

  def starttls(); end

  # Returns truth value if this object uses STARTTLS. If this object always uses
  # STARTTLS, returns :always. If this object uses STARTTLS when the server
  # support TLS, returns :auto.
  def starttls?(); end

  # true if this object uses STARTTLS.
  def starttls_always?(); end

  # true if this object uses STARTTLS when server advertises STARTTLS.
  def starttls_auto?(); end

  # true if this object uses SMTP/TLS (SMTPS).
  #
  # Also aliased as:
  # [`ssl?`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-i-ssl-3F)
  def tls?(); end

  # The default [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) port
  # number, 25.
  def self.default_port(); end

  def self.default_ssl_context(); end

  # Alias for:
  # [`default_tls_port`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-c-default_tls_port)
  def self.default_ssl_port(); end

  # The default mail submission port number, 587.
  def self.default_submission_port(); end

  # The default SMTPS port number, 465.
  #
  # Also aliased as:
  # [`default_ssl_port`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html#method-c-default_ssl_port)
  def self.default_tls_port(); end

  # Creates a new
  # [`Net::SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) object and
  # connects to the server.
  #
  # This method is equivalent to:
  #
  # ```ruby
  # Net::SMTP.new(address, port).start(helo_domain, account, password, authtype)
  # ```
  #
  # ### Example
  #
  # ```ruby
  # Net::SMTP.start('your.smtp.server') do |smtp|
  #   smtp.send_message msgstr, 'from@example.com', ['dest@example.com']
  # end
  # ```
  #
  # ### Block Usage
  #
  # If called with a block, the newly-opened
  # [`Net::SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) object is
  # yielded to the block, and automatically closed when the block finishes. If
  # called without a block, the newly-opened
  # [`Net::SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) object is
  # returned to the caller, and it is the caller's responsibility to close it
  # when finished.
  #
  # ### Parameters
  #
  # `address` is the hostname or ip address of your smtp server.
  #
  # `port` is the port to connect to; it defaults to port 25.
  #
  # `helo` is the *HELO* *domain* provided by the client to the server (see
  # overview comments); it defaults to 'localhost'.
  #
  # The remaining arguments are used for
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) authentication,
  # if required or desired. `user` is the account name; `secret` is your
  # password or other authentication token; and `authtype` is the authentication
  # type, one of :plain, :login, or :cram\_md5. See the discussion of
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) Authentication
  # in the overview notes.
  #
  # ### Errors
  #
  # This method may raise:
  #
  # *   [`Net::SMTPAuthenticationError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPAuthenticationError.html)
  # *   [`Net::SMTPServerBusy`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPServerBusy.html)
  # *   [`Net::SMTPSyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPSyntaxError.html)
  # *   [`Net::SMTPFatalError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPFatalError.html)
  # *   [`Net::SMTPUnknownError`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTPUnknownError.html)
  # *   [`Net::OpenTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/OpenTimeout.html)
  # *   [`Net::ReadTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/ReadTimeout.html)
  # *   [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html)
  def self.start(address, port=T.unsafe(nil), helo=T.unsafe(nil), user=T.unsafe(nil), secret=T.unsafe(nil), authtype=T.unsafe(nil), &block); end
end

# This class represents a response received by the
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) server. Instances
# of this class are created by the
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) class; they should
# not be directly created by the user. For more information on
# [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) responses, view
# [Section 4.2 of RFC 5321](http://tools.ietf.org/html/rfc5321#section-4.2)
class Net::SMTP::Response
  # Returns a hash of the human readable reply text in the response if it is
  # multiple lines. It does not return the first line. The key of the hash is
  # the first word the value of the hash is an array with each word thereafter
  # being a value in the array
  def capabilities(); end

  # Determines whether the response received was a Positive Intermediate reply
  # (3xx reply code)
  def continue?(); end

  # Creates a CRAM-MD5 challenge. You can view more information on CRAM-MD5 on
  # Wikipedia: https://en.wikipedia.org/wiki/CRAM-MD5
  def cram_md5_challenge(); end

  # Determines whether there was an error and raises the appropriate error based
  # on the reply code of the response
  def exception_class(); end

  def initialize(status, string); end

  # The first line of the human readable reply text
  def message(); end

  # The three digit reply code of the
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) response
  def status(); end

  # Takes the first digit of the reply code to determine the status type
  def status_type_char(); end

  # The human readable reply text of the
  # [`SMTP`](https://docs.ruby-lang.org/en/2.7.0/Net/SMTP.html) response
  def string(); end

  # Determines whether the response received was a Positive Completion reply
  # (2xx reply code)
  def success?(); end

  # Parses the received response and separates the reply code and the human
  # readable reply text
  def self.parse(str); end
end

# Represents an SMTP authentication error.
class Net::SMTPAuthenticationError < Net::ProtoAuthError
  include ::Net::SMTPError
end

# [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) mixed in to all
# SMTP error classes
module Net::SMTPError
end

# Represents a fatal SMTP error (error code 5xx, except for 500)
class Net::SMTPFatalError < Net::ProtoFatalError
  include ::Net::SMTPError
end

# Represents SMTP error code 4xx, a temporary error.
class Net::SMTPServerBusy < Net::ProtoServerError
  include ::Net::SMTPError
end

# Represents an SMTP command syntax error (error code 500)
class Net::SMTPSyntaxError < Net::ProtoSyntaxError
  include ::Net::SMTPError
end

# Unexpected reply code returned from server.
class Net::SMTPUnknownError < Net::ProtoUnknownError
  include ::Net::SMTPError
end

# Command is not supported on server.
class Net::SMTPUnsupportedCommand < Net::ProtocolError
  include ::Net::SMTPError
end

# The writer adapter class
class Net::WriteAdapter
  def <<(str); end

  def initialize(socket, method); end

  def inspect(); end

  # Alias for:
  # [`write`](https://docs.ruby-lang.org/en/2.7.0/Net/WriteAdapter.html#method-i-write)
  def print(str); end

  def printf(*args); end

  def puts(str=T.unsafe(nil)); end

  # Also aliased as:
  # [`print`](https://docs.ruby-lang.org/en/2.7.0/Net/WriteAdapter.html#method-i-print)
  def write(str); end
end

# [`WriteTimeout`](https://docs.ruby-lang.org/en/2.7.0/Net/WriteTimeout.html), a
# subclass of
# [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html), is
# raised if a chunk of the response cannot be written within the write\_timeout.
# Not raised on Windows.
class Net::WriteTimeout < ::Timeout::Error; end
