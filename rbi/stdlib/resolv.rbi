# typed: __STDLIB_INTERNAL

# [`Resolv`](https://docs.ruby-lang.org/en/2.7.0/Resolv.html) is a thread-aware
# DNS resolver library written in Ruby.
# [`Resolv`](https://docs.ruby-lang.org/en/2.7.0/Resolv.html) can handle
# multiple DNS requests concurrently without blocking the entire Ruby
# interpreter.
#
# See also resolv-replace.rb to replace the libc resolver with
# [`Resolv`](https://docs.ruby-lang.org/en/2.7.0/Resolv.html).
#
# [`Resolv`](https://docs.ruby-lang.org/en/2.7.0/Resolv.html) can look up
# various DNS resources using the DNS module directly.
#
# Examples:
#
# ```ruby
# p Resolv.getaddress "www.ruby-lang.org"
# p Resolv.getname "210.251.121.214"
#
# Resolv::DNS.open do |dns|
#   ress = dns.getresources "www.ruby-lang.org", Resolv::DNS::Resource::IN::A
#   p ress.map(&:address)
#   ress = dns.getresources "ruby-lang.org", Resolv::DNS::Resource::IN::MX
#   p ress.map { |r| [r.exchange.to_s, r.preference] }
# end
# ```
#
# ## Bugs
#
# *   NIS is not supported.
# *   /etc/nsswitch.conf is not supported.
class Resolv
  # Address [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) to use
  # for matching IP addresses.
  AddressRegex = T.let(T.unsafe(nil), Regexp)

  # Default resolver to use for
  # [`Resolv`](https://docs.ruby-lang.org/en/2.7.0/Resolv.html) class methods.
  DefaultResolver = T.let(T.unsafe(nil), Resolv)

  # Creates a new [`Resolv`](https://docs.ruby-lang.org/en/2.6.0/Resolv.html)
  # using `resolvers`.
  sig { params(resolvers: [Hosts, DNS]).void }
  def initialize(resolvers=[Hosts.new, DNS.new]); end

  # Iterates over all IP addresses for `name`.
  sig { params(name: String, block: T.proc.params(address: String).void).void }
  def each_address(name, &block); end

  # Iterates over all hostnames for `address`.
  sig { params(address: String, block: T.proc.params(name: String).void).void }
  def each_name(address, &block); end

  # Looks up the first IP address for `name`.
  sig { params(name: String).returns(String) }
  def getaddress(name); end

  # Looks up all IP address for `name`.
  sig { params(name: String).returns(T::Array[String]) }
  def getaddresses(name); end

  # Looks up the hostname of `address`.
  sig { params(address: String).returns(String) }
  def getname(address); end

  # Looks up all hostnames for `address`.
  sig { params(address: String).returns(T::Array[String]) }
  def getnames(address); end

  # Iterates over all IP addresses for `name`.
  sig { params(name: String, block: T.proc.params(address: String).void).void }
  def self.each_address(name, &block); end

  # Iterates over all hostnames for `address`.
  sig { params(address: String, block: T.proc.params(name: String).void).void }
  def self.each_name(address, &block); end

  # Looks up the first IP address for `name`.
  sig { params(name: String).returns(String) }
  def self.getaddress(name); end

  # Looks up all IP address for `name`.
  sig { params(name: String).returns(T::Array[String]) }
  def self.getaddresses(name); end

  # Looks up the hostname of `address`.
  sig { params(address: String).returns(String) }
  def self.getname(address); end

  # Looks up all hostnames for `address`.
  sig { params(address: String).returns(T::Array[String]) }
  def self.getnames(address); end
end

# [`Resolv::DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) is a
# [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) stub resolver.
#
# Information taken from the following places:
#
# *   STD0013
# *   RFC 1035
# *   ftp://ftp.isi.edu/in-notes/iana/assignments/dns-parameters
# *   etc.
class Resolv::DNS
  # Default [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html)
  # [`Port`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html#Port)
  Port = T.let(T.unsafe(nil), Integer)

  # Default [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) UDP
  # packet size
  UDPSize = T.let(T.unsafe(nil), Integer)

  # Creates a new [`DNS`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS.html)
  # resolver.
  #
  # `config_info` can be:
  #
  # nil
  # :   Uses /etc/resolv.conf.
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # :   Path to a file using /etc/resolv.conf's format.
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)
  # :   Must contain :nameserver, :search and :ndots keys.
  #
  # :nameserver\_port can be used to specify port number of nameserver address.
  #
  # The value of :nameserver should be an address string or an array of address
  # strings.
  # *   :nameserver => '8.8.8.8'
  # *   :nameserver => ['8.8.8.8', '8.8.4.4']
  #
  #
  # The value of :nameserver\_port should be an array of pair of nameserver
  # address and port number.
  # *   :nameserver\_port => [['8.8.8.8', 53], ['8.8.4.4', 53]]
  #
  #
  # Example:
  #
  # ```ruby
  # Resolv::DNS.new(:nameserver => ['210.251.121.21'],
  #                 :search => ['ruby-lang.org'],
  #                 :ndots => 1)
  # ```
  sig do
    params(
      config_info: T.any(
        NilClass,
        String,
        { nameserver: T.any(String, T::Array[String]), search: T::Array[String], ndots: Integer },
        { nameserver_port: T::Array[[String, Integer]], search: T::Array[String], ndots: Integer }
      )
    ).void
  end
  def initialize(config_info = nil); end

  # Closes the [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html)
  # resolver.
  sig { void }
  def close; end

  # Iterates over all IP addresses for `name` retrieved from the
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resolver.
  #
  # `name` can be a
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html)
  # or a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Retrieved
  # addresses will be a
  # [`Resolv::IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html) or
  # [`Resolv::IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  sig do
    params(
      name: T.any(String, Resolv::DNS::Name),
      block: T.proc.params(address: T.any(Resolv::IPv4, Resolv::IPv6)).void
    ).void
  end
  def each_address(name, &block); end

  # Iterates over all hostnames for `address` retrieved from the
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resolver.
  #
  # `address` must be a
  # [`Resolv::IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html),
  # [`Resolv::IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html) or a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Retrieved names
  # will be
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html)
  # instances.
  sig do
    params(
      address: T.any(String, Resolv::IPv4, Resolv::IPv6),
      block: T.proc.params(name: Resolv::DNS::Name).void)
    .void
  end
  def each_name(address, &block); end

  # Iterates over all `typeclass`
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resources for
  # `name`. See
  # [`getresource`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html#method-i-getresource)
  # for argument details.
  sig do
    params(
      name: T.any(String, Resolv::DNS::Name),
      typeclass: T.class_of(Resolv::DNS::Resource),
      block: T.proc.params(resource: Resolv::DNS::Resource).void
    ).void
  end
  def each_resource(name, typeclass, &block); end

  def fetch_resource(name, typeclass, &blk); end

  # Gets the IP address of `name` from the
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resolver.
  #
  # `name` can be a
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html)
  # or a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Retrieved
  # address will be a
  # [`Resolv::IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html) or
  # [`Resolv::IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  sig do
    params(name: T.any(String, Resolv::DNS::Name))
    .returns(T.any(Resolv::IPv4, Resolv::IPv6))
  end
  def getaddress(name); end

  # Gets all IP addresses for `name` from the
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resolver.
  #
  # `name` can be a
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html)
  # or a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Retrieved
  # addresses will be a
  # [`Resolv::IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html) or
  # [`Resolv::IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  sig do
    params(name: T.any(String, Resolv::DNS::Name))
    .returns(T::Array[T.any(Resolv::IPv4, Resolv::IPv6)])
  end
  def getaddresses(name); end

  # Gets the hostname for `address` from the
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resolver.
  #
  # `address` must be a
  # [`Resolv::IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html),
  # [`Resolv::IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html) or a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Retrieved name
  # will be a
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html).
  sig do
    params(address: T.any(String, Resolv::IPv4, Resolv::IPv6))
    .returns(Resolv::DNS::Name)
  end
  def getname(address); end

  # Gets all hostnames for `address` from the
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resolver.
  #
  # `address` must be a
  # [`Resolv::IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html),
  # [`Resolv::IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html) or a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Retrieved names
  # will be
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html)
  # instances.
  sig do
    params(address: T.any(String, Resolv::IPv4, Resolv::IPv6))
    .returns(T::Array[Resolv::DNS::Name])
  end
  def getnames(address); end

  # Look up the `typeclass`
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resource of
  # `name`.
  #
  # `name` must be a
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html)
  # or a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # `typeclass` should be one of the following:
  #
  # *   [`Resolv::DNS::Resource::IN::A`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/IN/A.html)
  # *   [`Resolv::DNS::Resource::IN::AAAA`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/IN/AAAA.html)
  # *   Resolv::DNS::Resource::IN::ANY
  # *   Resolv::DNS::Resource::IN::CNAME
  # *   Resolv::DNS::Resource::IN::HINFO
  # *   Resolv::DNS::Resource::IN::MINFO
  # *   Resolv::DNS::Resource::IN::MX
  # *   Resolv::DNS::Resource::IN::NS
  # *   Resolv::DNS::Resource::IN::PTR
  # *   Resolv::DNS::Resource::IN::SOA
  # *   Resolv::DNS::Resource::IN::TXT
  # *   [`Resolv::DNS::Resource::IN::WKS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/IN/WKS.html)
  #
  #
  # Returned resource is represented as a
  # [`Resolv::DNS::Resource`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource.html)
  # instance, i.e.
  # [`Resolv::DNS::Resource::IN::A`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/IN/A.html).
  sig do
    params(
      name: T.any(String, Resolv::DNS::Name),
      typeclass: T.class_of(Resolv::DNS::Resource)
    ).returns(Resolv::DNS::Resource)
  end
  def getresource(name, typeclass); end

  # Looks up all `typeclass`
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resources for
  # `name`. See
  # [`getresource`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html#method-i-getresource)
  # for argument details.
  sig do
    params(
      name: T.any(String, Resolv::DNS::Name),
      typeclass: T.class_of(Resolv::DNS::Resource)
    ).returns(T::Array[Resolv::DNS::Resource])
  end
  def getresources(name, typeclass); end

  def lazy_initialize; end

  def make_tcp_requester(host, port); end

  def make_udp_requester; end

  # Sets the resolver timeouts. This may be a single positive number or an array
  # of positive numbers representing timeouts in seconds. If an array is
  # specified, a [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html)
  # request will retry and wait for each successive interval in the array until
  # a successful response is received. Specifying `nil` reverts to the default
  # timeouts:
  # 5, second = 5 \* 2 / nameserver\_count, 2 \* second, 4 \* second
  # :   Example:
  #
  # ```ruby
  # dns.timeouts = 3
  # ```
  sig { params(values: T.any(NilClass, Numeric, T::Array[Numeric])).void }
  def timeouts=(values); end

  # Creates a new [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html)
  # resolver. See
  # [`Resolv::DNS.new`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html#method-c-new)
  # for argument details.
  #
  # Yields the created
  # [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resolver to the
  # block, if given, otherwise returns it.
  sig do
    params(
      config_info: T.any(
        NilClass,
        String,
        { nameserver: T.any(String, T::Array[String]), search: T::Array[String], ndots: Integer },
        { nameserver_port: T::Array[[String, Integer]], search: T::Array[String], ndots: Integer }
      ),
      blk: T.nilable(T.proc.params(arg0: Resolv::DNS).void),
    ).returns(Resolv::DNS)
  end
  def self.open(config_info = nil, &blk); end
end

class Resolv::DNS::Config
  InitialTimeout = T.let(T.unsafe(nil), Integer)

  def self.new(config_info = _); end

  def generate_candidates(name); end

  def generate_timeouts; end

  def lazy_initialize; end

  def nameserver_port; end

  def resolv(name); end

  def single?; end

  def timeouts=(values); end

  def self.default_config_hash(filename = _); end

  def self.parse_resolv_conf(filename); end
end

# Indicates no such domain was found.
class Resolv::DNS::Config::NXDomain < ::Resolv::ResolvError; end

# Indicates some other unhandled resolver error was encountered.
class Resolv::DNS::Config::OtherResolvError < ::Resolv::ResolvError; end

# Indicates that the
# [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) response was
# unable to be decoded.
class Resolv::DNS::DecodeError < ::StandardError; end

# Indicates that the
# [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) request was
# unable to be encoded.
class Resolv::DNS::EncodeError < ::StandardError; end

module Resolv::DNS::Label
  def self.split(arg); end
end

class Resolv::DNS::Label::Str
  def self.new(string); end

  def ==(other); end

  def downcase; end

  def eql?(other); end

  def hash; end

  def inspect; end

  def string; end

  def to_s; end
end

class Resolv::DNS::Message
  def self.new(id = _); end

  def ==(other); end

  def aa; end

  def aa=(_); end

  def add_additional(name, ttl, data); end

  def add_answer(name, ttl, data); end

  def add_authority(name, ttl, data); end

  def add_question(name, typeclass); end

  def additional; end

  def answer; end

  def authority; end

  def each_additional; end

  def each_answer; end

  def each_authority; end

  def each_question; end

  def each_resource; end

  def encode; end

  def id; end

  def id=(_); end

  def opcode; end

  def opcode=(_); end

  def qr; end

  def qr=(_); end

  def question; end

  def ra; end

  def ra=(_); end

  def rcode; end

  def rcode=(_); end

  def rd; end

  def rd=(_); end

  def tc; end

  def tc=(_); end

  def self.decode(m); end
end

class Resolv::DNS::Message::MessageDecoder
  def self.new(data); end

  def get_bytes(len = _); end

  def get_label; end

  def get_labels; end

  def get_length16; end

  def get_name; end

  def get_question; end

  def get_rr; end

  def get_string; end

  def get_string_list; end

  def get_unpack(template); end

  def inspect; end
end

class Resolv::DNS::Message::MessageEncoder
  def self.new; end

  def put_bytes(d); end

  def put_label(d); end

  def put_labels(d); end

  def put_length16; end

  def put_name(d); end

  def put_pack(template, *d); end

  def put_string(d); end

  def put_string_list(ds); end

  def to_s; end
end

# A representation of a
# [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) name.
class Resolv::DNS::Name
  sig { params(labels: T::Array[String], absolute: T.any(FalseClass, TrueClass)).void }
  def initialize(labels, absolute=true); end

  def ==(other); end

  def [](i); end

  # True if this name is absolute.
  sig { returns(T.any(FalseClass, TrueClass)) }
  def absolute?; end

  def eql?(other); end

  def hash; end

  def inspect; end

  def length; end

  # Returns true if `other` is a subdomain.
  #
  # Example:
  #
  # ```ruby
  # domain = Resolv::DNS::Name.create("y.z")
  # p Resolv::DNS::Name.create("w.x.y.z").subdomain_of?(domain) #=> true
  # p Resolv::DNS::Name.create("x.y.z").subdomain_of?(domain) #=> true
  # p Resolv::DNS::Name.create("y.z").subdomain_of?(domain) #=> false
  # p Resolv::DNS::Name.create("z").subdomain_of?(domain) #=> false
  # p Resolv::DNS::Name.create("x.y.z.").subdomain_of?(domain) #=> false
  # p Resolv::DNS::Name.create("w.z").subdomain_of?(domain) #=> false
  # ```
  sig { params(other: Resolv::DNS::Name).returns(T.any(FalseClass, TrueClass)) }
  def subdomain_of?(other); end

  def to_a; end

  # returns the domain name as a string.
  #
  # The domain name doesn't have a trailing dot even if the name object is
  # absolute.
  #
  # Example:
  #
  # ```ruby
  # p Resolv::DNS::Name.create("x.y.z.").to_s #=> "x.y.z"
  # p Resolv::DNS::Name.create("x.y.z").to_s #=> "x.y.z"
  # ```
  def to_s; end

  # Creates a new [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html)
  # name from `arg`. `arg` can be:
  #
  # [`Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html)
  # :   returns `arg`.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # :   Creates a new
  #     [`Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html).
  sig { params(arg: T.any(String, Resolv::DNS::Name)).returns(Resolv::DNS::Name) }
  def self.create(arg); end
end

module Resolv::DNS::OpCode
  IQuery = T.let(T.unsafe(nil), Integer)

  Notify = T.let(T.unsafe(nil), Integer)

  Query = T.let(T.unsafe(nil), Integer)

  Status = T.let(T.unsafe(nil), Integer)

  Update = T.let(T.unsafe(nil), Integer)
end

# A [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) query abstract
# class.
class Resolv::DNS::Query
  def encode_rdata(msg); end

  def self.decode_rdata(msg); end
end

module Resolv::DNS::RCode
  BADALG = T.let(T.unsafe(nil), Integer)

  BADKEY = T.let(T.unsafe(nil), Integer)

  BADMODE = T.let(T.unsafe(nil), Integer)

  BADNAME = T.let(T.unsafe(nil), Integer)

  BADSIG = T.let(T.unsafe(nil), Integer)

  BADTIME = T.let(T.unsafe(nil), Integer)

  BADVERS = T.let(T.unsafe(nil), Integer)

  FormErr = T.let(T.unsafe(nil), Integer)

  NXDomain = T.let(T.unsafe(nil), Integer)

  NXRRSet = T.let(T.unsafe(nil), Integer)

  NoError = T.let(T.unsafe(nil), Integer)

  NotAuth = T.let(T.unsafe(nil), Integer)

  NotImp = T.let(T.unsafe(nil), Integer)

  NotZone = T.let(T.unsafe(nil), Integer)

  Refused = T.let(T.unsafe(nil), Integer)

  ServFail = T.let(T.unsafe(nil), Integer)

  YXDomain = T.let(T.unsafe(nil), Integer)

  YXRRSet = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Requester
  def self.new; end

  def close; end

  def request(sender, tout); end

  def sender_for(addr, msg); end
end

class Resolv::DNS::Requester::ConnectedUDP < ::Resolv::DNS::Requester
  def self.new(host, port = _); end

  def close; end

  def recv_reply(readable_socks); end

  def sender(msg, data, host = _, port = _); end
end

class Resolv::DNS::Requester::ConnectedUDP::Sender < ::Resolv::DNS::Requester::Sender
  def data; end

  def send; end
end

class Resolv::DNS::Requester::MDNSOneShot < ::Resolv::DNS::Requester::UnconnectedUDP
  def sender(msg, data, host, port = _); end

  def sender_for(addr, msg); end
end

# Indicates a problem with the
# [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) request.
class Resolv::DNS::Requester::RequestError < ::StandardError; end

class Resolv::DNS::Requester::Sender
  def self.new(msg, data, sock); end
end

class Resolv::DNS::Requester::TCP < ::Resolv::DNS::Requester
  def self.new(host, port = _); end

  def close; end

  def recv_reply(readable_socks); end

  def sender(msg, data, host = _, port = _); end
end

class Resolv::DNS::Requester::TCP::Sender < ::Resolv::DNS::Requester::Sender
  def data; end

  def send; end
end

class Resolv::DNS::Requester::UnconnectedUDP < ::Resolv::DNS::Requester
  def self.new(*nameserver_port); end

  def close; end

  def recv_reply(readable_socks); end

  def sender(msg, data, host, port = _); end
end

class Resolv::DNS::Requester::UnconnectedUDP::Sender < ::Resolv::DNS::Requester::Sender
  def self.new(msg, data, sock, host, port); end

  def data; end

  def send; end
end

# A [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) resource
# abstract class.
class Resolv::DNS::Resource < ::Resolv::DNS::Query
  ClassInsensitiveTypes = T.let(T.unsafe(nil), T::Array[T.untyped])

  def ==(other); end

  def encode_rdata(msg); end

  def eql?(other); end

  def hash; end

  # Remaining [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) To Live
  # for this
  # [`Resource`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource.html).
  sig { returns(T.nilable(Integer)) }
  def ttl; end

  def self.decode_rdata(msg); end

  def self.get_class(type_value, class_value); end
end

module Resolv::DNS::Resource::ClassHash
  module_function def []=(type_class_value, klass); end
end

# A Query type requesting any RR.
class Resolv::DNS::Resource::ANY < ::Resolv::DNS::Query
  TypeValue = T.let(T.unsafe(nil), Integer)
end

# The canonical name for an alias.
class Resolv::DNS::Resource::CNAME < ::Resolv::DNS::Resource::DomainName
  TypeValue = T.let(T.unsafe(nil), Integer)
end

# Domain Name resource abstract class.
class Resolv::DNS::Resource::DomainName < ::Resolv::DNS::Resource
  # Creates a new
  # [`DomainName`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/DomainName.html)
  # from `name`.
  sig { params(name: T.any(String, Resolv::DNS::Name)).void }
  def initialize(name); end

  def encode_rdata(msg); end

  # The name of this
  # [`DomainName`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/DomainName.html).
  sig { returns(T.any(String, Resolv::DNS::Name)) }
  def name; end

  def self.decode_rdata(msg); end
end

# A generic resource abstract class.
class Resolv::DNS::Resource::Generic < ::Resolv::DNS::Resource
  # Creates a new generic resource.
  sig { params(data: T.untyped).void }
  def initialize(data); end

  # [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) for this generic
  # resource.
  sig { returns(T.untyped) }
  def data; end

  def encode_rdata(msg); end

  def self.create(type_value, class_value); end

  def self.decode_rdata(msg); end
end

# Host Information resource.
class Resolv::DNS::Resource::HINFO < ::Resolv::DNS::Resource
  TypeValue = T.let(T.unsafe(nil), Integer)

  # Creates a new
  # [`HINFO`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/HINFO.html)
  # running `os` on `cpu`.
  sig { params(cpu: String, os: String).void }
  def initialize(cpu, os); end

  # CPU architecture for this resource.
  sig { returns(String) }
  def cpu; end

  def encode_rdata(msg); end

  # Operating system for this resource.
  sig { returns(String) }
  def os; end

  def self.decode_rdata(msg); end
end

# module [`IN`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/IN.html)
# contains ARPA Internet specific RRs.
module Resolv::DNS::Resource::IN
  ClassValue = T.let(T.unsafe(nil), Integer)
end

# IPv4 Address resource
class Resolv::DNS::Resource::IN::A < ::Resolv::DNS::Resource
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)

  # Creates a new
  # [`A`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/IN/A.html) for
  # `address`.
  sig { params(address: String).void }
  def initialize(address); end

  # The [`Resolv::IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html)
  # address for this
  # [`A`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/IN/A.html).
  sig { returns(Resolv::IPv4) }
  def address; end

  def encode_rdata(msg); end

  def self.decode_rdata(msg); end
end

# An IPv6 address record.
class Resolv::DNS::Resource::IN::AAAA < ::Resolv::DNS::Resource
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)

  # Creates a new
  # [`AAAA`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/IN/AAAA.html)
  # for `address`.
  sig { params(address: String).void }
  def initialize(address); end

  # The [`Resolv::IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  # address for this
  # [`AAAA`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/IN/AAAA.html).
  sig { returns(Resolv::IPv6) }
  def address; end

  def encode_rdata(msg); end

  def self.decode_rdata(msg); end
end

class Resolv::DNS::Resource::IN::ANY < ::Resolv::DNS::Resource::ANY
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Resource::IN::CNAME < ::Resolv::DNS::Resource::CNAME
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Resource::IN::HINFO < ::Resolv::DNS::Resource::HINFO
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Resource::IN::LOC < ::Resolv::DNS::Resource::LOC
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Resource::IN::MINFO < ::Resolv::DNS::Resource::MINFO
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Resource::IN::MX < ::Resolv::DNS::Resource::MX
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Resource::IN::NS < ::Resolv::DNS::Resource::NS
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Resource::IN::PTR < ::Resolv::DNS::Resource::PTR
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

class Resolv::DNS::Resource::IN::SOA < ::Resolv::DNS::Resource::SOA
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

# [`SRV`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/IN/SRV.html)
# resource record defined in RFC 2782
#
# These records identify the hostname and port that a service is available at.
class Resolv::DNS::Resource::IN::SRV < ::Resolv::DNS::Resource
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)

  # Create a
  # [`SRV`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/IN/SRV.html)
  # resource record.
  #
  # See the documentation for
  # [`priority`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/IN/SRV.html#attribute-i-priority),
  # [`weight`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/IN/SRV.html#attribute-i-weight),
  # [`port`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/IN/SRV.html#attribute-i-port)
  # and
  # [`target`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/IN/SRV.html#attribute-i-target)
  # for `priority`, `weight`, +port and `target` respectively.
  sig do
    params(
      priority: T.any(Integer, String),
      weight: T.any(Integer, String),
      port: T.any(Integer, String),
      target: T.any(String, Resolv::DNS::Name)
    ).void
  end
  def initialize(priority, weight, port, target); end

  def encode_rdata(msg); end

  # The port on this target host of this service.
  #
  # The range is 0-65535.
  sig { returns(Integer) }
  def port; end

  # The priority of this target host.
  #
  # A client MUST attempt to contact the target host with the lowest-numbered
  # priority it can reach; target hosts with the same priority SHOULD be tried
  # in an order defined by the weight field. The range is 0-65535. Note that it
  # is not widely implemented and should be set to zero.
  sig { returns(Integer) }
  def priority; end

  # The domain name of the target host.
  #
  # A target of "." means that the service is decidedly not available at this
  # domain.
  sig { returns(Resolv::DNS::Name) }
  def target; end

  # A server selection mechanism.
  #
  # The weight field specifies a relative weight for entries with the same
  # priority. Larger weights SHOULD be given a proportionately higher
  # probability of being selected. The range of this number is 0-65535. Domain
  # administrators SHOULD use Weight 0 when there isn't any server selection to
  # do, to make the RR easier to read for humans (less noisy). Note that it is
  # not widely implemented and should be set to zero.
  sig { returns(Integer) }
  def weight; end

  def self.decode_rdata(msg); end
end

class Resolv::DNS::Resource::IN::TXT < ::Resolv::DNS::Resource::TXT
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)
end

# Well Known Service resource.
class Resolv::DNS::Resource::IN::WKS < ::Resolv::DNS::Resource
  ClassValue = T.let(T.unsafe(nil), Integer)

  TypeValue = T.let(T.unsafe(nil), Integer)

  sig { params(address: String, protocol: Integer, bitmap: String).void }
  def initialize(address, protocol, bitmap); end

  # The host these services run on.
  sig { returns(Resolv::IPv4) }
  def address; end

  # A bit map of enabled services on this host.
  #
  # If protocol is 6 (TCP) then the 26th bit corresponds to the SMTP service
  # (port 25). If this bit is set, then an SMTP server should be listening on
  # TCP port 25; if zero, SMTP service is not supported.
  sig { returns(String) }
  def bitmap; end

  def encode_rdata(msg); end

  # IP protocol number for these services.
  sig { returns(Integer) }
  def protocol; end

  def self.decode_rdata(msg); end
end

# Location resource
class Resolv::DNS::Resource::LOC < ::Resolv::DNS::Resource
  TypeValue = T.let(T.unsafe(nil), Integer)

  def self.new(version, ssize, hprecision, vprecision, latitude, longitude, altitude); end

  # The altitude of the
  # [`LOC`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/LOC.html)
  # above a reference sphere whose surface sits 100km below the WGS84 spheroid
  # in centimeters as an unsigned 32bit integer
  def altitude; end

  def encode_rdata(msg); end

  # The horizontal precision using ssize type values in meters using scientific
  # notation as 2 integers of XeY for precision use value/2 e.g. 2m = +/-1m
  def hprecision; end

  # The latitude for this
  # [`LOC`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/LOC.html)
  # where 2\*\*31 is the equator in thousandths of an arc second as an unsigned
  # 32bit integer
  def latitude; end

  # The longitude for this
  # [`LOC`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/LOC.html)
  # where 2\*\*31 is the prime meridian in thousandths of an arc second as an
  # unsigned 32bit integer
  def longitude; end

  # The spherical size of this
  # [`LOC`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/LOC.html) in
  # meters using scientific notation as 2 integers of XeY
  def ssize; end

  # Returns the version value for this
  # [`LOC`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/LOC.html)
  # record which should always be 00
  def version; end

  # The vertical precision using ssize type values in meters using scientific
  # notation as 2 integers of XeY for precision use value/2 e.g. 2m = +/-1m
  def vprecision; end

  def self.decode_rdata(msg); end
end

# Mailing list or mailbox information.
class Resolv::DNS::Resource::MINFO < ::Resolv::DNS::Resource
  TypeValue = T.let(T.unsafe(nil), Integer)

  sig { params(rmailbx: String, emailbx: String).void }
  def initialize(rmailbx, emailbx); end

  # Mailbox to use for error messages related to the mail list or mailbox.
  sig { returns(String) }
  def emailbx; end

  def encode_rdata(msg); end

  # Domain name responsible for this mail list or mailbox.
  sig { returns(String) }
  def rmailbx; end

  def self.decode_rdata(msg); end
end

# Mail Exchanger resource.
class Resolv::DNS::Resource::MX < ::Resolv::DNS::Resource
  TypeValue = T.let(T.unsafe(nil), Integer)

  # Creates a new
  # [`MX`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/MX.html)
  # record with `preference`, accepting mail at `exchange`.
  sig { params(preference: Integer, exchange: String).void }
  def initialize(preference, exchange); end

  def encode_rdata(msg); end

  # The host of this
  # [`MX`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/MX.html).
  sig { returns(String) }
  def exchange; end

  # The preference for this
  # [`MX`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/MX.html).
  sig { returns(Integer) }
  def preference; end

  def self.decode_rdata(msg); end
end

# An authoritative name server.
class Resolv::DNS::Resource::NS < ::Resolv::DNS::Resource::DomainName
  TypeValue = T.let(T.unsafe(nil), Integer)
end

# A Pointer to another
# [`DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html) name.
class Resolv::DNS::Resource::PTR < ::Resolv::DNS::Resource::DomainName
  TypeValue = T.let(T.unsafe(nil), Integer)
end

# Start Of Authority resource.
class Resolv::DNS::Resource::SOA < ::Resolv::DNS::Resource
  TypeValue = T.let(T.unsafe(nil), Integer)

  # Creates a new
  # [`SOA`](https://docs.ruby-lang.org/en/2.6.0/Resolv/DNS/Resource/SOA.html)
  # record. See the attr documentation for the details of each argument.
  sig do
    params(
      mname: String,
      rname: String,
      serial: Integer,
      refresh: Integer,
      retry_: Integer,
      expire: Integer,
      minimum: Integer
    ).void
  end
  def initialize(mname, rname, serial, refresh, retry_, expire, minimum); end

  def encode_rdata(msg); end

  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) in seconds that a
  # secondary name server is to use the data before refreshing from the primary
  # name server.
  sig { returns(Integer) }
  def expire; end

  # The minimum number of seconds to be used for TTL values in RRs.
  sig { returns(Integer) }
  def minimum; end

  # Name of the host where the master zone file for this zone resides.
  sig { returns(String) }
  def mname; end

  # How often, in seconds, a secondary name server is to check for updates from
  # the primary name server.
  sig { returns(Integer) }
  def refresh; end

  # How often, in seconds, a secondary name server is to retry after a failure
  # to check for a refresh.
  sig { returns(Integer) }
  def retry; end

  # The person responsible for this domain name.
  sig { returns(String) }
  def rname; end

  # The version number of the zone file.
  sig { returns(Integer) }
  def serial; end

  def self.decode_rdata(msg); end
end

# Unstructured text resource.
class Resolv::DNS::Resource::TXT < ::Resolv::DNS::Resource
  TypeValue = T.let(T.unsafe(nil), Integer)

  sig { params(first_string: String, rest_strings: String).void }
  def initialize(first_string, *rest_strings); end

  # Returns the concatenated string from `strings`.
  sig { returns(String) }
  def data; end

  def encode_rdata(msg); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # Strings for this
  # [`TXT`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Resource/TXT.html)
  # record.
  sig { returns(T::Array[String]) }
  def strings; end

  def self.decode_rdata(msg); end
end

# [`Resolv::Hosts`](https://docs.ruby-lang.org/en/2.7.0/Resolv/Hosts.html) is a
# hostname resolver that uses the system hosts file.
class Resolv::Hosts
  DefaultFileName = T.let(T.unsafe(nil), String)

  # Creates a new
  # [`Resolv::Hosts`](https://docs.ruby-lang.org/en/2.6.0/Resolv/Hosts.html),
  # using `filename` for its data source.
  sig { params(filename: String).void }
  def initialize(filename = DefaultFileName); end

  # Iterates over all IP addresses for `name` retrieved from the hosts file.
  sig { params(name: String, block: T.proc.params(address: String).void).void }
  def each_address(name, &block); end

  # Iterates over all hostnames for `address` retrieved from the hosts file.
  sig { params(address: String, block: T.proc.params(name: String).void).void }
  def each_name(address, &block); end

  # Gets the IP address of `name` from the hosts file.
  sig { params(name: String).returns(String) }
  def getaddress(name); end

  # Gets all IP addresses for `name` from the hosts file.
  sig { params(name: String).returns(T::Array[String]) }
  def getaddresses(name); end

  # Gets the hostname of `address` from the hosts file.
  sig { params(address: String).returns(String) }
  def getname(address); end

  # Gets all hostnames for `address` from the hosts file.
  sig { params(address: String).returns(T::Array[String]) }
  def getnames(address); end

  def lazy_initialize; end
end

# A [`Resolv::DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html)
# [`IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html) address.
class Resolv::IPv4
  Regex = T.let(T.unsafe(nil), Regexp)

  # Regular expression
  # [`IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html) addresses
  # must match.
  Regex256 = T.let(T.unsafe(nil), Regexp)

  sig { params(address: String).void }
  def initialize(address); end

  def ==(other); end

  # The raw [`IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html)
  # address as a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  sig { returns(String) }
  def address; end

  def eql?(other); end

  def hash; end

  def inspect; end

  # Turns this [`IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html)
  # address into a
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html).
  sig { returns(Resolv::DNS::Name) }
  def to_name; end

  def to_s; end

  sig { params(arg: T.any(String, Resolv::IPv4)).returns(Resolv::IPv4) }
  def self.create(arg); end
end

# A [`Resolv::DNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS.html)
# [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html) address.
class Resolv::IPv6
  # A composite [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  # address [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html).
  Regex = T.let(T.unsafe(nil), Regexp)

  # IPv4 mapped [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  # address format a:b:c:d:e:f:w.x.y.z
  Regex_6Hex4Dec = T.let(T.unsafe(nil), Regexp)

  # [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html) address
  # format a:b:c:d:e:f:g:h
  Regex_8Hex = T.let(T.unsafe(nil), Regexp)

  # Compressed [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  # address format a::b
  Regex_CompressedHex = T.let(T.unsafe(nil), Regexp)

  # Compressed IPv4 mapped
  # [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html) address
  # format a::b:w.x.y.z
  Regex_CompressedHex4Dec = T.let(T.unsafe(nil), Regexp)

  sig { params(address: String).void }
  def initialize(address); end

  def ==(other); end

  # The raw [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  # address as a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  sig { returns(String) }
  def address; end

  def eql?(other); end

  def hash; end

  def inspect; end

  # Turns this [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  # address into a
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html).
  sig { returns(Resolv::DNS::Name) }
  def to_name; end

  def to_s; end

  # Creates a new [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  # address from `arg` which may be:
  #
  # [`IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  # :   returns `arg`.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # :   `arg` must match one of the IPv6::Regex\* constants
  sig { params(arg: T.any(String, Resolv::IPv6)).returns(Resolv::IPv6) }
  def self.create(arg); end
end

module Resolv::LOC
  sig do
    params(
      version: String,
      ssize: T.any(String, Resolv::LOC::Size),
      hprecision: T.any(String, Resolv::LOC::Size),
      vprecision: T.any(String, Resolv::LOC::Size),
      latitude: T.any(String, Resolv::LOC::Coord),
      longitude: T.any(String, Resolv::LOC::Coord),
      altitude: T.any(String, Resolv::LOC::Alt)
    ).void
  end
  def initialize(version, ssize, hprecision, vprecision, latitude, longitude, altitude); end

  sig { returns(String) }
  def version; end

  sig { returns(Resolv::LOC::Size) }
  def ssize; end

  sig { returns(Resolv::LOC::Size) }
  def hprecision; end

  sig { returns(Resolv::LOC::Size) }
  def vprecision; end

  sig { returns(Resolv::LOC::Coord) }
  def latitude; end

  sig { returns(Resolv::LOC::Coord) }
  def longitude; end

  sig { returns(Resolv::LOC::Alt) }
  def altitude; end
end

# A
# [`Resolv::LOC::Alt`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Alt.html)
class Resolv::LOC::Alt
  Regex = T.let(T.unsafe(nil), Regexp)

  sig { params(altitude: String).void }
  def initialize(altitude); end

  def ==(other); end

  # The raw altitude
  sig { returns(String) }
  def altitude; end

  def eql?(other); end

  def hash; end

  def inspect; end

  def to_s; end

  # Creates a new
  # [`LOC::Alt`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Alt.html) from
  # `arg` which may be:
  #
  # [`LOC::Alt`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Alt.html)
  # :   returns `arg`.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # :   `arg` must match the LOC::Alt::Regex constant
  sig { params(arg: T.any(String, Resolv::LOC::Alt)).returns(Resolv::LOC::Alt) }
  def self.create(arg); end
end

# A
# [`Resolv::LOC::Coord`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Coord.html)
class Resolv::LOC::Coord
  Regex = T.let(T.unsafe(nil), Regexp)

  sig { params(coordinates: String, orientation: T.deprecated_enum(%w[lat lon])).void }
  def initialize(coordinates, orientation); end

  def ==(other); end

  # The raw coordinates
  sig { returns(String) }
  def coordinates; end

  def eql?(other); end

  def hash; end

  def inspect; end

  # The orientation of the hemisphere as 'lat' or 'lon'
  sig { returns(T.deprecated_enum(%w[lat lon])) }
  def orientation; end

  def to_s; end

  # Creates a new
  # [`LOC::Coord`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Coord.html)
  # from `arg` which may be:
  #
  # [`LOC::Coord`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Coord.html)
  # :   returns `arg`.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # :   `arg` must match the LOC::Coord::Regex constant
  sig { params(arg: T.any(String, Resolv::LOC::Coord)).returns(Resolv::LOC::Coord) }
  def self.create(arg); end
end

# A
# [`Resolv::LOC::Size`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Size.html)
class Resolv::LOC::Size
  Regex = T.let(T.unsafe(nil), Regexp)

  sig { params(scalar: String).void }
  def initialize(scalar); end

  def ==(other); end

  def eql?(other); end

  def hash; end

  def inspect; end

  # The raw size
  sig { returns(String) }
  def scalar; end

  def to_s; end

  # Creates a new
  # [`LOC::Size`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Size.html) from
  # `arg` which may be:
  #
  # [`LOC::Size`](https://docs.ruby-lang.org/en/2.7.0/Resolv/LOC/Size.html)
  # :   returns `arg`.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # :   `arg` must match the LOC::Size::Regex constant
  sig { params(arg: T.any(String, Resolv::LOC::Size)).returns(Resolv::LOC::Size) }
  def self.create(arg); end
end

# [`Resolv::MDNS`](https://docs.ruby-lang.org/en/2.7.0/Resolv/MDNS.html) is a
# one-shot Multicast DNS (mDNS) resolver. It blindly makes queries to the mDNS
# addresses without understanding anything about multicast ports.
#
# Information taken form the following places:
#
# *   RFC 6762
class Resolv::MDNS < ::Resolv::DNS
  # Default IPv4 mDNS address
  AddressV4 = T.let(T.unsafe(nil), String)

  # Default IPv6 mDNS address
  AddressV6 = T.let(T.unsafe(nil), String)

  # Default mDNS addresses
  Addresses = T.let(T.unsafe(nil), [[String, Integer], [String, Integer]])

  # Default mDNS
  # [`Port`](https://docs.ruby-lang.org/en/2.7.0/Resolv/MDNS.html#Port)
  Port = T.let(T.unsafe(nil), Integer)

  # Creates a new one-shot Multicast DNS (mDNS) resolver.
  #
  # `config_info` can be:
  #
  # nil
  # :   Uses the default mDNS addresses
  #
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)
  # :   Must contain :nameserver or :nameserver\_port like
  #     Resolv::DNS#initialize.
  sig do
    params(
      config_info: T.any(
        NilClass,
        { nameserver: T.any(String, T::Array[String]), search: T::Array[String], ndots: Integer },
        { nameserver_port: T::Array[[String, Integer]], search: T::Array[String], ndots: Integer }
      )
    ).void
  end
  def initialize(config_info = nil); end

  # Iterates over all IP addresses for `name` retrieved from the mDNS resolver,
  # provided name ends with "local". If the name does not end in "local" no
  # records will be returned.
  #
  # `name` can be a
  # [`Resolv::DNS::Name`](https://docs.ruby-lang.org/en/2.7.0/Resolv/DNS/Name.html)
  # or a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Retrieved
  # addresses will be a
  # [`Resolv::IPv4`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv4.html) or
  # [`Resolv::IPv6`](https://docs.ruby-lang.org/en/2.7.0/Resolv/IPv6.html)
  def each_address(name); end

  def make_udp_requester; end
end

# Indicates a failure to resolve a name or address.
class Resolv::ResolvError < ::StandardError; end

# Indicates a timeout resolving a name or address.
class Resolv::ResolvTimeout < ::Timeout::Error; end
