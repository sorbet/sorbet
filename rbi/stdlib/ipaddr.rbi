# typed: true

class IPAddr
  include(Comparable)

  class Error < ::ArgumentError

  end

  IN4MASK = T.let(T.unsafe(nil), Integer)

  IN6MASK = T.let(T.unsafe(nil), Integer)

  IN6FORMAT = T.let(T.unsafe(nil), String)

  RE_IPV4ADDRLIKE = T.let(T.unsafe(nil), Regexp)

  RE_IPV6ADDRLIKE_FULL = T.let(T.unsafe(nil), Regexp)

  RE_IPV6ADDRLIKE_COMPRESSED = T.let(T.unsafe(nil), Regexp)

  class AddressFamilyError < ::IPAddr::Error

  end

  class InvalidAddressError < ::IPAddr::Error

  end

  class InvalidPrefixError < ::IPAddr::InvalidAddressError

  end

  def <=>(other)
  end

  def <<(num)
  end

  def >>(num)
  end

  def ==(other)
  end

  def ===(other)
  end

  def include?(other)
  end

  def eql?(other)
  end

  def prefix()
  end

  def &(other)
  end

  def reverse()
  end

  def inspect()
  end

  def succ()
  end

  def to_s()
  end

  def to_i()
  end

  def family()
  end

  def native()
  end

  def ipv4_mapped?()
  end

  def to_string()
  end

  def ipv4?()
  end

  def hton()
  end

  def ipv6?()
  end

  def loopback?()
  end

  def private?()
  end

  def link_local?()
  end

  def ipv4_compat?()
  end

  def ipv4_mapped()
  end

  def ipv4_compat()
  end

  def hash()
  end

  def ip6_arpa()
  end

  def ip6_int()
  end

  def to_range()
  end

  def prefix=(prefix)
  end

  def mask(prefixlen)
  end

  def |(other)
  end

  def ~()
  end
end
