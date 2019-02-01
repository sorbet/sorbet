# typed: strict

class Addrinfo < Data
  Sorbet.sig {returns(::T.untyped)}
  def afamily(); end

  Sorbet.sig {returns(::T.untyped)}
  def bind(); end

  Sorbet.sig {returns(::T.untyped)}
  def canonname(); end

  Sorbet.sig do
    params(
      timeout: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect(timeout: T.unsafe(nil), &block); end

  Sorbet.sig do
    params(
      args: ::T.untyped,
      timeout: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect_from(*args, timeout: T.unsafe(nil), &block); end

  Sorbet.sig do
    params(
      args: ::T.untyped,
      timeout: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect_to(*args, timeout: T.unsafe(nil), &block); end

  Sorbet.sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def family_addrinfo(*args); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def getnameinfo(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(*_); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect_sockaddr(); end

  Sorbet.sig {returns(::T.untyped)}
  def ip?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ip_address(); end

  Sorbet.sig {returns(::T.untyped)}
  def ip_port(); end

  Sorbet.sig {returns(::T.untyped)}
  def ip_unpack(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv4?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv4_loopback?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv4_multicast?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv4_private?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_linklocal?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_loopback?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_mc_global?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_mc_linklocal?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_mc_nodelocal?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_mc_orglocal?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_mc_sitelocal?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_multicast?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_sitelocal?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_to_ipv4(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_unique_local?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_unspecified?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_v4compat?(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_v4mapped?(); end

  Sorbet.sig do
    params(
      backlog: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(backlog=T.unsafe(nil)); end

  Sorbet.sig {returns(::T.untyped)}
  def marshal_dump(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def marshal_load(_); end

  Sorbet.sig {returns(::T.untyped)}
  def pfamily(); end

  Sorbet.sig {returns(::T.untyped)}
  def protocol(); end

  Sorbet.sig {returns(::T.untyped)}
  def socktype(); end

  Sorbet.sig {returns(::T.untyped)}
  def to_s(); end

  Sorbet.sig {returns(::T.untyped)}
  def to_sockaddr(); end

  Sorbet.sig {returns(::T.untyped)}
  def unix?(); end

  Sorbet.sig {returns(::T.untyped)}
  def unix_path(); end

  Sorbet.sig do
    params(
      nodename: ::T.untyped,
      service: ::T.untyped,
      family: ::T.untyped,
      socktype: ::T.untyped,
      protocol: ::T.untyped,
      flags: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.foreach(nodename, service, family=T.unsafe(nil), socktype=T.unsafe(nil), protocol=T.unsafe(nil), flags=T.unsafe(nil), &block); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getaddrinfo(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ip(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.tcp(_, _1); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp(_, _1); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix(*_); end
end

class BasicSocket < IO
  extend T::Generic
  Elem = type_member(:out, fixed: String)

  Sorbet.sig {returns(::T.untyped)}
  def close_read(); end

  Sorbet.sig {returns(::T.untyped)}
  def close_write(); end

  Sorbet.sig {returns(::T.untyped)}
  def connect_address(); end

  Sorbet.sig {returns(::T.untyped)}
  def do_not_reverse_lookup(); end

  Sorbet.sig do
    params(
      do_not_reverse_lookup: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def do_not_reverse_lookup=(do_not_reverse_lookup); end

  Sorbet.sig {returns(::T.untyped)}
  def getpeereid(); end

  Sorbet.sig {returns(::T.untyped)}
  def getpeername(); end

  Sorbet.sig {returns(::T.untyped)}
  def getsockname(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def getsockopt(_, _1); end

  Sorbet.sig {returns(::T.untyped)}
  def local_address(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recv(*_); end

  Sorbet.sig do
    params(
      len: ::T.untyped,
      flag: ::T.untyped,
      str: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recv_nonblock(len, flag=T.unsafe(nil), str=T.unsafe(nil), exception: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      dlen: ::T.untyped,
      flags: ::T.untyped,
      clen: ::T.untyped,
      scm_rights: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvmsg(dlen=T.unsafe(nil), flags=T.unsafe(nil), clen=T.unsafe(nil), scm_rights: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      dlen: ::T.untyped,
      flags: ::T.untyped,
      clen: ::T.untyped,
      scm_rights: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvmsg_nonblock(dlen=T.unsafe(nil), flags=T.unsafe(nil), clen=T.unsafe(nil), scm_rights: T.unsafe(nil), exception: T.unsafe(nil)); end

  Sorbet.sig {returns(::T.untyped)}
  def remote_address(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def send(*_); end

  Sorbet.sig do
    params(
      mesg: ::T.untyped,
      flags: ::T.untyped,
      dest_sockaddr: ::T.untyped,
      controls: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sendmsg(mesg, flags=T.unsafe(nil), dest_sockaddr=T.unsafe(nil), *controls); end

  Sorbet.sig do
    params(
      mesg: ::T.untyped,
      flags: ::T.untyped,
      dest_sockaddr: ::T.untyped,
      controls: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sendmsg_nonblock(mesg, flags=T.unsafe(nil), dest_sockaddr=T.unsafe(nil), *controls, exception: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def setsockopt(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def shutdown(*_); end

  Sorbet.sig {returns(::T.untyped)}
  def self.do_not_reverse_lookup(); end

  Sorbet.sig do
    params(
      do_not_reverse_lookup: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.do_not_reverse_lookup=(do_not_reverse_lookup); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.for_fd(_); end
end

class IPSocket < BasicSocket
  extend T::Generic
  Elem = type_member(:out, fixed: String)

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def addr(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def peeraddr(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getaddress(_); end
end

class Socket < BasicSocket
  extend T::Generic
  Elem = type_member(:out, fixed: String)

  AF_APPLETALK = ::T.let(nil, ::T.untyped)
  AF_AX25 = ::T.let(nil, ::T.untyped)
  AF_INET = ::T.let(nil, ::T.untyped)
  AF_INET6 = ::T.let(nil, ::T.untyped)
  AF_IPX = ::T.let(nil, ::T.untyped)
  AF_ISDN = ::T.let(nil, ::T.untyped)
  AF_LOCAL = ::T.let(nil, ::T.untyped)
  AF_MAX = ::T.let(nil, ::T.untyped)
  AF_PACKET = ::T.let(nil, ::T.untyped)
  AF_ROUTE = ::T.let(nil, ::T.untyped)
  AF_SNA = ::T.let(nil, ::T.untyped)
  AF_UNIX = ::T.let(nil, ::T.untyped)
  AF_UNSPEC = ::T.let(nil, ::T.untyped)
  AI_ADDRCONFIG = ::T.let(nil, ::T.untyped)
  AI_ALL = ::T.let(nil, ::T.untyped)
  AI_CANONNAME = ::T.let(nil, ::T.untyped)
  AI_NUMERICHOST = ::T.let(nil, ::T.untyped)
  AI_NUMERICSERV = ::T.let(nil, ::T.untyped)
  AI_PASSIVE = ::T.let(nil, ::T.untyped)
  AI_V4MAPPED = ::T.let(nil, ::T.untyped)
  EAI_ADDRFAMILY = ::T.let(nil, ::T.untyped)
  EAI_AGAIN = ::T.let(nil, ::T.untyped)
  EAI_BADFLAGS = ::T.let(nil, ::T.untyped)
  EAI_FAIL = ::T.let(nil, ::T.untyped)
  EAI_FAMILY = ::T.let(nil, ::T.untyped)
  EAI_MEMORY = ::T.let(nil, ::T.untyped)
  EAI_NODATA = ::T.let(nil, ::T.untyped)
  EAI_NONAME = ::T.let(nil, ::T.untyped)
  EAI_OVERFLOW = ::T.let(nil, ::T.untyped)
  EAI_SERVICE = ::T.let(nil, ::T.untyped)
  EAI_SOCKTYPE = ::T.let(nil, ::T.untyped)
  EAI_SYSTEM = ::T.let(nil, ::T.untyped)
  IFF_ALLMULTI = ::T.let(nil, ::T.untyped)
  IFF_AUTOMEDIA = ::T.let(nil, ::T.untyped)
  IFF_BROADCAST = ::T.let(nil, ::T.untyped)
  IFF_DEBUG = ::T.let(nil, ::T.untyped)
  IFF_DYNAMIC = ::T.let(nil, ::T.untyped)
  IFF_LOOPBACK = ::T.let(nil, ::T.untyped)
  IFF_MASTER = ::T.let(nil, ::T.untyped)
  IFF_MULTICAST = ::T.let(nil, ::T.untyped)
  IFF_NOARP = ::T.let(nil, ::T.untyped)
  IFF_NOTRAILERS = ::T.let(nil, ::T.untyped)
  IFF_POINTOPOINT = ::T.let(nil, ::T.untyped)
  IFF_PORTSEL = ::T.let(nil, ::T.untyped)
  IFF_PROMISC = ::T.let(nil, ::T.untyped)
  IFF_RUNNING = ::T.let(nil, ::T.untyped)
  IFF_SLAVE = ::T.let(nil, ::T.untyped)
  IFF_UP = ::T.let(nil, ::T.untyped)
  IFNAMSIZ = ::T.let(nil, ::T.untyped)
  IF_NAMESIZE = ::T.let(nil, ::T.untyped)
  INADDR_ALLHOSTS_GROUP = ::T.let(nil, ::T.untyped)
  INADDR_ANY = ::T.let(nil, ::T.untyped)
  INADDR_BROADCAST = ::T.let(nil, ::T.untyped)
  INADDR_LOOPBACK = ::T.let(nil, ::T.untyped)
  INADDR_MAX_LOCAL_GROUP = ::T.let(nil, ::T.untyped)
  INADDR_NONE = ::T.let(nil, ::T.untyped)
  INADDR_UNSPEC_GROUP = ::T.let(nil, ::T.untyped)
  INET6_ADDRSTRLEN = ::T.let(nil, ::T.untyped)
  INET_ADDRSTRLEN = ::T.let(nil, ::T.untyped)
  IPPORT_RESERVED = ::T.let(nil, ::T.untyped)
  IPPORT_USERRESERVED = ::T.let(nil, ::T.untyped)
  IPPROTO_AH = ::T.let(nil, ::T.untyped)
  IPPROTO_DSTOPTS = ::T.let(nil, ::T.untyped)
  IPPROTO_EGP = ::T.let(nil, ::T.untyped)
  IPPROTO_ESP = ::T.let(nil, ::T.untyped)
  IPPROTO_FRAGMENT = ::T.let(nil, ::T.untyped)
  IPPROTO_HOPOPTS = ::T.let(nil, ::T.untyped)
  IPPROTO_ICMP = ::T.let(nil, ::T.untyped)
  IPPROTO_ICMPV6 = ::T.let(nil, ::T.untyped)
  IPPROTO_IDP = ::T.let(nil, ::T.untyped)
  IPPROTO_IGMP = ::T.let(nil, ::T.untyped)
  IPPROTO_IP = ::T.let(nil, ::T.untyped)
  IPPROTO_IPV6 = ::T.let(nil, ::T.untyped)
  IPPROTO_NONE = ::T.let(nil, ::T.untyped)
  IPPROTO_PUP = ::T.let(nil, ::T.untyped)
  IPPROTO_RAW = ::T.let(nil, ::T.untyped)
  IPPROTO_ROUTING = ::T.let(nil, ::T.untyped)
  IPPROTO_TCP = ::T.let(nil, ::T.untyped)
  IPPROTO_TP = ::T.let(nil, ::T.untyped)
  IPPROTO_UDP = ::T.let(nil, ::T.untyped)
  IPV6_CHECKSUM = ::T.let(nil, ::T.untyped)
  IPV6_DSTOPTS = ::T.let(nil, ::T.untyped)
  IPV6_HOPLIMIT = ::T.let(nil, ::T.untyped)
  IPV6_HOPOPTS = ::T.let(nil, ::T.untyped)
  IPV6_JOIN_GROUP = ::T.let(nil, ::T.untyped)
  IPV6_LEAVE_GROUP = ::T.let(nil, ::T.untyped)
  IPV6_MULTICAST_HOPS = ::T.let(nil, ::T.untyped)
  IPV6_MULTICAST_IF = ::T.let(nil, ::T.untyped)
  IPV6_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  IPV6_NEXTHOP = ::T.let(nil, ::T.untyped)
  IPV6_PKTINFO = ::T.let(nil, ::T.untyped)
  IPV6_RECVDSTOPTS = ::T.let(nil, ::T.untyped)
  IPV6_RECVHOPLIMIT = ::T.let(nil, ::T.untyped)
  IPV6_RECVHOPOPTS = ::T.let(nil, ::T.untyped)
  IPV6_RECVPKTINFO = ::T.let(nil, ::T.untyped)
  IPV6_RECVRTHDR = ::T.let(nil, ::T.untyped)
  IPV6_RECVTCLASS = ::T.let(nil, ::T.untyped)
  IPV6_RTHDR = ::T.let(nil, ::T.untyped)
  IPV6_RTHDRDSTOPTS = ::T.let(nil, ::T.untyped)
  IPV6_RTHDR_TYPE_0 = ::T.let(nil, ::T.untyped)
  IPV6_TCLASS = ::T.let(nil, ::T.untyped)
  IPV6_UNICAST_HOPS = ::T.let(nil, ::T.untyped)
  IPV6_V6ONLY = ::T.let(nil, ::T.untyped)
  IP_ADD_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  IP_ADD_SOURCE_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  IP_BLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  IP_DEFAULT_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  IP_DEFAULT_MULTICAST_TTL = ::T.let(nil, ::T.untyped)
  IP_DROP_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  IP_DROP_SOURCE_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  IP_FREEBIND = ::T.let(nil, ::T.untyped)
  IP_HDRINCL = ::T.let(nil, ::T.untyped)
  IP_IPSEC_POLICY = ::T.let(nil, ::T.untyped)
  IP_MAX_MEMBERSHIPS = ::T.let(nil, ::T.untyped)
  IP_MINTTL = ::T.let(nil, ::T.untyped)
  IP_MSFILTER = ::T.let(nil, ::T.untyped)
  IP_MTU = ::T.let(nil, ::T.untyped)
  IP_MTU_DISCOVER = ::T.let(nil, ::T.untyped)
  IP_MULTICAST_IF = ::T.let(nil, ::T.untyped)
  IP_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  IP_MULTICAST_TTL = ::T.let(nil, ::T.untyped)
  IP_OPTIONS = ::T.let(nil, ::T.untyped)
  IP_PASSSEC = ::T.let(nil, ::T.untyped)
  IP_PKTINFO = ::T.let(nil, ::T.untyped)
  IP_PKTOPTIONS = ::T.let(nil, ::T.untyped)
  IP_PMTUDISC_DO = ::T.let(nil, ::T.untyped)
  IP_PMTUDISC_DONT = ::T.let(nil, ::T.untyped)
  IP_PMTUDISC_WANT = ::T.let(nil, ::T.untyped)
  IP_RECVERR = ::T.let(nil, ::T.untyped)
  IP_RECVOPTS = ::T.let(nil, ::T.untyped)
  IP_RECVRETOPTS = ::T.let(nil, ::T.untyped)
  IP_RECVTOS = ::T.let(nil, ::T.untyped)
  IP_RECVTTL = ::T.let(nil, ::T.untyped)
  IP_RETOPTS = ::T.let(nil, ::T.untyped)
  IP_ROUTER_ALERT = ::T.let(nil, ::T.untyped)
  IP_TOS = ::T.let(nil, ::T.untyped)
  IP_TRANSPARENT = ::T.let(nil, ::T.untyped)
  IP_TTL = ::T.let(nil, ::T.untyped)
  IP_UNBLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  IP_XFRM_POLICY = ::T.let(nil, ::T.untyped)
  MCAST_BLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  MCAST_EXCLUDE = ::T.let(nil, ::T.untyped)
  MCAST_INCLUDE = ::T.let(nil, ::T.untyped)
  MCAST_JOIN_GROUP = ::T.let(nil, ::T.untyped)
  MCAST_JOIN_SOURCE_GROUP = ::T.let(nil, ::T.untyped)
  MCAST_LEAVE_GROUP = ::T.let(nil, ::T.untyped)
  MCAST_LEAVE_SOURCE_GROUP = ::T.let(nil, ::T.untyped)
  MCAST_MSFILTER = ::T.let(nil, ::T.untyped)
  MCAST_UNBLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  MSG_CONFIRM = ::T.let(nil, ::T.untyped)
  MSG_CTRUNC = ::T.let(nil, ::T.untyped)
  MSG_DONTROUTE = ::T.let(nil, ::T.untyped)
  MSG_DONTWAIT = ::T.let(nil, ::T.untyped)
  MSG_EOR = ::T.let(nil, ::T.untyped)
  MSG_ERRQUEUE = ::T.let(nil, ::T.untyped)
  MSG_FASTOPEN = ::T.let(nil, ::T.untyped)
  MSG_FIN = ::T.let(nil, ::T.untyped)
  MSG_MORE = ::T.let(nil, ::T.untyped)
  MSG_NOSIGNAL = ::T.let(nil, ::T.untyped)
  MSG_OOB = ::T.let(nil, ::T.untyped)
  MSG_PEEK = ::T.let(nil, ::T.untyped)
  MSG_PROXY = ::T.let(nil, ::T.untyped)
  MSG_RST = ::T.let(nil, ::T.untyped)
  MSG_SYN = ::T.let(nil, ::T.untyped)
  MSG_TRUNC = ::T.let(nil, ::T.untyped)
  MSG_WAITALL = ::T.let(nil, ::T.untyped)
  NI_DGRAM = ::T.let(nil, ::T.untyped)
  NI_MAXHOST = ::T.let(nil, ::T.untyped)
  NI_MAXSERV = ::T.let(nil, ::T.untyped)
  NI_NAMEREQD = ::T.let(nil, ::T.untyped)
  NI_NOFQDN = ::T.let(nil, ::T.untyped)
  NI_NUMERICHOST = ::T.let(nil, ::T.untyped)
  NI_NUMERICSERV = ::T.let(nil, ::T.untyped)
  PF_APPLETALK = ::T.let(nil, ::T.untyped)
  PF_AX25 = ::T.let(nil, ::T.untyped)
  PF_INET = ::T.let(nil, ::T.untyped)
  PF_INET6 = ::T.let(nil, ::T.untyped)
  PF_IPX = ::T.let(nil, ::T.untyped)
  PF_ISDN = ::T.let(nil, ::T.untyped)
  PF_KEY = ::T.let(nil, ::T.untyped)
  PF_LOCAL = ::T.let(nil, ::T.untyped)
  PF_MAX = ::T.let(nil, ::T.untyped)
  PF_PACKET = ::T.let(nil, ::T.untyped)
  PF_ROUTE = ::T.let(nil, ::T.untyped)
  PF_SNA = ::T.let(nil, ::T.untyped)
  PF_UNIX = ::T.let(nil, ::T.untyped)
  PF_UNSPEC = ::T.let(nil, ::T.untyped)
  SCM_CREDENTIALS = ::T.let(nil, ::T.untyped)
  SCM_RIGHTS = ::T.let(nil, ::T.untyped)
  SCM_TIMESTAMP = ::T.let(nil, ::T.untyped)
  SCM_TIMESTAMPING = ::T.let(nil, ::T.untyped)
  SCM_TIMESTAMPNS = ::T.let(nil, ::T.untyped)
  SCM_WIFI_STATUS = ::T.let(nil, ::T.untyped)
  SHUT_RD = ::T.let(nil, ::T.untyped)
  SHUT_RDWR = ::T.let(nil, ::T.untyped)
  SHUT_WR = ::T.let(nil, ::T.untyped)
  SOCK_DGRAM = ::T.let(nil, ::T.untyped)
  SOCK_PACKET = ::T.let(nil, ::T.untyped)
  SOCK_RAW = ::T.let(nil, ::T.untyped)
  SOCK_RDM = ::T.let(nil, ::T.untyped)
  SOCK_SEQPACKET = ::T.let(nil, ::T.untyped)
  SOCK_STREAM = ::T.let(nil, ::T.untyped)
  SOL_IP = ::T.let(nil, ::T.untyped)
  SOL_SOCKET = ::T.let(nil, ::T.untyped)
  SOL_TCP = ::T.let(nil, ::T.untyped)
  SOL_UDP = ::T.let(nil, ::T.untyped)
  SOMAXCONN = ::T.let(nil, ::T.untyped)
  SO_ACCEPTCONN = ::T.let(nil, ::T.untyped)
  SO_ATTACH_FILTER = ::T.let(nil, ::T.untyped)
  SO_BINDTODEVICE = ::T.let(nil, ::T.untyped)
  SO_BROADCAST = ::T.let(nil, ::T.untyped)
  SO_BUSY_POLL = ::T.let(nil, ::T.untyped)
  SO_DEBUG = ::T.let(nil, ::T.untyped)
  SO_DETACH_FILTER = ::T.let(nil, ::T.untyped)
  SO_DOMAIN = ::T.let(nil, ::T.untyped)
  SO_DONTROUTE = ::T.let(nil, ::T.untyped)
  SO_ERROR = ::T.let(nil, ::T.untyped)
  SO_GET_FILTER = ::T.let(nil, ::T.untyped)
  SO_KEEPALIVE = ::T.let(nil, ::T.untyped)
  SO_LINGER = ::T.let(nil, ::T.untyped)
  SO_LOCK_FILTER = ::T.let(nil, ::T.untyped)
  SO_MARK = ::T.let(nil, ::T.untyped)
  SO_MAX_PACING_RATE = ::T.let(nil, ::T.untyped)
  SO_NOFCS = ::T.let(nil, ::T.untyped)
  SO_NO_CHECK = ::T.let(nil, ::T.untyped)
  SO_OOBINLINE = ::T.let(nil, ::T.untyped)
  SO_PASSCRED = ::T.let(nil, ::T.untyped)
  SO_PASSSEC = ::T.let(nil, ::T.untyped)
  SO_PEEK_OFF = ::T.let(nil, ::T.untyped)
  SO_PEERCRED = ::T.let(nil, ::T.untyped)
  SO_PEERNAME = ::T.let(nil, ::T.untyped)
  SO_PEERSEC = ::T.let(nil, ::T.untyped)
  SO_PRIORITY = ::T.let(nil, ::T.untyped)
  SO_PROTOCOL = ::T.let(nil, ::T.untyped)
  SO_RCVBUF = ::T.let(nil, ::T.untyped)
  SO_RCVBUFFORCE = ::T.let(nil, ::T.untyped)
  SO_RCVLOWAT = ::T.let(nil, ::T.untyped)
  SO_RCVTIMEO = ::T.let(nil, ::T.untyped)
  SO_REUSEADDR = ::T.let(nil, ::T.untyped)
  SO_REUSEPORT = ::T.let(nil, ::T.untyped)
  SO_RXQ_OVFL = ::T.let(nil, ::T.untyped)
  SO_SECURITY_AUTHENTICATION = ::T.let(nil, ::T.untyped)
  SO_SECURITY_ENCRYPTION_NETWORK = ::T.let(nil, ::T.untyped)
  SO_SECURITY_ENCRYPTION_TRANSPORT = ::T.let(nil, ::T.untyped)
  SO_SELECT_ERR_QUEUE = ::T.let(nil, ::T.untyped)
  SO_SNDBUF = ::T.let(nil, ::T.untyped)
  SO_SNDBUFFORCE = ::T.let(nil, ::T.untyped)
  SO_SNDLOWAT = ::T.let(nil, ::T.untyped)
  SO_SNDTIMEO = ::T.let(nil, ::T.untyped)
  SO_TIMESTAMP = ::T.let(nil, ::T.untyped)
  SO_TIMESTAMPING = ::T.let(nil, ::T.untyped)
  SO_TIMESTAMPNS = ::T.let(nil, ::T.untyped)
  SO_TYPE = ::T.let(nil, ::T.untyped)
  SO_WIFI_STATUS = ::T.let(nil, ::T.untyped)
  TCP_CONGESTION = ::T.let(nil, ::T.untyped)
  TCP_COOKIE_TRANSACTIONS = ::T.let(nil, ::T.untyped)
  TCP_CORK = ::T.let(nil, ::T.untyped)
  TCP_DEFER_ACCEPT = ::T.let(nil, ::T.untyped)
  TCP_FASTOPEN = ::T.let(nil, ::T.untyped)
  TCP_INFO = ::T.let(nil, ::T.untyped)
  TCP_KEEPCNT = ::T.let(nil, ::T.untyped)
  TCP_KEEPIDLE = ::T.let(nil, ::T.untyped)
  TCP_KEEPINTVL = ::T.let(nil, ::T.untyped)
  TCP_LINGER2 = ::T.let(nil, ::T.untyped)
  TCP_MAXSEG = ::T.let(nil, ::T.untyped)
  TCP_MD5SIG = ::T.let(nil, ::T.untyped)
  TCP_NODELAY = ::T.let(nil, ::T.untyped)
  TCP_QUEUE_SEQ = ::T.let(nil, ::T.untyped)
  TCP_QUICKACK = ::T.let(nil, ::T.untyped)
  TCP_REPAIR = ::T.let(nil, ::T.untyped)
  TCP_REPAIR_OPTIONS = ::T.let(nil, ::T.untyped)
  TCP_REPAIR_QUEUE = ::T.let(nil, ::T.untyped)
  TCP_SYNCNT = ::T.let(nil, ::T.untyped)
  TCP_THIN_DUPACK = ::T.let(nil, ::T.untyped)
  TCP_THIN_LINEAR_TIMEOUTS = ::T.let(nil, ::T.untyped)
  TCP_TIMESTAMP = ::T.let(nil, ::T.untyped)
  TCP_USER_TIMEOUT = ::T.let(nil, ::T.untyped)
  TCP_WINDOW_CLAMP = ::T.let(nil, ::T.untyped)
  UDP_CORK = ::T.let(nil, ::T.untyped)

  Sorbet.sig {returns(::T.untyped)}
  def accept(); end

  Sorbet.sig do
    params(
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def accept_nonblock(exception: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def bind(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect(_); end

  Sorbet.sig do
    params(
      addr: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect_nonblock(addr, exception: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(*_); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6only!(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom(*_); end

  Sorbet.sig do
    params(
      len: ::T.untyped,
      flag: ::T.untyped,
      str: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom_nonblock(len, flag=T.unsafe(nil), str=T.unsafe(nil), exception: T.unsafe(nil)); end

  Sorbet.sig {returns(::T.untyped)}
  def sysaccept(); end

  Sorbet.sig do
    params(
      sockets: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.accept_loop(*sockets); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getaddrinfo(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.gethostbyaddr(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.gethostbyname(_); end

  Sorbet.sig {returns(::T.untyped)}
  def self.gethostname(); end

  Sorbet.sig {returns(::T.untyped)}
  def self.getifaddrs(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getnameinfo(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getservbyname(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getservbyport(*_); end

  Sorbet.sig {returns(::T.untyped)}
  def self.ip_address_list(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pack_sockaddr_in(_, _1); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pack_sockaddr_un(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pair(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.sockaddr_in(_, _1); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.sockaddr_un(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socketpair(*_); end

  Sorbet.sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
      local_host: ::T.untyped,
      local_port: ::T.untyped,
      connect_timeout: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.tcp(host, port, local_host=T.unsafe(nil), local_port=T.unsafe(nil), connect_timeout: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.tcp_server_loop(host=T.unsafe(nil), port, &b); end

  Sorbet.sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.tcp_server_sockets(host=T.unsafe(nil), port); end

  Sorbet.sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp_server_loop(host=T.unsafe(nil), port, &b); end

  Sorbet.sig do
    params(
      sockets: ::T.untyped,
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp_server_loop_on(sockets, &b); end

  Sorbet.sig do
    params(
      sockets: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp_server_recv(sockets); end

  Sorbet.sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp_server_sockets(host=T.unsafe(nil), port); end

  Sorbet.sig do
    params(
      path: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix(path); end

  Sorbet.sig do
    params(
      path: ::T.untyped,
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix_server_loop(path, &b); end

  Sorbet.sig do
    params(
      path: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix_server_socket(path); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unpack_sockaddr_in(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unpack_sockaddr_un(_); end
end

class Socket::AncillaryData
  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cmsg_is?(_, _1); end

  Sorbet.sig {returns(::T.untyped)}
  def data(); end

  Sorbet.sig {returns(::T.untyped)}
  def family(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
      _2: ::T.untyped,
      _3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(_, _1, _2, _3); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def int(); end

  Sorbet.sig {returns(::T.untyped)}
  def ip_pktinfo(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_pktinfo(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_pktinfo_addr(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv6_pktinfo_ifindex(); end

  Sorbet.sig {returns(::T.untyped)}
  def level(); end

  Sorbet.sig {returns(::T.untyped)}
  def timestamp(); end

  Sorbet.sig {returns(::T.untyped)}
  def type(); end

  Sorbet.sig {returns(::T.untyped)}
  def unix_rights(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
      _2: ::T.untyped,
      _3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.int(_, _1, _2, _3); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ip_pktinfo(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ipv6_pktinfo(_, _1); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix_rights(*_); end
end

module Socket::Constants
  AF_APPLETALK = ::T.let(nil, ::T.untyped)
  AF_AX25 = ::T.let(nil, ::T.untyped)
  AF_INET = ::T.let(nil, ::T.untyped)
  AF_INET6 = ::T.let(nil, ::T.untyped)
  AF_IPX = ::T.let(nil, ::T.untyped)
  AF_ISDN = ::T.let(nil, ::T.untyped)
  AF_LOCAL = ::T.let(nil, ::T.untyped)
  AF_MAX = ::T.let(nil, ::T.untyped)
  AF_PACKET = ::T.let(nil, ::T.untyped)
  AF_ROUTE = ::T.let(nil, ::T.untyped)
  AF_SNA = ::T.let(nil, ::T.untyped)
  AF_UNIX = ::T.let(nil, ::T.untyped)
  AF_UNSPEC = ::T.let(nil, ::T.untyped)
  AI_ADDRCONFIG = ::T.let(nil, ::T.untyped)
  AI_ALL = ::T.let(nil, ::T.untyped)
  AI_CANONNAME = ::T.let(nil, ::T.untyped)
  AI_NUMERICHOST = ::T.let(nil, ::T.untyped)
  AI_NUMERICSERV = ::T.let(nil, ::T.untyped)
  AI_PASSIVE = ::T.let(nil, ::T.untyped)
  AI_V4MAPPED = ::T.let(nil, ::T.untyped)
  EAI_ADDRFAMILY = ::T.let(nil, ::T.untyped)
  EAI_AGAIN = ::T.let(nil, ::T.untyped)
  EAI_BADFLAGS = ::T.let(nil, ::T.untyped)
  EAI_FAIL = ::T.let(nil, ::T.untyped)
  EAI_FAMILY = ::T.let(nil, ::T.untyped)
  EAI_MEMORY = ::T.let(nil, ::T.untyped)
  EAI_NODATA = ::T.let(nil, ::T.untyped)
  EAI_NONAME = ::T.let(nil, ::T.untyped)
  EAI_OVERFLOW = ::T.let(nil, ::T.untyped)
  EAI_SERVICE = ::T.let(nil, ::T.untyped)
  EAI_SOCKTYPE = ::T.let(nil, ::T.untyped)
  EAI_SYSTEM = ::T.let(nil, ::T.untyped)
  IFF_ALLMULTI = ::T.let(nil, ::T.untyped)
  IFF_AUTOMEDIA = ::T.let(nil, ::T.untyped)
  IFF_BROADCAST = ::T.let(nil, ::T.untyped)
  IFF_DEBUG = ::T.let(nil, ::T.untyped)
  IFF_DYNAMIC = ::T.let(nil, ::T.untyped)
  IFF_LOOPBACK = ::T.let(nil, ::T.untyped)
  IFF_MASTER = ::T.let(nil, ::T.untyped)
  IFF_MULTICAST = ::T.let(nil, ::T.untyped)
  IFF_NOARP = ::T.let(nil, ::T.untyped)
  IFF_NOTRAILERS = ::T.let(nil, ::T.untyped)
  IFF_POINTOPOINT = ::T.let(nil, ::T.untyped)
  IFF_PORTSEL = ::T.let(nil, ::T.untyped)
  IFF_PROMISC = ::T.let(nil, ::T.untyped)
  IFF_RUNNING = ::T.let(nil, ::T.untyped)
  IFF_SLAVE = ::T.let(nil, ::T.untyped)
  IFF_UP = ::T.let(nil, ::T.untyped)
  IFNAMSIZ = ::T.let(nil, ::T.untyped)
  IF_NAMESIZE = ::T.let(nil, ::T.untyped)
  INADDR_ALLHOSTS_GROUP = ::T.let(nil, ::T.untyped)
  INADDR_ANY = ::T.let(nil, ::T.untyped)
  INADDR_BROADCAST = ::T.let(nil, ::T.untyped)
  INADDR_LOOPBACK = ::T.let(nil, ::T.untyped)
  INADDR_MAX_LOCAL_GROUP = ::T.let(nil, ::T.untyped)
  INADDR_NONE = ::T.let(nil, ::T.untyped)
  INADDR_UNSPEC_GROUP = ::T.let(nil, ::T.untyped)
  INET6_ADDRSTRLEN = ::T.let(nil, ::T.untyped)
  INET_ADDRSTRLEN = ::T.let(nil, ::T.untyped)
  IPPORT_RESERVED = ::T.let(nil, ::T.untyped)
  IPPORT_USERRESERVED = ::T.let(nil, ::T.untyped)
  IPPROTO_AH = ::T.let(nil, ::T.untyped)
  IPPROTO_DSTOPTS = ::T.let(nil, ::T.untyped)
  IPPROTO_EGP = ::T.let(nil, ::T.untyped)
  IPPROTO_ESP = ::T.let(nil, ::T.untyped)
  IPPROTO_FRAGMENT = ::T.let(nil, ::T.untyped)
  IPPROTO_HOPOPTS = ::T.let(nil, ::T.untyped)
  IPPROTO_ICMP = ::T.let(nil, ::T.untyped)
  IPPROTO_ICMPV6 = ::T.let(nil, ::T.untyped)
  IPPROTO_IDP = ::T.let(nil, ::T.untyped)
  IPPROTO_IGMP = ::T.let(nil, ::T.untyped)
  IPPROTO_IP = ::T.let(nil, ::T.untyped)
  IPPROTO_IPV6 = ::T.let(nil, ::T.untyped)
  IPPROTO_NONE = ::T.let(nil, ::T.untyped)
  IPPROTO_PUP = ::T.let(nil, ::T.untyped)
  IPPROTO_RAW = ::T.let(nil, ::T.untyped)
  IPPROTO_ROUTING = ::T.let(nil, ::T.untyped)
  IPPROTO_TCP = ::T.let(nil, ::T.untyped)
  IPPROTO_TP = ::T.let(nil, ::T.untyped)
  IPPROTO_UDP = ::T.let(nil, ::T.untyped)
  IPV6_CHECKSUM = ::T.let(nil, ::T.untyped)
  IPV6_DSTOPTS = ::T.let(nil, ::T.untyped)
  IPV6_HOPLIMIT = ::T.let(nil, ::T.untyped)
  IPV6_HOPOPTS = ::T.let(nil, ::T.untyped)
  IPV6_JOIN_GROUP = ::T.let(nil, ::T.untyped)
  IPV6_LEAVE_GROUP = ::T.let(nil, ::T.untyped)
  IPV6_MULTICAST_HOPS = ::T.let(nil, ::T.untyped)
  IPV6_MULTICAST_IF = ::T.let(nil, ::T.untyped)
  IPV6_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  IPV6_NEXTHOP = ::T.let(nil, ::T.untyped)
  IPV6_PKTINFO = ::T.let(nil, ::T.untyped)
  IPV6_RECVDSTOPTS = ::T.let(nil, ::T.untyped)
  IPV6_RECVHOPLIMIT = ::T.let(nil, ::T.untyped)
  IPV6_RECVHOPOPTS = ::T.let(nil, ::T.untyped)
  IPV6_RECVPKTINFO = ::T.let(nil, ::T.untyped)
  IPV6_RECVRTHDR = ::T.let(nil, ::T.untyped)
  IPV6_RECVTCLASS = ::T.let(nil, ::T.untyped)
  IPV6_RTHDR = ::T.let(nil, ::T.untyped)
  IPV6_RTHDRDSTOPTS = ::T.let(nil, ::T.untyped)
  IPV6_RTHDR_TYPE_0 = ::T.let(nil, ::T.untyped)
  IPV6_TCLASS = ::T.let(nil, ::T.untyped)
  IPV6_UNICAST_HOPS = ::T.let(nil, ::T.untyped)
  IPV6_V6ONLY = ::T.let(nil, ::T.untyped)
  IP_ADD_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  IP_ADD_SOURCE_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  IP_BLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  IP_DEFAULT_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  IP_DEFAULT_MULTICAST_TTL = ::T.let(nil, ::T.untyped)
  IP_DROP_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  IP_DROP_SOURCE_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  IP_FREEBIND = ::T.let(nil, ::T.untyped)
  IP_HDRINCL = ::T.let(nil, ::T.untyped)
  IP_IPSEC_POLICY = ::T.let(nil, ::T.untyped)
  IP_MAX_MEMBERSHIPS = ::T.let(nil, ::T.untyped)
  IP_MINTTL = ::T.let(nil, ::T.untyped)
  IP_MSFILTER = ::T.let(nil, ::T.untyped)
  IP_MTU = ::T.let(nil, ::T.untyped)
  IP_MTU_DISCOVER = ::T.let(nil, ::T.untyped)
  IP_MULTICAST_IF = ::T.let(nil, ::T.untyped)
  IP_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  IP_MULTICAST_TTL = ::T.let(nil, ::T.untyped)
  IP_OPTIONS = ::T.let(nil, ::T.untyped)
  IP_PASSSEC = ::T.let(nil, ::T.untyped)
  IP_PKTINFO = ::T.let(nil, ::T.untyped)
  IP_PKTOPTIONS = ::T.let(nil, ::T.untyped)
  IP_PMTUDISC_DO = ::T.let(nil, ::T.untyped)
  IP_PMTUDISC_DONT = ::T.let(nil, ::T.untyped)
  IP_PMTUDISC_WANT = ::T.let(nil, ::T.untyped)
  IP_RECVERR = ::T.let(nil, ::T.untyped)
  IP_RECVOPTS = ::T.let(nil, ::T.untyped)
  IP_RECVRETOPTS = ::T.let(nil, ::T.untyped)
  IP_RECVTOS = ::T.let(nil, ::T.untyped)
  IP_RECVTTL = ::T.let(nil, ::T.untyped)
  IP_RETOPTS = ::T.let(nil, ::T.untyped)
  IP_ROUTER_ALERT = ::T.let(nil, ::T.untyped)
  IP_TOS = ::T.let(nil, ::T.untyped)
  IP_TRANSPARENT = ::T.let(nil, ::T.untyped)
  IP_TTL = ::T.let(nil, ::T.untyped)
  IP_UNBLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  IP_XFRM_POLICY = ::T.let(nil, ::T.untyped)
  MCAST_BLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  MCAST_EXCLUDE = ::T.let(nil, ::T.untyped)
  MCAST_INCLUDE = ::T.let(nil, ::T.untyped)
  MCAST_JOIN_GROUP = ::T.let(nil, ::T.untyped)
  MCAST_JOIN_SOURCE_GROUP = ::T.let(nil, ::T.untyped)
  MCAST_LEAVE_GROUP = ::T.let(nil, ::T.untyped)
  MCAST_LEAVE_SOURCE_GROUP = ::T.let(nil, ::T.untyped)
  MCAST_MSFILTER = ::T.let(nil, ::T.untyped)
  MCAST_UNBLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  MSG_CONFIRM = ::T.let(nil, ::T.untyped)
  MSG_CTRUNC = ::T.let(nil, ::T.untyped)
  MSG_DONTROUTE = ::T.let(nil, ::T.untyped)
  MSG_DONTWAIT = ::T.let(nil, ::T.untyped)
  MSG_EOR = ::T.let(nil, ::T.untyped)
  MSG_ERRQUEUE = ::T.let(nil, ::T.untyped)
  MSG_FASTOPEN = ::T.let(nil, ::T.untyped)
  MSG_FIN = ::T.let(nil, ::T.untyped)
  MSG_MORE = ::T.let(nil, ::T.untyped)
  MSG_NOSIGNAL = ::T.let(nil, ::T.untyped)
  MSG_OOB = ::T.let(nil, ::T.untyped)
  MSG_PEEK = ::T.let(nil, ::T.untyped)
  MSG_PROXY = ::T.let(nil, ::T.untyped)
  MSG_RST = ::T.let(nil, ::T.untyped)
  MSG_SYN = ::T.let(nil, ::T.untyped)
  MSG_TRUNC = ::T.let(nil, ::T.untyped)
  MSG_WAITALL = ::T.let(nil, ::T.untyped)
  NI_DGRAM = ::T.let(nil, ::T.untyped)
  NI_MAXHOST = ::T.let(nil, ::T.untyped)
  NI_MAXSERV = ::T.let(nil, ::T.untyped)
  NI_NAMEREQD = ::T.let(nil, ::T.untyped)
  NI_NOFQDN = ::T.let(nil, ::T.untyped)
  NI_NUMERICHOST = ::T.let(nil, ::T.untyped)
  NI_NUMERICSERV = ::T.let(nil, ::T.untyped)
  PF_APPLETALK = ::T.let(nil, ::T.untyped)
  PF_AX25 = ::T.let(nil, ::T.untyped)
  PF_INET = ::T.let(nil, ::T.untyped)
  PF_INET6 = ::T.let(nil, ::T.untyped)
  PF_IPX = ::T.let(nil, ::T.untyped)
  PF_ISDN = ::T.let(nil, ::T.untyped)
  PF_KEY = ::T.let(nil, ::T.untyped)
  PF_LOCAL = ::T.let(nil, ::T.untyped)
  PF_MAX = ::T.let(nil, ::T.untyped)
  PF_PACKET = ::T.let(nil, ::T.untyped)
  PF_ROUTE = ::T.let(nil, ::T.untyped)
  PF_SNA = ::T.let(nil, ::T.untyped)
  PF_UNIX = ::T.let(nil, ::T.untyped)
  PF_UNSPEC = ::T.let(nil, ::T.untyped)
  SCM_CREDENTIALS = ::T.let(nil, ::T.untyped)
  SCM_RIGHTS = ::T.let(nil, ::T.untyped)
  SCM_TIMESTAMP = ::T.let(nil, ::T.untyped)
  SCM_TIMESTAMPING = ::T.let(nil, ::T.untyped)
  SCM_TIMESTAMPNS = ::T.let(nil, ::T.untyped)
  SCM_WIFI_STATUS = ::T.let(nil, ::T.untyped)
  SHUT_RD = ::T.let(nil, ::T.untyped)
  SHUT_RDWR = ::T.let(nil, ::T.untyped)
  SHUT_WR = ::T.let(nil, ::T.untyped)
  SOCK_DGRAM = ::T.let(nil, ::T.untyped)
  SOCK_PACKET = ::T.let(nil, ::T.untyped)
  SOCK_RAW = ::T.let(nil, ::T.untyped)
  SOCK_RDM = ::T.let(nil, ::T.untyped)
  SOCK_SEQPACKET = ::T.let(nil, ::T.untyped)
  SOCK_STREAM = ::T.let(nil, ::T.untyped)
  SOL_IP = ::T.let(nil, ::T.untyped)
  SOL_SOCKET = ::T.let(nil, ::T.untyped)
  SOL_TCP = ::T.let(nil, ::T.untyped)
  SOL_UDP = ::T.let(nil, ::T.untyped)
  SOMAXCONN = ::T.let(nil, ::T.untyped)
  SO_ACCEPTCONN = ::T.let(nil, ::T.untyped)
  SO_ATTACH_FILTER = ::T.let(nil, ::T.untyped)
  SO_BINDTODEVICE = ::T.let(nil, ::T.untyped)
  SO_BROADCAST = ::T.let(nil, ::T.untyped)
  SO_BUSY_POLL = ::T.let(nil, ::T.untyped)
  SO_DEBUG = ::T.let(nil, ::T.untyped)
  SO_DETACH_FILTER = ::T.let(nil, ::T.untyped)
  SO_DOMAIN = ::T.let(nil, ::T.untyped)
  SO_DONTROUTE = ::T.let(nil, ::T.untyped)
  SO_ERROR = ::T.let(nil, ::T.untyped)
  SO_GET_FILTER = ::T.let(nil, ::T.untyped)
  SO_KEEPALIVE = ::T.let(nil, ::T.untyped)
  SO_LINGER = ::T.let(nil, ::T.untyped)
  SO_LOCK_FILTER = ::T.let(nil, ::T.untyped)
  SO_MARK = ::T.let(nil, ::T.untyped)
  SO_MAX_PACING_RATE = ::T.let(nil, ::T.untyped)
  SO_NOFCS = ::T.let(nil, ::T.untyped)
  SO_NO_CHECK = ::T.let(nil, ::T.untyped)
  SO_OOBINLINE = ::T.let(nil, ::T.untyped)
  SO_PASSCRED = ::T.let(nil, ::T.untyped)
  SO_PASSSEC = ::T.let(nil, ::T.untyped)
  SO_PEEK_OFF = ::T.let(nil, ::T.untyped)
  SO_PEERCRED = ::T.let(nil, ::T.untyped)
  SO_PEERNAME = ::T.let(nil, ::T.untyped)
  SO_PEERSEC = ::T.let(nil, ::T.untyped)
  SO_PRIORITY = ::T.let(nil, ::T.untyped)
  SO_PROTOCOL = ::T.let(nil, ::T.untyped)
  SO_RCVBUF = ::T.let(nil, ::T.untyped)
  SO_RCVBUFFORCE = ::T.let(nil, ::T.untyped)
  SO_RCVLOWAT = ::T.let(nil, ::T.untyped)
  SO_RCVTIMEO = ::T.let(nil, ::T.untyped)
  SO_REUSEADDR = ::T.let(nil, ::T.untyped)
  SO_REUSEPORT = ::T.let(nil, ::T.untyped)
  SO_RXQ_OVFL = ::T.let(nil, ::T.untyped)
  SO_SECURITY_AUTHENTICATION = ::T.let(nil, ::T.untyped)
  SO_SECURITY_ENCRYPTION_NETWORK = ::T.let(nil, ::T.untyped)
  SO_SECURITY_ENCRYPTION_TRANSPORT = ::T.let(nil, ::T.untyped)
  SO_SELECT_ERR_QUEUE = ::T.let(nil, ::T.untyped)
  SO_SNDBUF = ::T.let(nil, ::T.untyped)
  SO_SNDBUFFORCE = ::T.let(nil, ::T.untyped)
  SO_SNDLOWAT = ::T.let(nil, ::T.untyped)
  SO_SNDTIMEO = ::T.let(nil, ::T.untyped)
  SO_TIMESTAMP = ::T.let(nil, ::T.untyped)
  SO_TIMESTAMPING = ::T.let(nil, ::T.untyped)
  SO_TIMESTAMPNS = ::T.let(nil, ::T.untyped)
  SO_TYPE = ::T.let(nil, ::T.untyped)
  SO_WIFI_STATUS = ::T.let(nil, ::T.untyped)
  TCP_CONGESTION = ::T.let(nil, ::T.untyped)
  TCP_COOKIE_TRANSACTIONS = ::T.let(nil, ::T.untyped)
  TCP_CORK = ::T.let(nil, ::T.untyped)
  TCP_DEFER_ACCEPT = ::T.let(nil, ::T.untyped)
  TCP_FASTOPEN = ::T.let(nil, ::T.untyped)
  TCP_INFO = ::T.let(nil, ::T.untyped)
  TCP_KEEPCNT = ::T.let(nil, ::T.untyped)
  TCP_KEEPIDLE = ::T.let(nil, ::T.untyped)
  TCP_KEEPINTVL = ::T.let(nil, ::T.untyped)
  TCP_LINGER2 = ::T.let(nil, ::T.untyped)
  TCP_MAXSEG = ::T.let(nil, ::T.untyped)
  TCP_MD5SIG = ::T.let(nil, ::T.untyped)
  TCP_NODELAY = ::T.let(nil, ::T.untyped)
  TCP_QUEUE_SEQ = ::T.let(nil, ::T.untyped)
  TCP_QUICKACK = ::T.let(nil, ::T.untyped)
  TCP_REPAIR = ::T.let(nil, ::T.untyped)
  TCP_REPAIR_OPTIONS = ::T.let(nil, ::T.untyped)
  TCP_REPAIR_QUEUE = ::T.let(nil, ::T.untyped)
  TCP_SYNCNT = ::T.let(nil, ::T.untyped)
  TCP_THIN_DUPACK = ::T.let(nil, ::T.untyped)
  TCP_THIN_LINEAR_TIMEOUTS = ::T.let(nil, ::T.untyped)
  TCP_TIMESTAMP = ::T.let(nil, ::T.untyped)
  TCP_USER_TIMEOUT = ::T.let(nil, ::T.untyped)
  TCP_WINDOW_CLAMP = ::T.let(nil, ::T.untyped)
  UDP_CORK = ::T.let(nil, ::T.untyped)

end

class Socket::Ifaddr < Data
  Sorbet.sig {returns(::T.untyped)}
  def addr(); end

  Sorbet.sig {returns(::T.untyped)}
  def broadaddr(); end

  Sorbet.sig {returns(::T.untyped)}
  def dstaddr(); end

  Sorbet.sig {returns(::T.untyped)}
  def flags(); end

  Sorbet.sig {returns(::T.untyped)}
  def ifindex(); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def name(); end

  Sorbet.sig {returns(::T.untyped)}
  def netmask(); end
end

class Socket::Option
  Sorbet.sig {returns(::T.untyped)}
  def bool(); end

  Sorbet.sig {returns(::T.untyped)}
  def byte(); end

  Sorbet.sig {returns(::T.untyped)}
  def data(); end

  Sorbet.sig {returns(::T.untyped)}
  def family(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
      _2: ::T.untyped,
      _3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(_, _1, _2, _3); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def int(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv4_multicast_loop(); end

  Sorbet.sig {returns(::T.untyped)}
  def ipv4_multicast_ttl(); end

  Sorbet.sig {returns(::T.untyped)}
  def level(); end

  Sorbet.sig {returns(::T.untyped)}
  def linger(); end

  Sorbet.sig {returns(::T.untyped)}
  def optname(); end

  Sorbet.sig {returns(::T.untyped)}
  def to_s(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unpack(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
      _2: ::T.untyped,
      _3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.bool(_, _1, _2, _3); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
      _2: ::T.untyped,
      _3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.byte(_, _1, _2, _3); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
      _2: ::T.untyped,
      _3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.int(_, _1, _2, _3); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ipv4_multicast_loop(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ipv4_multicast_ttl(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.linger(_, _1); end
end

class SocketError < StandardError
end


class Socket::UDPSource
  Sorbet.sig do
    params(
      remote_address: ::T.untyped,
      local_address: ::T.untyped,
      reply_proc: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(remote_address, local_address, &reply_proc); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def local_address(); end

  Sorbet.sig {returns(::T.untyped)}
  def remote_address(); end

  Sorbet.sig do
    params(
      msg: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def reply(msg); end
end

class TCPServer < TCPSocket
  extend T::Generic
  Elem = type_member(:out, fixed: String)

  Sorbet.sig {returns(::T.untyped)}
  def accept(); end

  Sorbet.sig do
    params(
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def accept_nonblock(exception: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(_); end

  Sorbet.sig {returns(::T.untyped)}
  def sysaccept(); end
end

class TCPSocket < IPSocket
  extend T::Generic
  Elem = type_member(:out, fixed: String)

  Sorbet.sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
      local_host: ::T.untyped,
      local_port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(host=T.unsafe(nil), port=T.unsafe(nil), local_host=T.unsafe(nil), local_port=T.unsafe(nil)); end

  Sorbet.sig {returns(::T.untyped)}
  def socks_authenticate(); end

  Sorbet.sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def socks_connect(host, port); end

  Sorbet.sig {returns(::T.untyped)}
  def socks_receive_reply(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.gethostbyname(_); end

  Sorbet.sig {returns(::T.untyped)}
  def self.socks_ignores(); end

  Sorbet.sig do
    params(
      ignores: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_ignores=(ignores); end

  Sorbet.sig {returns(::T.untyped)}
  def self.socks_password(); end

  Sorbet.sig do
    params(
      password: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_password=(password); end

  Sorbet.sig {returns(::T.untyped)}
  def self.socks_port(); end

  Sorbet.sig do
    params(
      port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_port=(port); end

  Sorbet.sig {returns(::T.untyped)}
  def self.socks_server(); end

  Sorbet.sig do
    params(
      host: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_server=(host); end

  Sorbet.sig {returns(::T.untyped)}
  def self.socks_username(); end

  Sorbet.sig do
    params(
      username: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_username=(username); end

  Sorbet.sig {returns(::T.untyped)}
  def self.socks_version(); end

  Sorbet.sig do
    params(
      version: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_version=(version); end
end

class UDPSocket < IPSocket
  extend T::Generic
  Elem = type_member(:out, fixed: String)

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def bind(_, _1); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
      _1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect(_, _1); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(*_); end

  Sorbet.sig do
    params(
      len: ::T.untyped,
      flag: ::T.untyped,
      outbuf: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom_nonblock(len, flag=T.unsafe(nil), outbuf=T.unsafe(nil), exception: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def send(*_); end
end

class UNIXServer < UNIXSocket
  Sorbet.sig {returns(::T.untyped)}
  def accept(); end

  Sorbet.sig do
    params(
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def accept_nonblock(exception: T.unsafe(nil)); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(_); end

  Sorbet.sig {returns(::T.untyped)}
  def sysaccept(); end
end

class UNIXSocket < BasicSocket
  extend T::Generic
  Elem = type_member(:out, fixed: String)

  Sorbet.sig {returns(::T.untyped)}
  def addr(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(_); end

  Sorbet.sig {returns(::T.untyped)}
  def path(); end

  Sorbet.sig {returns(::T.untyped)}
  def peeraddr(); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recv_io(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def send_io(_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pair(*_); end

  Sorbet.sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socketpair(*_); end
end
