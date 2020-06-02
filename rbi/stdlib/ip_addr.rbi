# typed: __STDLIB_INTERNAL

# [`IPAddr`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html) provides a set of
# methods to manipulate an IP address. Both IPv4 and IPv6 are supported.
#
# ## Example
#
# ```ruby
# require 'ipaddr'
#
# ipaddr1 = IPAddr.new "3ffe:505:2::1"
#
# p ipaddr1                   #=> #<IPAddr: IPv6:3ffe:0505:0002:0000:0000:0000:0000:0001/ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff>
#
# p ipaddr1.to_s              #=> "3ffe:505:2::1"
#
# ipaddr2 = ipaddr1.mask(48)  #=> #<IPAddr: IPv6:3ffe:0505:0002:0000:0000:0000:0000:0000/ffff:ffff:ffff:0000:0000:0000:0000:0000>
#
# p ipaddr2.to_s              #=> "3ffe:505:2::"
#
# ipaddr3 = IPAddr.new "192.168.2.0/24"
#
# p ipaddr3                   #=> #<IPAddr: IPv4:192.168.2.0/255.255.255.0>
# ```
class IPAddr
  include(::Comparable)

  # 32 bit mask for IPv4
  IN4MASK = T.let(T.unsafe(nil), Integer)

  # Format string for IPv6
  IN6FORMAT = T.let(T.unsafe(nil), String)

  # 128 bit mask for IPv6
  IN6MASK = T.let(T.unsafe(nil), Integer)

  # [`Regexp`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html) *internally*
  # used for parsing IPv4 address.
  RE_IPV4ADDRLIKE = T.let(T.unsafe(nil), Regexp)

  # [`Regexp`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html) *internally*
  # used for parsing IPv6 address.
  RE_IPV6ADDRLIKE_COMPRESSED = T.let(T.unsafe(nil), Regexp)

  # [`Regexp`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html) *internally*
  # used for parsing IPv6 address.
  RE_IPV6ADDRLIKE_FULL = T.let(T.unsafe(nil), Regexp)

  # Creates a new ipaddr object either from a human readable IP address
  # representation in string, or from a packed
  # [`in_addr`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html#method-i-in_addr)
  # value followed by an address family.
  #
  # In the former case, the following are the valid formats that will be
  # recognized: "address", "address/prefixlen" and "address/mask", where IPv6
  # address may be enclosed in square brackets (`[' and `]'). If a prefixlen or
  # a mask is specified, it returns a masked IP address. Although the address
  # family is determined automatically from a specified string, you can specify
  # one explicitly by the optional second argument.
  #
  # Otherwise an IP address is generated from a packed
  # [`in_addr`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html#method-i-in_addr)
  # value and an address family.
  #
  # The [`IPAddr`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html) class
  # defines many methods and operators, and some of those, such as &, |,
  # include? and ==, accept a string, or a packed
  # [`in_addr`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html#method-i-in_addr)
  # value instead of an
  # [`IPAddr`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html) object.
  def self.new(addr = _, family = _); end

  # Returns a new ipaddr built by bitwise AND.
  def &(other); end

  # Returns a new ipaddr built by bitwise left shift.
  def <<(num); end

  # Compares the ipaddr with another.
  def <=>(other); end

  # Alias for:
  # [`include?`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html#method-i-include-3F)
  def ===(other); end

  # Returns a new ipaddr built by bitwise right-shift.
  def >>(num); end

  # Checks equality used by
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
  def eql?(other); end

  # Returns the address family of this IP address.
  def family; end

  # Returns a hash value used by
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html),
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html), and
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) classes
  def hash; end

  # Returns a network byte ordered string form of the IP address.
  def hton; end

  # Returns true if the given ipaddr is in the range.
  #
  # e.g.:
  #
  # ```ruby
  # require 'ipaddr'
  # net1 = IPAddr.new("192.168.2.0/24")
  # net2 = IPAddr.new("192.168.2.100")
  # net3 = IPAddr.new("192.168.3.0")
  # p net1.include?(net2)     #=> true
  # p net1.include?(net3)     #=> false
  # ```
  #
  #
  # Also aliased as:
  # [`===`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html#method-i-3D-3D-3D)
  def include?(other); end

  # Returns a string containing a human-readable representation of the ipaddr.
  # ("#<IPAddr: family:address/mask>")
  def inspect; end

  # Returns a string for DNS reverse lookup compatible with RFC3172.
  def ip6_arpa; end

  # Returns a string for DNS reverse lookup compatible with RFC1886.
  def ip6_int; end

  # Returns true if the ipaddr is an IPv4 address.
  def ipv4?; end

  # Returns a new ipaddr built by converting the native IPv4 address into an
  # IPv4-compatible IPv6 address.
  def ipv4_compat; end

  # Returns true if the ipaddr is an IPv4-compatible IPv6 address.
  def ipv4_compat?; end

  # Returns a new ipaddr built by converting the native IPv4 address into an
  # IPv4-mapped IPv6 address.
  def ipv4_mapped; end

  # Returns true if the ipaddr is an IPv4-mapped IPv6 address.
  def ipv4_mapped?; end

  # Returns true if the ipaddr is an IPv6 address.
  def ipv6?; end

  # Returns true if the ipaddr is a link-local address. IPv4 addresses in
  # 169.254.0.0/16 reserved by RFC 3927 and Link-Local IPv6 Unicast Addresses in
  # fe80::/10 reserved by RFC 4291 are considered link-local.
  def link_local?; end

  # Returns true if the ipaddr is a loopback address.
  def loopback?; end

  # Returns a new ipaddr built by masking IP address with the given
  # prefixlen/netmask. (e.g. 8, 64, "255.255.255.0", etc.)
  def mask(prefixlen); end

  # Returns a new ipaddr built by converting the IPv6 address into a native IPv4
  # address. If the IP address is not an IPv4-mapped or IPv4-compatible IPv6
  # address, returns self.
  def native; end

  # Returns the prefix length in bits for the ipaddr.
  def prefix; end

  # Sets the prefix length in bits
  def prefix=(prefix); end

  # Returns true if the ipaddr is a private address. IPv4 addresses in
  # 10.0.0.0/8, 172.16.0.0/12 and 192.168.0.0/16 as defined in RFC 1918 and IPv6
  # Unique Local Addresses in fc00::/7 as defined in RFC 4193 are considered
  # private.
  def private?; end

  # Returns a string for DNS reverse lookup. It returns a string in RFC3172 form
  # for an IPv6 address.
  def reverse; end

  # Returns the successor to the ipaddr.
  def succ; end

  # Returns the integer representation of the ipaddr.
  def to_i; end

  # Creates a [`Range`](https://docs.ruby-lang.org/en/2.6.0/Range.html) object
  # for the network address.
  def to_range; end

  # Returns a string containing the IP address representation.
  def to_s; end

  # Returns a string containing the IP address representation in canonical form.
  def to_string; end

  # Returns a new ipaddr built by bitwise OR.
  def |(other); end

  # Returns a new ipaddr built by bitwise negation.
  def ~; end

  protected

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) current netmask to
  # given mask.
  def mask!(mask); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) +@addr+, the internal
  # stored ip address, to given `addr`. The parameter `addr` is validated using
  # the first `family` member, which is `Socket::AF_INET` or `Socket::AF_INET6`.
  def set(addr, *family); end

  # Creates a new ipaddr containing the given network byte ordered string form
  # of an IP address.
  def self.new_ntoh(addr); end

  # Convert a network byte ordered string form of an IP address into human
  # readable form.
  def self.ntop(addr); end
end

# Raised when the address family is invalid such as an address with an
# unsupported family, an address with an inconsistent family, or an address
# who's family cannot be determined.
class IPAddr::AddressFamilyError < ::IPAddr::Error; end

# Generic [`IPAddr`](https://docs.ruby-lang.org/en/2.6.0/IPAddr.html) related
# error. Exceptions raised in this class should inherit from
# [`Error`](https://docs.ruby-lang.org/en/2.6.0/IPAddr/Error.html).
class IPAddr::Error < ::ArgumentError; end

# Raised when the provided IP address is an invalid address.
class IPAddr::InvalidAddressError < ::IPAddr::Error; end

# Raised when the address is an invalid length.
class IPAddr::InvalidPrefixError < ::IPAddr::InvalidAddressError; end
