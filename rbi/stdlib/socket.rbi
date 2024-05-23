# typed: __STDLIB_INTERNAL

# The [`Addrinfo`](https://docs.ruby-lang.org/en/2.6.0/Addrinfo.html) class maps
# `struct addrinfo` to ruby. This structure identifies an Internet host and a
# service.
class Addrinfo < Data
  # returns the address family as an integer.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", 80).afamily == Socket::AF_INET #=> true
  # ```
  sig {returns(::T.untyped)}
  def afamily(); end

  # creates a socket bound to self.
  #
  # If a block is given, it is called with the socket and the value of the block
  # is returned. The socket is returned otherwise.
  #
  # ```ruby
  # Addrinfo.udp("0.0.0.0", 9981).bind {|s|
  #   s.local_address.connect {|s| s.send "hello", 0 }
  #   p s.recv(10) #=> "hello"
  # }
  # ```
  sig {returns(::T.untyped)}
  def bind(); end

  # returns the canonical name as an string.
  #
  # nil is returned if no canonical name.
  #
  # The canonical name is set by
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo)
  # when AI\_CANONNAME is specified.
  #
  # ```ruby
  # list = Addrinfo.getaddrinfo("www.ruby-lang.org", 80, :INET, :STREAM, nil, Socket::AI_CANONNAME)
  # p list[0] #=> #<Addrinfo: 221.186.184.68:80 TCP carbon.ruby-lang.org (www.ruby-lang.org)>
  # p list[0].canonname #=> "carbon.ruby-lang.org"
  # ```
  sig {returns(::T.untyped)}
  def canonname(); end

  # creates a socket connected to the address of self.
  #
  # The optional argument *opts* is options represented by a hash. *opts* may
  # have following options:
  #
  # :timeout
  # :   specify the timeout in seconds.
  #
  #
  # If a block is given, it is called with the socket and the value of the block
  # is returned. The socket is returned otherwise.
  #
  # ```ruby
  # Addrinfo.tcp("www.ruby-lang.org", 80).connect {|s|
  #   s.print "GET / HTTP/1.0\r\nHost: www.ruby-lang.org\r\n\r\n"
  #   puts s.read
  # }
  # ```
  sig do
    params(
      timeout: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect(timeout: T.unsafe(nil), &block); end

  # creates a socket connected to the address of self.
  #
  # If one or more arguments given as *local\_addr\_args*, it is used as the
  # local address of the socket. *local\_addr\_args* is given for
  # [`family_addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-i-family_addrinfo)
  # to obtain actual address.
  #
  # If *local\_addr\_args* is not given, the local address of the socket is not
  # bound.
  #
  # The optional last argument *opts* is options represented by a hash. *opts*
  # may have following options:
  #
  # :timeout
  # :   specify the timeout in seconds.
  #
  #
  # If a block is given, it is called with the socket and the value of the block
  # is returned. The socket is returned otherwise.
  #
  # ```ruby
  # Addrinfo.tcp("www.ruby-lang.org", 80).connect_from("0.0.0.0", 4649) {|s|
  #   s.print "GET / HTTP/1.0\r\nHost: www.ruby-lang.org\r\n\r\n"
  #   puts s.read
  # }
  #
  # # Addrinfo object can be taken for the argument.
  # Addrinfo.tcp("www.ruby-lang.org", 80).connect_from(Addrinfo.tcp("0.0.0.0", 4649)) {|s|
  #   s.print "GET / HTTP/1.0\r\nHost: www.ruby-lang.org\r\n\r\n"
  #   puts s.read
  # }
  # ```
  sig do
    params(
      args: ::T.untyped,
      timeout: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect_from(*args, timeout: T.unsafe(nil), &block); end

  # creates a socket connected to *remote\_addr\_args* and bound to self.
  #
  # The optional last argument *opts* is options represented by a hash. *opts*
  # may have following options:
  #
  # :timeout
  # :   specify the timeout in seconds.
  #
  #
  # If a block is given, it is called with the socket and the value of the block
  # is returned. The socket is returned otherwise.
  #
  # ```ruby
  # Addrinfo.tcp("0.0.0.0", 4649).connect_to("www.ruby-lang.org", 80) {|s|
  #   s.print "GET / HTTP/1.0\r\nHost: www.ruby-lang.org\r\n\r\n"
  #   puts s.read
  # }
  # ```
  sig do
    params(
      args: ::T.untyped,
      timeout: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect_to(*args, timeout: T.unsafe(nil), &block); end

  # creates an [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html)
  # object from the arguments.
  #
  # The arguments are interpreted as similar to self.
  #
  # ```ruby
  # Addrinfo.tcp("0.0.0.0", 4649).family_addrinfo("www.ruby-lang.org", 80)
  # #=> #<Addrinfo: 221.186.184.68:80 TCP (www.ruby-lang.org:80)>
  #
  # Addrinfo.unix("/tmp/sock").family_addrinfo("/tmp/sock2")
  # #=> #<Addrinfo: /tmp/sock2 SOCK_STREAM>
  # ```
  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def family_addrinfo(*args); end

  # returns nodename and service as a pair of strings. This converts struct
  # sockaddr in addrinfo to textual representation.
  #
  # flags should be bitwise OR of Socket::NI\_??? constants.
  #
  # ```ruby
  # Addrinfo.tcp("127.0.0.1", 80).getnameinfo #=> ["localhost", "www"]
  #
  # Addrinfo.tcp("127.0.0.1", 80).getnameinfo(Socket::NI_NUMERICSERV)
  # #=> ["localhost", "80"]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def getnameinfo(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # returns a string which shows addrinfo in human-readable form.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", 80).inspect #=> "#<Addrinfo: 127.0.0.1:80 TCP (localhost)>"
  # Addrinfo.unix("/tmp/sock").inspect    #=> "#<Addrinfo: /tmp/sock SOCK_STREAM>"
  # ```
  sig {returns(::T.untyped)}
  def inspect(); end

  # returns a string which shows the sockaddr in *addrinfo* with human-readable
  # form.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", 80).inspect_sockaddr     #=> "127.0.0.1:80"
  # Addrinfo.tcp("ip6-localhost", 80).inspect_sockaddr #=> "[::1]:80"
  # Addrinfo.unix("/tmp/sock").inspect_sockaddr        #=> "/tmp/sock"
  # ```
  sig {returns(::T.untyped)}
  def inspect_sockaddr(); end

  # returns true if addrinfo is internet (IPv4/IPv6) address. returns false
  # otherwise.
  #
  # ```ruby
  # Addrinfo.tcp("127.0.0.1", 80).ip? #=> true
  # Addrinfo.tcp("::1", 80).ip?       #=> true
  # Addrinfo.unix("/tmp/sock").ip?    #=> false
  # ```
  sig {returns(::T.untyped)}
  def ip?(); end

  # Returns the IP address as a string.
  #
  # ```ruby
  # Addrinfo.tcp("127.0.0.1", 80).ip_address    #=> "127.0.0.1"
  # Addrinfo.tcp("::1", 80).ip_address          #=> "::1"
  # ```
  sig {returns(::T.untyped)}
  def ip_address(); end

  # Returns the port number as an integer.
  #
  # ```ruby
  # Addrinfo.tcp("127.0.0.1", 80).ip_port    #=> 80
  # Addrinfo.tcp("::1", 80).ip_port          #=> 80
  # ```
  sig {returns(::T.untyped)}
  def ip_port(); end

  # Returns the IP address and port number as 2-element array.
  #
  # ```ruby
  # Addrinfo.tcp("127.0.0.1", 80).ip_unpack    #=> ["127.0.0.1", 80]
  # Addrinfo.tcp("::1", 80).ip_unpack          #=> ["::1", 80]
  # ```
  sig {returns(::T.untyped)}
  def ip_unpack(); end

  # returns true if addrinfo is IPv4 address. returns false otherwise.
  #
  # ```ruby
  # Addrinfo.tcp("127.0.0.1", 80).ipv4? #=> true
  # Addrinfo.tcp("::1", 80).ipv4?       #=> false
  # Addrinfo.unix("/tmp/sock").ipv4?    #=> false
  # ```
  sig {returns(::T.untyped)}
  def ipv4?(); end

  # Returns true for IPv4 loopback address (127.0.0.0/8). It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv4_loopback?(); end

  # Returns true for IPv4 multicast address (224.0.0.0/4). It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv4_multicast?(); end

  # Returns true for IPv4 private address (10.0.0.0/8, 172.16.0.0/12,
  # 192.168.0.0/16). It returns false otherwise.
  sig {returns(::T.untyped)}
  def ipv4_private?(); end

  # returns true if addrinfo is IPv6 address. returns false otherwise.
  #
  # ```ruby
  # Addrinfo.tcp("127.0.0.1", 80).ipv6? #=> false
  # Addrinfo.tcp("::1", 80).ipv6?       #=> true
  # Addrinfo.unix("/tmp/sock").ipv6?    #=> false
  # ```
  sig {returns(::T.untyped)}
  def ipv6?(); end

  # Returns true for IPv6 link local address (ff80::/10). It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_linklocal?(); end

  # Returns true for IPv6 loopback address (::1). It returns false otherwise.
  sig {returns(::T.untyped)}
  def ipv6_loopback?(); end

  # Returns true for IPv6 multicast global scope address. It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_mc_global?(); end

  # Returns true for IPv6 multicast link-local scope address. It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_mc_linklocal?(); end

  # Returns true for IPv6 multicast node-local scope address. It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_mc_nodelocal?(); end

  # Returns true for IPv6 multicast organization-local scope address. It returns
  # false otherwise.
  sig {returns(::T.untyped)}
  def ipv6_mc_orglocal?(); end

  # Returns true for IPv6 multicast site-local scope address. It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_mc_sitelocal?(); end

  # Returns true for IPv6 multicast address (ff00::/8). It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_multicast?(); end

  # Returns true for IPv6 site local address (ffc0::/10). It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_sitelocal?(); end

  # Returns IPv4 address of IPv4 mapped/compatible IPv6 address. It returns nil
  # if `self` is not IPv4 mapped/compatible IPv6 address.
  #
  # ```ruby
  # Addrinfo.ip("::192.0.2.3").ipv6_to_ipv4      #=> #<Addrinfo: 192.0.2.3>
  # Addrinfo.ip("::ffff:192.0.2.3").ipv6_to_ipv4 #=> #<Addrinfo: 192.0.2.3>
  # Addrinfo.ip("::1").ipv6_to_ipv4              #=> nil
  # Addrinfo.ip("192.0.2.3").ipv6_to_ipv4        #=> nil
  # Addrinfo.unix("/tmp/sock").ipv6_to_ipv4      #=> nil
  # ```
  sig {returns(::T.untyped)}
  def ipv6_to_ipv4(); end

  # Returns true for IPv6 unique local address (fc00::/7, RFC4193). It returns
  # false otherwise.
  sig {returns(::T.untyped)}
  def ipv6_unique_local?(); end

  # Returns true for IPv6 unspecified address (::). It returns false otherwise.
  sig {returns(::T.untyped)}
  def ipv6_unspecified?(); end

  # Returns true for IPv4-compatible IPv6 address (::/80). It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_v4compat?(); end

  # Returns true for IPv4-mapped IPv6 address (::ffff:0:0/80). It returns false
  # otherwise.
  sig {returns(::T.untyped)}
  def ipv6_v4mapped?(); end

  # creates a listening socket bound to self.
  sig do
    params(
      backlog: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(backlog=T.unsafe(nil)); end

  sig {returns(::T.untyped)}
  def marshal_dump(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def marshal_load(arg0); end

  # returns the protocol family as an integer.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", 80).pfamily == Socket::PF_INET #=> true
  # ```
  sig {returns(::T.untyped)}
  def pfamily(); end

  # returns the socket type as an integer.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", 80).protocol == Socket::IPPROTO_TCP #=> true
  # ```
  sig {returns(::T.untyped)}
  def protocol(); end

  # returns the socket type as an integer.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", 80).socktype == Socket::SOCK_STREAM #=> true
  # ```
  sig {returns(::T.untyped)}
  def socktype(); end

  # returns the socket address as packed struct sockaddr string.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", 80).to_sockaddr
  # #=> "\x02\x00\x00P\x7F\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
  # ```
  sig {returns(::T.untyped)}
  def to_s(); end

  # returns the socket address as packed struct sockaddr string.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", 80).to_sockaddr
  # #=> "\x02\x00\x00P\x7F\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
  # ```
  sig {returns(::T.untyped)}
  def to_sockaddr(); end

  # returns true if addrinfo is UNIX address. returns false otherwise.
  #
  # ```ruby
  # Addrinfo.tcp("127.0.0.1", 80).unix? #=> false
  # Addrinfo.tcp("::1", 80).unix?       #=> false
  # Addrinfo.unix("/tmp/sock").unix?    #=> true
  # ```
  sig {returns(::T.untyped)}
  def unix?(); end

  # Returns the socket path as a string.
  #
  # ```ruby
  # Addrinfo.unix("/tmp/sock").unix_path       #=> "/tmp/sock"
  # ```
  sig {returns(::T.untyped)}
  def unix_path(); end

  # iterates over the list of
  # [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) objects
  # obtained by
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo).
  #
  # ```ruby
  # Addrinfo.foreach(nil, 80) {|x| p x }
  # #=> #<Addrinfo: 127.0.0.1:80 TCP (:80)>
  # #   #<Addrinfo: 127.0.0.1:80 UDP (:80)>
  # #   #<Addrinfo: [::1]:80 TCP (:80)>
  # #   #<Addrinfo: [::1]:80 UDP (:80)>
  # ```
  sig do
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

  # returns a list of addrinfo objects as an array.
  #
  # This method converts nodename (hostname) and service (port) to addrinfo.
  # Since the conversion is not unique, the result is a list of addrinfo
  # objects.
  #
  # nodename or service can be nil if no conversion intended.
  #
  # family, socktype and protocol are hint for preferred protocol. If the result
  # will be used for a socket with SOCK\_STREAM, SOCK\_STREAM should be
  # specified as socktype. If so,
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo)
  # returns addrinfo list appropriate for SOCK\_STREAM. If they are omitted or
  # nil is given, the result is not restricted.
  #
  # Similarly, PF\_INET6 as family restricts for IPv6.
  #
  # flags should be bitwise OR of Socket::AI\_??? constants such as follows.
  # Note that the exact list of the constants depends on OS.
  #
  # ```
  # AI_PASSIVE      Get address to use with bind()
  # AI_CANONNAME    Fill in the canonical name
  # AI_NUMERICHOST  Prevent host name resolution
  # AI_NUMERICSERV  Prevent service name resolution
  # AI_V4MAPPED     Accept IPv4-mapped IPv6 addresses
  # AI_ALL          Allow all addresses
  # AI_ADDRCONFIG   Accept only if any address is assigned
  # ```
  #
  # Note that socktype should be specified whenever application knows the usage
  # of the address. Some platform causes an error when socktype is omitted and
  # servname is specified as an integer because some port numbers, 512 for
  # example, are ambiguous without socktype.
  #
  # ```ruby
  # Addrinfo.getaddrinfo("www.kame.net", 80, nil, :STREAM)
  # #=> [#<Addrinfo: 203.178.141.194:80 TCP (www.kame.net)>,
  # #    #<Addrinfo: [2001:200:dff:fff1:216:3eff:feb1:44d7]:80 TCP (www.kame.net)>]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getaddrinfo(*arg0); end

  # returns an addrinfo object for IP address.
  #
  # The port, socktype, protocol of the result is filled by zero. So, it is not
  # appropriate to create a socket.
  #
  # ```ruby
  # Addrinfo.ip("localhost") #=> #<Addrinfo: 127.0.0.1 (localhost)>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ip(arg0); end

  # returns an addrinfo object for TCP address.
  #
  # ```ruby
  # Addrinfo.tcp("localhost", "smtp") #=> #<Addrinfo: 127.0.0.1:25 TCP (localhost:smtp)>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.tcp(arg0, arg1); end

  # returns an addrinfo object for UDP address.
  #
  # ```ruby
  # Addrinfo.udp("localhost", "daytime") #=> #<Addrinfo: 127.0.0.1:13 UDP (localhost:daytime)>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp(arg0, arg1); end

  # returns an addrinfo object for UNIX socket address.
  #
  # *socktype* specifies the socket type. If it is omitted, :STREAM is used.
  #
  # ```ruby
  # Addrinfo.unix("/tmp/sock")         #=> #<Addrinfo: /tmp/sock SOCK_STREAM>
  # Addrinfo.unix("/tmp/sock", :DGRAM) #=> #<Addrinfo: /tmp/sock SOCK_DGRAM>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix(*arg0); end
end

# [`BasicSocket`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html) is the
# super class for all the
# [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html) classes.
class BasicSocket < IO
  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  # Disallows further read using shutdown system call.
  #
  # ```ruby
  # s1, s2 = UNIXSocket.pair
  # s1.close_read
  # s2.puts #=> Broken pipe (Errno::EPIPE)
  # ```
  sig {returns(::T.untyped)}
  def close_read(); end

  # Disallows further write using shutdown system call.
  #
  # ```ruby
  # UNIXSocket.pair {|s1, s2|
  #   s1.print "ping"
  #   s1.close_write
  #   p s2.read        #=> "ping"
  #   s2.print "pong"
  #   s2.close
  #   p s1.read        #=> "pong"
  # }
  # ```
  sig {returns(::T.untyped)}
  def close_write(); end

  # Returns an address of the socket suitable for connect in the local machine.
  #
  # This method returns *self*.local\_address, except following condition.
  #
  # *   IPv4 unspecified address (0.0.0.0) is replaced by IPv4 loopback address
  #     (127.0.0.1).
  # *   IPv6 unspecified address (::) is replaced by IPv6 loopback address
  #     (::1).
  #
  #
  # If the local address is not suitable for connect,
  # [`SocketError`](https://docs.ruby-lang.org/en/2.7.0/SocketError.html) is
  # raised. IPv4 and IPv6 address which port is 0 is not suitable for connect.
  # Unix domain socket which has no path is not suitable for connect.
  #
  # ```ruby
  # Addrinfo.tcp("0.0.0.0", 0).listen {|serv|
  #   p serv.connect_address #=> #<Addrinfo: 127.0.0.1:53660 TCP>
  #   serv.connect_address.connect {|c|
  #     s, _ = serv.accept
  #     p [c, s] #=> [#<Socket:fd 4>, #<Socket:fd 6>]
  #   }
  # }
  # ```
  sig {returns(::T.untyped)}
  def connect_address(); end

  # Gets the
  # [`do_not_reverse_lookup`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-c-do_not_reverse_lookup)
  # flag of *basicsocket*.
  #
  # ```ruby
  # require 'socket'
  #
  # BasicSocket.do_not_reverse_lookup = false
  # TCPSocket.open("www.ruby-lang.org", 80) {|sock|
  #   p sock.do_not_reverse_lookup      #=> false
  # }
  # BasicSocket.do_not_reverse_lookup = true
  # TCPSocket.open("www.ruby-lang.org", 80) {|sock|
  #   p sock.do_not_reverse_lookup      #=> true
  # }
  # ```
  sig {returns(::T.untyped)}
  def do_not_reverse_lookup(); end

  # Sets the
  # [`do_not_reverse_lookup`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-c-do_not_reverse_lookup)
  # flag of *basicsocket*.
  #
  # ```ruby
  # TCPSocket.open("www.ruby-lang.org", 80) {|sock|
  #   p sock.do_not_reverse_lookup       #=> true
  #   p sock.peeraddr                    #=> ["AF_INET", 80, "221.186.184.68", "221.186.184.68"]
  #   sock.do_not_reverse_lookup = false
  #   p sock.peeraddr                    #=> ["AF_INET", 80, "carbon.ruby-lang.org", "54.163.249.195"]
  # }
  # ```
  sig do
    params(
      do_not_reverse_lookup: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def do_not_reverse_lookup=(do_not_reverse_lookup); end

  # Returns the user and group on the peer of the UNIX socket. The result is a
  # two element array which contains the effective uid and the effective gid.
  #
  # ```ruby
  # Socket.unix_server_loop("/tmp/sock") {|s|
  #   begin
  #     euid, egid = s.getpeereid
  #
  #     # Check the connected client is myself or not.
  #     next if euid != Process.uid
  #
  #     # do something about my resource.
  #
  #   ensure
  #     s.close
  #   end
  # }
  # ```
  sig {returns(::T.untyped)}
  def getpeereid(); end

  # Returns the remote address of the socket as a sockaddr string.
  #
  # ```ruby
  # TCPServer.open("127.0.0.1", 1440) {|serv|
  #   c = TCPSocket.new("127.0.0.1", 1440)
  #   s = serv.accept
  #   p s.getpeername #=> "\x02\x00\x82u\x7F\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
  # }
  # ```
  #
  # If [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object is
  # preferred over the binary string, use
  # [`BasicSocket#remote_address`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-remote_address).
  sig {returns(::T.untyped)}
  def getpeername(); end

  # Returns the local address of the socket as a sockaddr string.
  #
  # ```ruby
  # TCPServer.open("127.0.0.1", 15120) {|serv|
  #   p serv.getsockname #=> "\x02\x00;\x10\x7F\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
  # }
  # ```
  #
  # If [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object is
  # preferred over the binary string, use
  # [`BasicSocket#local_address`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-local_address).
  sig {returns(::T.untyped)}
  def getsockname(); end

  # Gets a socket option. These are protocol and system specific, see your local
  # system documentation for details. The option is returned as a
  # [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
  # object.
  #
  # ### Parameters
  # *   `level` is an integer, usually one of the SOL\_ constants such as
  #     Socket::SOL\_SOCKET, or a protocol level. A string or symbol of the
  #     name, possibly without prefix, is also accepted.
  # *   `optname` is an integer, usually one of the SO\_ constants, such as
  #     Socket::SO\_REUSEADDR. A string or symbol of the name, possibly without
  #     prefix, is also accepted.
  #
  #
  # ### Examples
  #
  # Some socket options are integers with boolean values, in this case
  # [`getsockopt`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-getsockopt)
  # could be called like this:
  #
  # ```ruby
  # reuseaddr = sock.getsockopt(:SOCKET, :REUSEADDR).bool
  #
  # optval = sock.getsockopt(Socket::SOL_SOCKET,Socket::SO_REUSEADDR)
  # optval = optval.unpack "i"
  # reuseaddr = optval[0] == 0 ? false : true
  # ```
  #
  # Some socket options are integers with numeric values, in this case
  # [`getsockopt`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-getsockopt)
  # could be called like this:
  #
  # ```ruby
  # ipttl = sock.getsockopt(:IP, :TTL).int
  #
  # optval = sock.getsockopt(Socket::IPPROTO_IP, Socket::IP_TTL)
  # ipttl = optval.unpack("i")[0]
  # ```
  #
  # Option values may be structs. Decoding them can be complex as it involves
  # examining your system headers to determine the correct definition. An
  # example is a +struct linger+, which may be defined in your system headers
  # as:
  #
  # ```ruby
  # struct linger {
  #   int l_onoff;
  #   int l_linger;
  # };
  # ```
  #
  # In this case
  # [`getsockopt`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-getsockopt)
  # could be called like this:
  #
  # ```ruby
  # # Socket::Option knows linger structure.
  # onoff, linger = sock.getsockopt(:SOCKET, :LINGER).linger
  #
  # optval =  sock.getsockopt(Socket::SOL_SOCKET, Socket::SO_LINGER)
  # onoff, linger = optval.unpack "ii"
  # onoff = onoff == 0 ? false : true
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def getsockopt(arg0, arg1); end

  # Returns an [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html)
  # object for local address obtained by getsockname.
  #
  # Note that addrinfo.protocol is filled by 0.
  #
  # ```ruby
  # TCPSocket.open("www.ruby-lang.org", 80) {|s|
  #   p s.local_address #=> #<Addrinfo: 192.168.0.129:36873 TCP>
  # }
  #
  # TCPServer.open("127.0.0.1", 1512) {|serv|
  #   p serv.local_address #=> #<Addrinfo: 127.0.0.1:1512 TCP>
  # }
  # ```
  sig {returns(::T.untyped)}
  def local_address(); end

  # Receives a message.
  #
  # *maxlen* is the maximum number of bytes to receive.
  #
  # *flags* should be a bitwise OR of Socket::MSG\_\* constants.
  #
  # *outbuf* will contain only the received data after the method call even if
  # it is not empty at the beginning.
  #
  # ```ruby
  # UNIXSocket.pair {|s1, s2|
  #   s1.puts "Hello World"
  #   p s2.recv(4)                     #=> "Hell"
  #   p s2.recv(4, Socket::MSG_PEEK)   #=> "o Wo"
  #   p s2.recv(4)                     #=> "o Wo"
  #   p s2.recv(10)                    #=> "rld\n"
  # }
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recv(*arg0); end

  # Receives up to *maxlen* bytes from `socket` using recvfrom(2) after
  # O\_NONBLOCK is set for the underlying file descriptor. *flags* is zero or
  # more of the `MSG_` options. The result, *mesg*, is the data received.
  #
  # When recvfrom(2) returns 0,
  # [`Socket#recv_nonblock`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-recv_nonblock)
  # returns an empty string as data. The meaning depends on the socket: EOF on
  # TCP, empty packet on UDP, etc.
  #
  # ### Parameters
  # *   `maxlen` - the number of bytes to receive from the socket
  # *   `flags` - zero or more of the `MSG_` options
  # *   `buf` - destination
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) buffer
  # *   `options` - keyword hash, supporting `exception: false`
  #
  #
  # ### Example
  #
  # ```ruby
  # serv = TCPServer.new("127.0.0.1", 0)
  # af, port, host, addr = serv.addr
  # c = TCPSocket.new(addr, port)
  # s = serv.accept
  # c.send "aaa", 0
  # begin # emulate blocking recv.
  #   p s.recv_nonblock(10) #=> "aaa"
  # rescue IO::WaitReadable
  #   IO.select([s])
  #   retry
  # end
  # ```
  #
  # Refer to
  # [`Socket#recvfrom`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom)
  # for the exceptions that may be thrown if the call to *recv\_nonblock* fails.
  #
  # [`BasicSocket#recv_nonblock`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-recv_nonblock)
  # may raise any error corresponding to recvfrom(2) failure, including
  # Errno::EWOULDBLOCK.
  #
  # If the exception is Errno::EWOULDBLOCK or Errno::EAGAIN, it is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html).
  # So
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # can be used to rescue the exceptions for retrying recv\_nonblock.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`recv_nonblock`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-recv_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead.
  #
  # ### See
  # *   [`Socket#recvfrom`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom)
  sig do
    params(
      len: ::T.untyped,
      flag: ::T.untyped,
      str: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recv_nonblock(len, flag=T.unsafe(nil), str=T.unsafe(nil), exception: T.unsafe(nil)); end

  # recvmsg receives a message using recvmsg(2) system call in blocking manner.
  #
  # *maxmesglen* is the maximum length of mesg to receive.
  #
  # *flags* is bitwise OR of MSG\_\* constants such as Socket::MSG\_PEEK.
  #
  # *maxcontrollen* is the maximum length of controls (ancillary data) to
  # receive.
  #
  # *opts* is option hash. Currently :scm\_rights=>bool is the only option.
  #
  # :scm\_rights option specifies that application expects SCM\_RIGHTS control
  # message. If the value is nil or false, application don't expects SCM\_RIGHTS
  # control message. In this case, recvmsg closes the passed file descriptors
  # immediately. This is the default behavior.
  #
  # If :scm\_rights value is neither nil nor false, application expects
  # SCM\_RIGHTS control message. In this case, recvmsg creates
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects for each file
  # descriptors for
  # [`Socket::AncillaryData#unix_rights`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html#method-i-unix_rights)
  # method.
  #
  # The return value is 4-elements array.
  #
  # *mesg* is a string of the received message.
  #
  # *sender\_addrinfo* is a sender socket address for connection-less socket. It
  # is an [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html)
  # object. For connection-oriented socket such as TCP, sender\_addrinfo is
  # platform dependent.
  #
  # *rflags* is a flags on the received message which is bitwise OR of MSG\_\*
  # constants such as Socket::MSG\_TRUNC. It will be nil if the system uses
  # 4.3BSD style old recvmsg system call.
  #
  # *controls* is ancillary data which is an array of
  # [`Socket::AncillaryData`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html)
  # objects such as:
  #
  # ```ruby
  # #<Socket::AncillaryData: AF_UNIX SOCKET RIGHTS 7>
  # ```
  #
  # *maxmesglen* and *maxcontrollen* can be nil. In that case, the buffer will
  # be grown until the message is not truncated. Internally, MSG\_PEEK is used.
  # Buffer full and MSG\_CTRUNC are checked for truncation.
  #
  # recvmsg can be used to implement recv\_io as follows:
  #
  # ```ruby
  # mesg, sender_sockaddr, rflags, *controls = sock.recvmsg(:scm_rights=>true)
  # controls.each {|ancdata|
  #   if ancdata.cmsg_is?(:SOCKET, :RIGHTS)
  #     return ancdata.unix_rights[0]
  #   end
  # }
  # ```
  sig do
    params(
      dlen: ::T.untyped,
      flags: ::T.untyped,
      clen: ::T.untyped,
      scm_rights: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvmsg(dlen=T.unsafe(nil), flags=T.unsafe(nil), clen=T.unsafe(nil), scm_rights: T.unsafe(nil)); end

  # recvmsg receives a message using recvmsg(2) system call in non-blocking
  # manner.
  #
  # It is similar to
  # [`BasicSocket#recvmsg`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-recvmsg)
  # but non-blocking flag is set before the system call and it doesn't retry the
  # system call.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`recvmsg_nonblock`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-recvmsg_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead.
  sig do
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

  # Returns an [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html)
  # object for remote address obtained by getpeername.
  #
  # Note that addrinfo.protocol is filled by 0.
  #
  # ```ruby
  # TCPSocket.open("www.ruby-lang.org", 80) {|s|
  #   p s.remote_address #=> #<Addrinfo: 221.186.184.68:80 TCP>
  # }
  #
  # TCPServer.open("127.0.0.1", 1728) {|serv|
  #   c = TCPSocket.new("127.0.0.1", 1728)
  #   s = serv.accept
  #   p s.remote_address #=> #<Addrinfo: 127.0.0.1:36504 TCP>
  # }
  # ```
  sig {returns(::T.untyped)}
  def remote_address(); end

  # send *mesg* via *basicsocket*.
  #
  # *mesg* should be a string.
  #
  # *flags* should be a bitwise OR of Socket::MSG\_\* constants.
  #
  # *dest\_sockaddr* should be a packed sockaddr string or an addrinfo.
  #
  # ```ruby
  # TCPSocket.open("localhost", 80) {|s|
  #   s.send "GET / HTTP/1.0\r\n\r\n", 0
  #   p s.read
  # }
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def send(*arg0); end

  # sendmsg sends a message using sendmsg(2) system call in blocking manner.
  #
  # *mesg* is a string to send.
  #
  # *flags* is bitwise OR of MSG\_\* constants such as Socket::MSG\_OOB.
  #
  # *dest\_sockaddr* is a destination socket address for connection-less socket.
  # It should be a sockaddr such as a result of
  # [`Socket.sockaddr_in`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-sockaddr_in).
  # An [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object
  # can be used too.
  #
  # *controls* is a list of ancillary data. The element of *controls* should be
  # [`Socket::AncillaryData`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html)
  # or 3-elements array. The 3-element array should contains cmsg\_level,
  # cmsg\_type and data.
  #
  # The return value, *numbytes\_sent* is an integer which is the number of
  # bytes sent.
  #
  # sendmsg can be used to implement send\_io as follows:
  #
  # ```ruby
  # # use Socket::AncillaryData.
  # ancdata = Socket::AncillaryData.int(:UNIX, :SOCKET, :RIGHTS, io.fileno)
  # sock.sendmsg("a", 0, nil, ancdata)
  #
  # # use 3-element array.
  # ancdata = [:SOCKET, :RIGHTS, [io.fileno].pack("i!")]
  # sock.sendmsg("\0", 0, nil, ancdata)
  # ```
  sig do
    params(
      mesg: ::T.untyped,
      flags: ::T.untyped,
      dest_sockaddr: ::T.untyped,
      controls: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sendmsg(mesg, flags=T.unsafe(nil), dest_sockaddr=T.unsafe(nil), *controls); end

  # [`sendmsg_nonblock`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-sendmsg_nonblock)
  # sends a message using sendmsg(2) system call in non-blocking manner.
  #
  # It is similar to
  # [`BasicSocket#sendmsg`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-sendmsg)
  # but the non-blocking flag is set before the system call and it doesn't retry
  # the system call.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`sendmsg_nonblock`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-sendmsg_nonblock)
  # should not raise an
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # exception, but return the symbol `:wait_writable` instead.
  sig do
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

  # Sets a socket option. These are protocol and system specific, see your local
  # system documentation for details.
  #
  # ### Parameters
  # *   `level` is an integer, usually one of the SOL\_ constants such as
  #     Socket::SOL\_SOCKET, or a protocol level. A string or symbol of the
  #     name, possibly without prefix, is also accepted.
  # *   `optname` is an integer, usually one of the SO\_ constants, such as
  #     Socket::SO\_REUSEADDR. A string or symbol of the name, possibly without
  #     prefix, is also accepted.
  # *   `optval` is the value of the option, it is passed to the underlying
  #     setsockopt() as a pointer to a certain number of bytes. How this is done
  #     depends on the type:
  #     *   Integer: value is assigned to an int, and a pointer to the int is
  #         passed, with length of sizeof(int).
  #     *   true or false: 1 or 0 (respectively) is assigned to an int, and the
  #         int is passed as for an
  #         [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html). Note
  #         that `false` must be passed, not `nil`.
  #     *   String: the string's data and length is passed to the socket.
  #
  # *   `socketoption` is an instance of
  #     [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
  #
  #
  # ### Examples
  #
  # Some socket options are integers with boolean values, in this case
  # [`setsockopt`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-setsockopt)
  # could be called like this:
  #
  # ```ruby
  # sock.setsockopt(:SOCKET, :REUSEADDR, true)
  # sock.setsockopt(Socket::SOL_SOCKET,Socket::SO_REUSEADDR, true)
  # sock.setsockopt(Socket::Option.bool(:INET, :SOCKET, :REUSEADDR, true))
  # ```
  #
  # Some socket options are integers with numeric values, in this case
  # [`setsockopt`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-setsockopt)
  # could be called like this:
  #
  # ```ruby
  # sock.setsockopt(:IP, :TTL, 255)
  # sock.setsockopt(Socket::IPPROTO_IP, Socket::IP_TTL, 255)
  # sock.setsockopt(Socket::Option.int(:INET, :IP, :TTL, 255))
  # ```
  #
  # Option values may be structs. Passing them can be complex as it involves
  # examining your system headers to determine the correct definition. An
  # example is an `ip_mreq`, which may be defined in your system headers as:
  #
  # ```ruby
  # struct ip_mreq {
  #   struct  in_addr imr_multiaddr;
  #   struct  in_addr imr_interface;
  # };
  # ```
  #
  # In this case
  # [`setsockopt`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-setsockopt)
  # could be called like this:
  #
  # ```ruby
  # optval = IPAddr.new("224.0.0.251").hton +
  #          IPAddr.new(Socket::INADDR_ANY, Socket::AF_INET).hton
  # sock.setsockopt(Socket::IPPROTO_IP, Socket::IP_ADD_MEMBERSHIP, optval)
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def setsockopt(*arg0); end

  # Calls shutdown(2) system call.
  #
  # s.shutdown(Socket::SHUT\_RD) disallows further read.
  #
  # s.shutdown(Socket::SHUT\_WR) disallows further write.
  #
  # s.shutdown(Socket::SHUT\_RDWR) disallows further read and write.
  #
  # *how* can be symbol or string:
  # *   :RD, :SHUT\_RD, "RD" and "SHUT\_RD" are accepted as Socket::SHUT\_RD.
  # *   :WR, :SHUT\_WR, "WR" and "SHUT\_WR" are accepted as Socket::SHUT\_WR.
  # *   :RDWR, :SHUT\_RDWR, "RDWR" and "SHUT\_RDWR" are accepted as
  #     Socket::SHUT\_RDWR.
  #
  #     [`UNIXSocket.pair`](https://docs.ruby-lang.org/en/2.7.0/UNIXSocket.html#method-c-pair)
  #     {|s1, s2|
  #
  # ```ruby
  # s1.puts "ping"
  # s1.shutdown(:WR)
  # p s2.read          #=> "ping\n"
  # s2.puts "pong"
  # s2.close
  # p s1.read          #=> "pong\n"
  # ```
  #
  #     }
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def shutdown(*arg0); end

  # Gets the global
  # [`do_not_reverse_lookup`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-c-do_not_reverse_lookup)
  # flag.
  #
  # ```ruby
  # BasicSocket.do_not_reverse_lookup  #=> false
  # ```
  sig {returns(::T.untyped)}
  def self.do_not_reverse_lookup(); end

  # Sets the global
  # [`do_not_reverse_lookup`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-c-do_not_reverse_lookup)
  # flag.
  #
  # The flag is used for initial value of
  # [`do_not_reverse_lookup`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-c-do_not_reverse_lookup)
  # for each socket.
  #
  # ```ruby
  # s1 = TCPSocket.new("localhost", 80)
  # p s1.do_not_reverse_lookup                 #=> true
  # BasicSocket.do_not_reverse_lookup = false
  # s2 = TCPSocket.new("localhost", 80)
  # p s2.do_not_reverse_lookup                 #=> false
  # p s1.do_not_reverse_lookup                 #=> true
  # ```
  sig do
    params(
      do_not_reverse_lookup: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.do_not_reverse_lookup=(do_not_reverse_lookup); end

  # Returns a socket object which contains the file descriptor, *fd*.
  #
  # ```ruby
  # # If invoked by inetd, STDIN/STDOUT/STDERR is a socket.
  # STDIN_SOCK = Socket.for_fd(STDIN.fileno)
  # p STDIN_SOCK.remote_address
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.for_fd(arg0); end
end

# [`IPSocket`](https://docs.ruby-lang.org/en/2.7.0/IPSocket.html) is the super
# class of [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html) and
# [`UDPSocket`](https://docs.ruby-lang.org/en/2.7.0/UDPSocket.html).
class IPSocket < BasicSocket
  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  # Returns the local address as an array which contains address\_family, port,
  # hostname and numeric\_address.
  #
  # If `reverse_lookup` is `true` or `:hostname`, hostname is obtained from
  # numeric\_address using reverse lookup. Or if it is `false`, or `:numeric`,
  # hostname is same as numeric\_address. Or if it is `nil` or omitted, obeys to
  # `ipsocket.do_not_reverse_lookup`. See `Socket.getaddrinfo` also.
  #
  # ```ruby
  # TCPSocket.open("www.ruby-lang.org", 80) {|sock|
  #   p sock.addr #=> ["AF_INET", 49429, "hal", "192.168.0.128"]
  #   p sock.addr(true)  #=> ["AF_INET", 49429, "hal", "192.168.0.128"]
  #   p sock.addr(false) #=> ["AF_INET", 49429, "192.168.0.128", "192.168.0.128"]
  #   p sock.addr(:hostname)  #=> ["AF_INET", 49429, "hal", "192.168.0.128"]
  #   p sock.addr(:numeric)   #=> ["AF_INET", 49429, "192.168.0.128", "192.168.0.128"]
  # }
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def addr(*arg0); end

  # Returns the remote address as an array which contains address\_family, port,
  # hostname and numeric\_address. It is defined for connection oriented socket
  # such as [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html).
  #
  # If `reverse_lookup` is `true` or `:hostname`, hostname is obtained from
  # numeric\_address using reverse lookup. Or if it is `false`, or `:numeric`,
  # hostname is same as numeric\_address. Or if it is `nil` or omitted, obeys to
  # `ipsocket.do_not_reverse_lookup`. See `Socket.getaddrinfo` also.
  #
  # ```ruby
  # TCPSocket.open("www.ruby-lang.org", 80) {|sock|
  #   p sock.peeraddr #=> ["AF_INET", 80, "carbon.ruby-lang.org", "221.186.184.68"]
  #   p sock.peeraddr(true)  #=> ["AF_INET", 80, "carbon.ruby-lang.org", "221.186.184.68"]
  #   p sock.peeraddr(false) #=> ["AF_INET", 80, "221.186.184.68", "221.186.184.68"]
  #   p sock.peeraddr(:hostname) #=> ["AF_INET", 80, "carbon.ruby-lang.org", "221.186.184.68"]
  #   p sock.peeraddr(:numeric)  #=> ["AF_INET", 80, "221.186.184.68", "221.186.184.68"]
  # }
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def peeraddr(*arg0); end

  # Receives a message and return the message as a string and an address which
  # the message come from.
  #
  # *maxlen* is the maximum number of bytes to receive.
  #
  # *flags* should be a bitwise OR of Socket::MSG\_\* constants.
  #
  # ipaddr is same as IPSocket#{peeraddr,addr}.
  #
  # ```ruby
  # u1 = UDPSocket.new
  # u1.bind("127.0.0.1", 4913)
  # u2 = UDPSocket.new
  # u2.send "uuuu", 0, "127.0.0.1", 4913
  # p u1.recvfrom(10) #=> ["uuuu", ["AF_INET", 33230, "localhost", "127.0.0.1"]]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom(*arg0); end

  # Lookups the IP address of *host*.
  #
  # ```ruby
  # require 'socket'
  #
  # IPSocket.getaddress("localhost")     #=> "127.0.0.1"
  # IPSocket.getaddress("ip6-localhost") #=> "::1"
  # ```
  #
  #
  # Also aliased as:
  # [`getaddress_orig`](https://docs.ruby-lang.org/en/2.7.0/IPSocket.html#method-c-getaddress_orig)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getaddress(arg0); end
end

# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) `Socket` provides
# access to the underlying operating system socket implementations. It can be
# used to provide more operating system specific functionality than the
# protocol-specific socket classes.
#
# The constants defined under
# [`Socket::Constants`](https://docs.ruby-lang.org/en/2.7.0/Socket/Constants.html)
# are also defined under
# [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html). For example,
# [`Socket::AF_INET`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#AF_INET)
# is usable as well as Socket::Constants::AF\_INET. See
# [`Socket::Constants`](https://docs.ruby-lang.org/en/2.7.0/Socket/Constants.html)
# for the list of constants.
#
# ### What's a socket?
#
# Sockets are endpoints of a bidirectional communication channel. Sockets can
# communicate within a process, between processes on the same machine or between
# different machines. There are many types of socket:
# [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html),
# [`UDPSocket`](https://docs.ruby-lang.org/en/2.7.0/UDPSocket.html) or
# [`UNIXSocket`](https://docs.ruby-lang.org/en/2.7.0/UNIXSocket.html) for
# example.
#
# Sockets have their own vocabulary:
#
# **domain:** The family of protocols:
# *   [`Socket::PF_INET`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#PF_INET)
# *   [`Socket::PF_INET6`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#PF_INET6)
# *   [`Socket::PF_UNIX`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#PF_UNIX)
# *   etc.
#
#
# **type:** The type of communications between the two endpoints, typically
# *   [`Socket::SOCK_STREAM`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#SOCK_STREAM)
# *   [`Socket::SOCK_DGRAM`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#SOCK_DGRAM).
#
#
# **protocol:** Typically *zero*. This may be used to identify a variant of a
# protocol.
#
# **hostname:** The identifier of a network interface:
# *   a string (hostname, IPv4 or IPv6 address or `broadcast` which specifies a
#     broadcast address)
# *   a zero-length string which specifies
#     [`INADDR_ANY`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#INADDR_ANY)
# *   an integer (interpreted as binary address in host byte order).
#
#
# ### Quick start
#
# Many of the classes, such as
# [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html),
# [`UDPSocket`](https://docs.ruby-lang.org/en/2.7.0/UDPSocket.html) or
# [`UNIXSocket`](https://docs.ruby-lang.org/en/2.7.0/UNIXSocket.html), ease the
# use of sockets comparatively to the equivalent C programming interface.
#
# Let's create an internet socket using the IPv4 protocol in a C-like manner:
#
# ```ruby
# require 'socket'
#
# s = Socket.new Socket::AF_INET, Socket::SOCK_STREAM
# s.connect Socket.pack_sockaddr_in(80, 'example.com')
# ```
#
# You could also use the
# [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html) class:
#
# ```ruby
# s = TCPSocket.new 'example.com', 80
# ```
#
# A simple server might look like this:
#
# ```ruby
# require 'socket'
#
# server = TCPServer.new 2000 # Server bound to port 2000
#
# loop do
#   client = server.accept    # Wait for a client to connect
#   client.puts "Hello !"
#   client.puts "Time is #{Time.now}"
#   client.close
# end
# ```
#
# A simple client may look like this:
#
# ```ruby
# require 'socket'
#
# s = TCPSocket.new 'localhost', 2000
#
# while line = s.gets # Read lines from socket
#   puts line         # and print them
# end
#
# s.close             # close socket when done
# ```
#
# ### [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) Handling
#
# Ruby's [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html)
# implementation raises exceptions based on the error generated by the system
# dependent implementation. This is why the methods are documented in a way that
# isolate Unix-based system exceptions from Windows based exceptions. If more
# information on a particular exception is needed, please refer to the Unix
# manual pages or the Windows WinSock reference.
#
# ### Convenience methods
#
# Although the general way to create socket is
# [`Socket.new`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-new),
# there are several methods of socket creation for most cases.
#
# TCP client socket
# :   [`Socket.tcp`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-tcp),
#     [`TCPSocket.open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open)
# TCP server socket
# :   [`Socket.tcp_server_loop`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-tcp_server_loop),
#     [`TCPServer.open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open)
# UNIX client socket
# :   [`Socket.unix`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-unix),
#     [`UNIXSocket.open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open)
# UNIX server socket
# :   [`Socket.unix_server_loop`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-unix_server_loop),
#     [`UNIXServer.open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open)
#
#
# ### Documentation by
#
# *   Zach Dennis
# *   Sam Roberts
# *   *Programming Ruby* from The Pragmatic Bookshelf.
#
#
# Much material in this documentation is taken with permission from *Programming
# Ruby* from The Pragmatic Bookshelf.
class Socket < BasicSocket
  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  # AppleTalk protocol
  AF_APPLETALK = ::T.let(nil, ::T.untyped)
  # AX.25 protocol
  AF_AX25 = ::T.let(nil, ::T.untyped)
  # IPv4 protocol
  AF_INET = ::T.let(nil, ::T.untyped)
  # IPv6 protocol
  AF_INET6 = ::T.let(nil, ::T.untyped)
  # IPX protocol
  AF_IPX = ::T.let(nil, ::T.untyped)
  # Integrated Services Digital Network
  AF_ISDN = ::T.let(nil, ::T.untyped)
  # Host-internal protocols
  AF_LOCAL = ::T.let(nil, ::T.untyped)
  # Maximum address family for this platform
  AF_MAX = ::T.let(nil, ::T.untyped)
  # Direct link-layer access
  AF_PACKET = ::T.let(nil, ::T.untyped)
  # Internal routing protocol
  AF_ROUTE = ::T.let(nil, ::T.untyped)
  # IBM SNA protocol
  AF_SNA = ::T.let(nil, ::T.untyped)
  # UNIX sockets
  AF_UNIX = ::T.let(nil, ::T.untyped)
  # Unspecified protocol, any supported address family
  AF_UNSPEC = ::T.let(nil, ::T.untyped)
  # Accept only if any address is assigned
  AI_ADDRCONFIG = ::T.let(nil, ::T.untyped)
  # Allow all addresses
  AI_ALL = ::T.let(nil, ::T.untyped)
  # Fill in the canonical name
  AI_CANONNAME = ::T.let(nil, ::T.untyped)
  # Prevent host name resolution
  AI_NUMERICHOST = ::T.let(nil, ::T.untyped)
  # Prevent service name resolution
  AI_NUMERICSERV = ::T.let(nil, ::T.untyped)
  # Get address to use with bind()
  AI_PASSIVE = ::T.let(nil, ::T.untyped)
  # Accept IPv4-mapped IPv6 addresses
  AI_V4MAPPED = ::T.let(nil, ::T.untyped)
  # Address family for hostname not supported
  EAI_ADDRFAMILY = ::T.let(nil, ::T.untyped)
  # Temporary failure in name resolution
  EAI_AGAIN = ::T.let(nil, ::T.untyped)
  # Invalid flags
  EAI_BADFLAGS = ::T.let(nil, ::T.untyped)
  # Non-recoverable failure in name resolution
  EAI_FAIL = ::T.let(nil, ::T.untyped)
  # Address family not supported
  EAI_FAMILY = ::T.let(nil, ::T.untyped)
  # Memory allocation failure
  EAI_MEMORY = ::T.let(nil, ::T.untyped)
  # No address associated with hostname
  EAI_NODATA = ::T.let(nil, ::T.untyped)
  # Hostname nor servname, or not known
  EAI_NONAME = ::T.let(nil, ::T.untyped)
  # Argument buffer overflow
  EAI_OVERFLOW = ::T.let(nil, ::T.untyped)
  # Servname not supported for socket type
  EAI_SERVICE = ::T.let(nil, ::T.untyped)
  # [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html) type not
  # supported
  EAI_SOCKTYPE = ::T.let(nil, ::T.untyped)
  # System error returned in errno
  EAI_SYSTEM = ::T.let(nil, ::T.untyped)
  # receive all multicast packets
  IFF_ALLMULTI = ::T.let(nil, ::T.untyped)
  # auto media select active
  IFF_AUTOMEDIA = ::T.let(nil, ::T.untyped)
  # broadcast address valid
  IFF_BROADCAST = ::T.let(nil, ::T.untyped)
  # turn on debugging
  IFF_DEBUG = ::T.let(nil, ::T.untyped)
  # dialup device with changing addresses
  IFF_DYNAMIC = ::T.let(nil, ::T.untyped)
  # loopback net
  IFF_LOOPBACK = ::T.let(nil, ::T.untyped)
  # master of a load balancer
  IFF_MASTER = ::T.let(nil, ::T.untyped)
  # supports multicast
  IFF_MULTICAST = ::T.let(nil, ::T.untyped)
  # no address resolution protocol
  IFF_NOARP = ::T.let(nil, ::T.untyped)
  # avoid use of trailers
  IFF_NOTRAILERS = ::T.let(nil, ::T.untyped)
  # point-to-point link
  IFF_POINTOPOINT = ::T.let(nil, ::T.untyped)
  # can set media type
  IFF_PORTSEL = ::T.let(nil, ::T.untyped)
  # receive all packets
  IFF_PROMISC = ::T.let(nil, ::T.untyped)
  # resources allocated
  IFF_RUNNING = ::T.let(nil, ::T.untyped)
  # slave of a load balancer
  IFF_SLAVE = ::T.let(nil, ::T.untyped)
  # interface is up
  IFF_UP = ::T.let(nil, ::T.untyped)
  # Maximum interface name size
  IFNAMSIZ = ::T.let(nil, ::T.untyped)
  # Maximum interface name size
  IF_NAMESIZE = ::T.let(nil, ::T.untyped)
  # Multicast group for all systems on this subset
  INADDR_ALLHOSTS_GROUP = ::T.let(nil, ::T.untyped)
  # A socket bound to
  # [`INADDR_ANY`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#INADDR_ANY)
  # receives packets from all interfaces and sends from the default IP address
  INADDR_ANY = ::T.let(nil, ::T.untyped)
  # The network broadcast address
  INADDR_BROADCAST = ::T.let(nil, ::T.untyped)
  # The loopback address
  INADDR_LOOPBACK = ::T.let(nil, ::T.untyped)
  # The last local network multicast group
  INADDR_MAX_LOCAL_GROUP = ::T.let(nil, ::T.untyped)
  # A bitmask for matching no valid IP address
  INADDR_NONE = ::T.let(nil, ::T.untyped)
  # The reserved multicast group
  INADDR_UNSPEC_GROUP = ::T.let(nil, ::T.untyped)
  # Maximum length of an IPv6 address string
  INET6_ADDRSTRLEN = ::T.let(nil, ::T.untyped)
  # Maximum length of an IPv4 address string
  INET_ADDRSTRLEN = ::T.let(nil, ::T.untyped)
  # Default minimum address for bind or connect
  IPPORT_RESERVED = ::T.let(nil, ::T.untyped)
  # Default maximum address for bind or connect
  IPPORT_USERRESERVED = ::T.let(nil, ::T.untyped)
  # IP6 auth header
  IPPROTO_AH = ::T.let(nil, ::T.untyped)
  # IP6 destination option
  IPPROTO_DSTOPTS = ::T.let(nil, ::T.untyped)
  # Exterior Gateway Protocol
  IPPROTO_EGP = ::T.let(nil, ::T.untyped)
  # IP6 Encapsulated Security Payload
  IPPROTO_ESP = ::T.let(nil, ::T.untyped)
  # IP6 fragmentation header
  IPPROTO_FRAGMENT = ::T.let(nil, ::T.untyped)
  # IP6 hop-by-hop options
  IPPROTO_HOPOPTS = ::T.let(nil, ::T.untyped)
  # Control message protocol
  IPPROTO_ICMP = ::T.let(nil, ::T.untyped)
  # ICMP6
  IPPROTO_ICMPV6 = ::T.let(nil, ::T.untyped)
  # XNS IDP
  IPPROTO_IDP = ::T.let(nil, ::T.untyped)
  # Group Management Protocol
  IPPROTO_IGMP = ::T.let(nil, ::T.untyped)
  # Dummy protocol for IP
  IPPROTO_IP = ::T.let(nil, ::T.untyped)
  # IP6 header
  IPPROTO_IPV6 = ::T.let(nil, ::T.untyped)
  # IP6 no next header
  IPPROTO_NONE = ::T.let(nil, ::T.untyped)
  # PARC Universal Packet protocol
  IPPROTO_PUP = ::T.let(nil, ::T.untyped)
  # Raw IP packet
  IPPROTO_RAW = ::T.let(nil, ::T.untyped)
  # IP6 routing header
  IPPROTO_ROUTING = ::T.let(nil, ::T.untyped)
  # TCP
  IPPROTO_TCP = ::T.let(nil, ::T.untyped)
  # ISO transport protocol class 4
  IPPROTO_TP = ::T.let(nil, ::T.untyped)
  # UDP
  IPPROTO_UDP = ::T.let(nil, ::T.untyped)
  # Checksum offset for raw sockets
  IPV6_CHECKSUM = ::T.let(nil, ::T.untyped)
  # Destination option
  IPV6_DSTOPTS = ::T.let(nil, ::T.untyped)
  # Hop limit
  IPV6_HOPLIMIT = ::T.let(nil, ::T.untyped)
  # Hop-by-hop option
  IPV6_HOPOPTS = ::T.let(nil, ::T.untyped)
  # Join a group membership
  IPV6_JOIN_GROUP = ::T.let(nil, ::T.untyped)
  # Leave a group membership
  IPV6_LEAVE_GROUP = ::T.let(nil, ::T.untyped)
  # IP6 multicast hops
  IPV6_MULTICAST_HOPS = ::T.let(nil, ::T.untyped)
  # IP6 multicast interface
  IPV6_MULTICAST_IF = ::T.let(nil, ::T.untyped)
  # IP6 multicast loopback
  IPV6_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  # Next hop address
  IPV6_NEXTHOP = ::T.let(nil, ::T.untyped)
  # Receive packet information with datagram
  IPV6_PKTINFO = ::T.let(nil, ::T.untyped)
  # Receive all IP6 options for response
  IPV6_RECVDSTOPTS = ::T.let(nil, ::T.untyped)
  # Receive hop limit with datagram
  IPV6_RECVHOPLIMIT = ::T.let(nil, ::T.untyped)
  # Receive hop-by-hop options
  IPV6_RECVHOPOPTS = ::T.let(nil, ::T.untyped)
  # Receive destination IP address and incoming interface
  IPV6_RECVPKTINFO = ::T.let(nil, ::T.untyped)
  # Receive routing header
  IPV6_RECVRTHDR = ::T.let(nil, ::T.untyped)
  # Receive traffic class
  IPV6_RECVTCLASS = ::T.let(nil, ::T.untyped)
  # Allows removal of sticky routing headers
  IPV6_RTHDR = ::T.let(nil, ::T.untyped)
  # Allows removal of sticky destination options header
  IPV6_RTHDRDSTOPTS = ::T.let(nil, ::T.untyped)
  # Routing header type 0
  IPV6_RTHDR_TYPE_0 = ::T.let(nil, ::T.untyped)
  # Specify the traffic class
  IPV6_TCLASS = ::T.let(nil, ::T.untyped)
  # IP6 unicast hops
  IPV6_UNICAST_HOPS = ::T.let(nil, ::T.untyped)
  # Only bind IPv6 with a wildcard bind
  IPV6_V6ONLY = ::T.let(nil, ::T.untyped)
  # Add a multicast group membership
  IP_ADD_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  # Add a multicast group membership
  IP_ADD_SOURCE_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  # Block IPv4 multicast packets with a give source address
  IP_BLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  # Default multicast loopback
  IP_DEFAULT_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  # Default multicast TTL
  IP_DEFAULT_MULTICAST_TTL = ::T.let(nil, ::T.untyped)
  # Drop a multicast group membership
  IP_DROP_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  # Drop a multicast group membership
  IP_DROP_SOURCE_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  # Allow binding to nonexistent IP addresses
  IP_FREEBIND = ::T.let(nil, ::T.untyped)
  # Header is included with data
  IP_HDRINCL = ::T.let(nil, ::T.untyped)
  # IPsec security policy
  IP_IPSEC_POLICY = ::T.let(nil, ::T.untyped)
  # Maximum number multicast groups a socket can join
  IP_MAX_MEMBERSHIPS = ::T.let(nil, ::T.untyped)
  # Minimum TTL allowed for received packets
  IP_MINTTL = ::T.let(nil, ::T.untyped)
  # Multicast source filtering
  IP_MSFILTER = ::T.let(nil, ::T.untyped)
  # The Maximum Transmission Unit of the socket
  IP_MTU = ::T.let(nil, ::T.untyped)
  # Path MTU discovery
  IP_MTU_DISCOVER = ::T.let(nil, ::T.untyped)
  # IP multicast interface
  IP_MULTICAST_IF = ::T.let(nil, ::T.untyped)
  # IP multicast loopback
  IP_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  # IP multicast TTL
  IP_MULTICAST_TTL = ::T.let(nil, ::T.untyped)
  # IP options to be included in packets
  IP_OPTIONS = ::T.let(nil, ::T.untyped)
  # Retrieve security context with datagram
  IP_PASSSEC = ::T.let(nil, ::T.untyped)
  # Receive packet information with datagrams
  IP_PKTINFO = ::T.let(nil, ::T.untyped)
  # Receive packet options with datagrams
  IP_PKTOPTIONS = ::T.let(nil, ::T.untyped)
  # Always send DF frames
  IP_PMTUDISC_DO = ::T.let(nil, ::T.untyped)
  # Never send DF frames
  IP_PMTUDISC_DONT = ::T.let(nil, ::T.untyped)
  # Use per-route hints
  IP_PMTUDISC_WANT = ::T.let(nil, ::T.untyped)
  # Enable extended reliable error message passing
  IP_RECVERR = ::T.let(nil, ::T.untyped)
  # Receive all IP options with datagram
  IP_RECVOPTS = ::T.let(nil, ::T.untyped)
  # Receive all IP options for response
  IP_RECVRETOPTS = ::T.let(nil, ::T.untyped)
  # Receive TOS with incoming packets
  IP_RECVTOS = ::T.let(nil, ::T.untyped)
  # Receive IP TTL with datagrams
  IP_RECVTTL = ::T.let(nil, ::T.untyped)
  # IP options to be included in datagrams
  IP_RETOPTS = ::T.let(nil, ::T.untyped)
  # Notify transit routers to more closely examine the contents of an IP packet
  IP_ROUTER_ALERT = ::T.let(nil, ::T.untyped)
  # IP type-of-service
  IP_TOS = ::T.let(nil, ::T.untyped)
  # Transparent proxy
  IP_TRANSPARENT = ::T.let(nil, ::T.untyped)
  # IP time-to-live
  IP_TTL = ::T.let(nil, ::T.untyped)
  # Unblock IPv4 multicast packets with a give source address
  IP_UNBLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  IP_XFRM_POLICY = ::T.let(nil, ::T.untyped)
  # Block multicast packets from this source
  MCAST_BLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  # Exclusive multicast source filter
  MCAST_EXCLUDE = ::T.let(nil, ::T.untyped)
  # Inclusive multicast source filter
  MCAST_INCLUDE = ::T.let(nil, ::T.untyped)
  # Join a multicast group
  MCAST_JOIN_GROUP = ::T.let(nil, ::T.untyped)
  # Join a multicast source group
  MCAST_JOIN_SOURCE_GROUP = ::T.let(nil, ::T.untyped)
  # Leave a multicast group
  MCAST_LEAVE_GROUP = ::T.let(nil, ::T.untyped)
  # Leave a multicast source group
  MCAST_LEAVE_SOURCE_GROUP = ::T.let(nil, ::T.untyped)
  # Multicast source filtering
  MCAST_MSFILTER = ::T.let(nil, ::T.untyped)
  # Unblock multicast packets from this source
  MCAST_UNBLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  # Confirm path validity
  MSG_CONFIRM = ::T.let(nil, ::T.untyped)
  # Control data lost before delivery
  MSG_CTRUNC = ::T.let(nil, ::T.untyped)
  # Send without using the routing tables
  MSG_DONTROUTE = ::T.let(nil, ::T.untyped)
  # This message should be non-blocking
  MSG_DONTWAIT = ::T.let(nil, ::T.untyped)
  # [Data completes connection](https://ruby-doc.org/stdlib-3.0.0/libdoc/socket/rdoc/Socket.html#MSG_EOF)
  MSG_EOF = ::T.let(nil, ::T.untyped) 
  # [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) completes record
  MSG_EOR = ::T.let(nil, ::T.untyped)
  # Fetch message from error queue
  MSG_ERRQUEUE = ::T.let(nil, ::T.untyped)
  # Reduce step of the handshake process
  MSG_FASTOPEN = ::T.let(nil, ::T.untyped)
  MSG_FIN = ::T.let(nil, ::T.untyped)
  # Sender will send more
  MSG_MORE = ::T.let(nil, ::T.untyped)
  # Do not generate SIGPIPE
  MSG_NOSIGNAL = ::T.let(nil, ::T.untyped)
  # [`Process`](https://docs.ruby-lang.org/en/2.7.0/Process.html) out-of-band
  # data
  MSG_OOB = ::T.let(nil, ::T.untyped)
  # Peek at incoming message
  MSG_PEEK = ::T.let(nil, ::T.untyped)
  # Wait for full request
  MSG_PROXY = ::T.let(nil, ::T.untyped)
  MSG_RST = ::T.let(nil, ::T.untyped)
  MSG_SYN = ::T.let(nil, ::T.untyped)
  # [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) discarded before
  # delivery
  MSG_TRUNC = ::T.let(nil, ::T.untyped)
  # Wait for full request or error
  MSG_WAITALL = ::T.let(nil, ::T.untyped)
  # The service specified is a datagram service (looks up UDP ports)
  NI_DGRAM = ::T.let(nil, ::T.untyped)
  # Maximum length of a hostname
  NI_MAXHOST = ::T.let(nil, ::T.untyped)
  # Maximum length of a service name
  NI_MAXSERV = ::T.let(nil, ::T.untyped)
  # A name is required
  NI_NAMEREQD = ::T.let(nil, ::T.untyped)
  # An FQDN is not required for local hosts, return only the local part
  NI_NOFQDN = ::T.let(nil, ::T.untyped)
  # Return a numeric address
  NI_NUMERICHOST = ::T.let(nil, ::T.untyped)
  # Return the service name as a digit string
  NI_NUMERICSERV = ::T.let(nil, ::T.untyped)
  # AppleTalk protocol
  PF_APPLETALK = ::T.let(nil, ::T.untyped)
  # AX.25 protocol
  PF_AX25 = ::T.let(nil, ::T.untyped)
  # IPv4 protocol
  PF_INET = ::T.let(nil, ::T.untyped)
  # IPv6 protocol
  PF_INET6 = ::T.let(nil, ::T.untyped)
  # IPX protocol
  PF_IPX = ::T.let(nil, ::T.untyped)
  # Integrated Services Digital Network
  PF_ISDN = ::T.let(nil, ::T.untyped)
  PF_KEY = ::T.let(nil, ::T.untyped)
  # Host-internal protocols
  PF_LOCAL = ::T.let(nil, ::T.untyped)
  # Maximum address family for this platform
  PF_MAX = ::T.let(nil, ::T.untyped)
  # Direct link-layer access
  PF_PACKET = ::T.let(nil, ::T.untyped)
  # Internal routing protocol
  PF_ROUTE = ::T.let(nil, ::T.untyped)
  # IBM SNA protocol
  PF_SNA = ::T.let(nil, ::T.untyped)
  # UNIX sockets
  PF_UNIX = ::T.let(nil, ::T.untyped)
  # Unspecified protocol, any supported address family
  PF_UNSPEC = ::T.let(nil, ::T.untyped)
  # The sender's credentials
  SCM_CREDENTIALS = ::T.let(nil, ::T.untyped)
  # Access rights
  SCM_RIGHTS = ::T.let(nil, ::T.untyped)
  # Timestamp (timeval)
  SCM_TIMESTAMP = ::T.let(nil, ::T.untyped)
  # Timestamp (timespec list) (Linux 2.6.30)
  SCM_TIMESTAMPING = ::T.let(nil, ::T.untyped)
  # Timespec (timespec)
  SCM_TIMESTAMPNS = ::T.let(nil, ::T.untyped)
  # Wifi status (Linux 3.3)
  SCM_WIFI_STATUS = ::T.let(nil, ::T.untyped)
  # Shut down the reading side of the socket
  SHUT_RD = ::T.let(nil, ::T.untyped)
  # Shut down the both sides of the socket
  SHUT_RDWR = ::T.let(nil, ::T.untyped)
  # Shut down the writing side of the socket
  SHUT_WR = ::T.let(nil, ::T.untyped)
  # A datagram socket provides connectionless, unreliable messaging
  SOCK_DGRAM = ::T.let(nil, ::T.untyped)
  # Device-level packet access
  SOCK_PACKET = ::T.let(nil, ::T.untyped)
  # A raw socket provides low-level access for direct access or implementing
  # network protocols
  SOCK_RAW = ::T.let(nil, ::T.untyped)
  # A reliable datagram socket provides reliable delivery of messages
  SOCK_RDM = ::T.let(nil, ::T.untyped)
  # A sequential packet socket provides sequenced, reliable two-way connection
  # for datagrams
  SOCK_SEQPACKET = ::T.let(nil, ::T.untyped)
  # A stream socket provides a sequenced, reliable two-way connection for a byte
  # stream
  SOCK_STREAM = ::T.let(nil, ::T.untyped)
  # IP socket options
  SOL_IP = ::T.let(nil, ::T.untyped)
  # Socket-level options
  SOL_SOCKET = ::T.let(nil, ::T.untyped)
  # TCP socket options
  SOL_TCP = ::T.let(nil, ::T.untyped)
  # UDP socket options
  SOL_UDP = ::T.let(nil, ::T.untyped)
  # Maximum connection requests that may be queued for a socket
  SOMAXCONN = ::T.let(nil, ::T.untyped)
  # [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html) has had listen()
  # called on it
  SO_ACCEPTCONN = ::T.let(nil, ::T.untyped)
  # Attach an accept filter
  SO_ATTACH_FILTER = ::T.let(nil, ::T.untyped)
  # Only send packets from the given interface
  SO_BINDTODEVICE = ::T.let(nil, ::T.untyped)
  # Permit sending of broadcast messages
  SO_BROADCAST = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the threshold in
  # microseconds for low latency polling (Linux 3.11)
  SO_BUSY_POLL = ::T.let(nil, ::T.untyped)
  # Debug info recording
  SO_DEBUG = ::T.let(nil, ::T.untyped)
  # Detach an accept filter
  SO_DETACH_FILTER = ::T.let(nil, ::T.untyped)
  # Domain given for socket() (Linux 2.6.32)
  SO_DOMAIN = ::T.let(nil, ::T.untyped)
  # Use interface addresses
  SO_DONTROUTE = ::T.let(nil, ::T.untyped)
  # Get and clear the error status
  SO_ERROR = ::T.let(nil, ::T.untyped)
  # Obtain filter set by
  # [`SO_ATTACH_FILTER`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#SO_ATTACH_FILTER)
  # (Linux 3.8)
  SO_GET_FILTER = ::T.let(nil, ::T.untyped)
  # Keep connections alive
  SO_KEEPALIVE = ::T.let(nil, ::T.untyped)
  # Linger on close if data is present
  SO_LINGER = ::T.let(nil, ::T.untyped)
  # Lock the filter attached to a socket (Linux 3.9)
  SO_LOCK_FILTER = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the mark for
  # mark-based routing (Linux 2.6.25)
  SO_MARK = ::T.let(nil, ::T.untyped)
  # Cap the rate computed by transport layer. [bytes per second] (Linux 3.13)
  SO_MAX_PACING_RATE = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) netns of a socket
  # (Linux 3.4)
  SO_NOFCS = ::T.let(nil, ::T.untyped)
  # Disable checksums
  SO_NO_CHECK = ::T.let(nil, ::T.untyped)
  # Leave received out-of-band data in-line
  SO_OOBINLINE = ::T.let(nil, ::T.untyped)
  # Receive
  # [`SCM_CREDENTIALS`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#SCM_CREDENTIALS)
  # messages
  SO_PASSCRED = ::T.let(nil, ::T.untyped)
  # Toggle security context passing (Linux 2.6.18)
  SO_PASSSEC = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the peek offset (Linux
  # 3.4)
  SO_PEEK_OFF = ::T.let(nil, ::T.untyped)
  # The credentials of the foreign process connected to this socket
  SO_PEERCRED = ::T.let(nil, ::T.untyped)
  # Name of the connecting user
  SO_PEERNAME = ::T.let(nil, ::T.untyped)
  # Obtain the security credentials (Linux 2.6.2)
  SO_PEERSEC = ::T.let(nil, ::T.untyped)
  # The protocol-defined priority for all packets on this socket
  SO_PRIORITY = ::T.let(nil, ::T.untyped)
  # Protocol given for socket() (Linux 2.6.32)
  SO_PROTOCOL = ::T.let(nil, ::T.untyped)
  # Receive buffer size
  SO_RCVBUF = ::T.let(nil, ::T.untyped)
  # Receive buffer size without rmem\_max limit (Linux 2.6.14)
  SO_RCVBUFFORCE = ::T.let(nil, ::T.untyped)
  # Receive low-water mark
  SO_RCVLOWAT = ::T.let(nil, ::T.untyped)
  # Receive timeout
  SO_RCVTIMEO = ::T.let(nil, ::T.untyped)
  # Allow local address reuse
  SO_REUSEADDR = ::T.let(nil, ::T.untyped)
  # Allow local address and port reuse
  SO_REUSEPORT = ::T.let(nil, ::T.untyped)
  # Toggle cmsg for number of packets dropped (Linux 2.6.33)
  SO_RXQ_OVFL = ::T.let(nil, ::T.untyped)
  SO_SECURITY_AUTHENTICATION = ::T.let(nil, ::T.untyped)
  SO_SECURITY_ENCRYPTION_NETWORK = ::T.let(nil, ::T.untyped)
  SO_SECURITY_ENCRYPTION_TRANSPORT = ::T.let(nil, ::T.untyped)
  # Make select() detect socket error queue with errorfds (Linux 3.10)
  SO_SELECT_ERR_QUEUE = ::T.let(nil, ::T.untyped)
  # Send buffer size
  SO_SNDBUF = ::T.let(nil, ::T.untyped)
  # Send buffer size without wmem\_max limit (Linux 2.6.14)
  SO_SNDBUFFORCE = ::T.let(nil, ::T.untyped)
  # Send low-water mark
  SO_SNDLOWAT = ::T.let(nil, ::T.untyped)
  # Send timeout
  SO_SNDTIMEO = ::T.let(nil, ::T.untyped)
  # Receive timestamp with datagrams (timeval)
  SO_TIMESTAMP = ::T.let(nil, ::T.untyped)
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) stamping of incoming
  # and outgoing packets (Linux 2.6.30)
  SO_TIMESTAMPING = ::T.let(nil, ::T.untyped)
  # Receive nanosecond timestamp with datagrams (timespec)
  SO_TIMESTAMPNS = ::T.let(nil, ::T.untyped)
  # Get the socket type
  SO_TYPE = ::T.let(nil, ::T.untyped)
  # Toggle cmsg for wifi status (Linux 3.3)
  SO_WIFI_STATUS = ::T.let(nil, ::T.untyped)
  # TCP congestion control algorithm (Linux 2.6.13, glibc 2.6)
  TCP_CONGESTION = ::T.let(nil, ::T.untyped)
  # TCP Cookie Transactions (Linux 2.6.33, glibc 2.18)
  TCP_COOKIE_TRANSACTIONS = ::T.let(nil, ::T.untyped)
  # Don't send partial frames (Linux 2.2, glibc 2.2)
  TCP_CORK = ::T.let(nil, ::T.untyped)
  # Don't notify a listening socket until data is ready (Linux 2.4, glibc 2.2)
  TCP_DEFER_ACCEPT = ::T.let(nil, ::T.untyped)
  # Reduce step of the handshake process (Linux 3.7, glibc 2.18)
  TCP_FASTOPEN = ::T.let(nil, ::T.untyped)
  # Retrieve information about this socket (Linux 2.4, glibc 2.2)
  TCP_INFO = ::T.let(nil, ::T.untyped)
  # Maximum number of keepalive probes allowed before dropping a connection
  # (Linux 2.4, glibc 2.2)
  TCP_KEEPCNT = ::T.let(nil, ::T.untyped)
  # Idle time before keepalive probes are sent (Linux 2.4, glibc 2.2)
  TCP_KEEPIDLE = ::T.let(nil, ::T.untyped)
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) between keepalive
  # probes (Linux 2.4, glibc 2.2)
  TCP_KEEPINTVL = ::T.let(nil, ::T.untyped)
  # Lifetime of orphaned FIN\_WAIT2 sockets (Linux 2.4, glibc 2.2)
  TCP_LINGER2 = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) maximum segment size
  TCP_MAXSEG = ::T.let(nil, ::T.untyped)
  # Use MD5 digests (RFC2385, Linux 2.6.20, glibc 2.7)
  TCP_MD5SIG = ::T.let(nil, ::T.untyped)
  # Don't delay sending to coalesce packets
  TCP_NODELAY = ::T.let(nil, ::T.untyped)
  # Sequence of a queue for repair mode (Linux 3.5, glibc 2.18)
  TCP_QUEUE_SEQ = ::T.let(nil, ::T.untyped)
  # Enable quickack mode (Linux 2.4.4, glibc 2.3)
  TCP_QUICKACK = ::T.let(nil, ::T.untyped)
  # Repair mode (Linux 3.5, glibc 2.18)
  TCP_REPAIR = ::T.let(nil, ::T.untyped)
  # Options for repair mode (Linux 3.5, glibc 2.18)
  TCP_REPAIR_OPTIONS = ::T.let(nil, ::T.untyped)
  # [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html) for repair mode
  # (Linux 3.5, glibc 2.18)
  TCP_REPAIR_QUEUE = ::T.let(nil, ::T.untyped)
  # Number of SYN retransmits before a connection is dropped (Linux 2.4, glibc
  # 2.2)
  TCP_SYNCNT = ::T.let(nil, ::T.untyped)
  # Duplicated acknowledgments handling for thin-streams (Linux 2.6.34, glibc
  # 2.18)
  TCP_THIN_DUPACK = ::T.let(nil, ::T.untyped)
  # Linear timeouts for thin-streams (Linux 2.6.34, glibc 2.18)
  TCP_THIN_LINEAR_TIMEOUTS = ::T.let(nil, ::T.untyped)
  # TCP timestamp (Linux 3.9, glibc 2.18)
  TCP_TIMESTAMP = ::T.let(nil, ::T.untyped)
  # Max timeout before a TCP connection is aborted (Linux 2.6.37, glibc 2.18)
  TCP_USER_TIMEOUT = ::T.let(nil, ::T.untyped)
  # Clamp the size of the advertised window (Linux 2.4, glibc 2.2)
  TCP_WINDOW_CLAMP = ::T.let(nil, ::T.untyped)
  # Don't send partial frames (Linux 2.5.44, glibc 2.11)
  UDP_CORK = ::T.let(nil, ::T.untyped)

  # Accepts a next connection. Returns a new
  # [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html) object and
  # [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object.
  #
  # ```ruby
  # serv = Socket.new(:INET, :STREAM, 0)
  # serv.listen(5)
  # c = Socket.new(:INET, :STREAM, 0)
  # c.connect(serv.connect_address)
  # p serv.accept #=> [#<Socket:fd 6>, #<Addrinfo: 127.0.0.1:48555 TCP>]
  # ```
  sig {returns(::T.untyped)}
  def accept(); end

  # Accepts an incoming connection using accept(2) after O\_NONBLOCK is set for
  # the underlying file descriptor. It returns an array containing the accepted
  # socket for the incoming connection, *client\_socket*, and an
  # [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html),
  # *client\_addrinfo*.
  #
  # ### Example
  #
  # ```ruby
  # # In one script, start this first
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new(AF_INET, SOCK_STREAM, 0)
  # sockaddr = Socket.sockaddr_in(2200, 'localhost')
  # socket.bind(sockaddr)
  # socket.listen(5)
  # begin # emulate blocking accept
  #   client_socket, client_addrinfo = socket.accept_nonblock
  # rescue IO::WaitReadable, Errno::EINTR
  #   IO.select([socket])
  #   retry
  # end
  # puts "The client said, '#{client_socket.readline.chomp}'"
  # client_socket.puts "Hello from script one!"
  # socket.close
  #
  # # In another script, start this second
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new(AF_INET, SOCK_STREAM, 0)
  # sockaddr = Socket.sockaddr_in(2200, 'localhost')
  # socket.connect(sockaddr)
  # socket.puts "Hello from script 2."
  # puts "The server said, '#{socket.readline.chomp}'"
  # socket.close
  # ```
  #
  # Refer to
  # [`Socket#accept`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept)
  # for the exceptions that may be thrown if the call to *accept\_nonblock*
  # fails.
  #
  # [`Socket#accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept_nonblock)
  # may raise any error corresponding to accept(2) failure, including
  # Errno::EWOULDBLOCK.
  #
  # If the exception is Errno::EWOULDBLOCK, Errno::EAGAIN,
  # [`Errno::ECONNABORTED`](https://docs.ruby-lang.org/en/2.7.0/Errno/ECONNABORTED.html)
  # or [`Errno::EPROTO`](https://docs.ruby-lang.org/en/2.7.0/Errno/EPROTO.html),
  # it is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html).
  # So
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # can be used to rescue the exceptions for retrying accept\_nonblock.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead.
  #
  # ### See
  # *   [`Socket#accept`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept)
  sig do
    params(
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def accept_nonblock(exception: T.unsafe(nil)); end

  # Binds to the given local address.
  #
  # ### Parameter
  # *   `local_sockaddr` - the `struct` sockaddr contained in a string or an
  #     [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object
  #
  #
  # ### Example
  #
  # ```ruby
  # require 'socket'
  #
  # # use Addrinfo
  # socket = Socket.new(:INET, :STREAM, 0)
  # socket.bind(Addrinfo.tcp("127.0.0.1", 2222))
  # p socket.local_address #=> #<Addrinfo: 127.0.0.1:2222 TCP>
  #
  # # use struct sockaddr
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 2200, 'localhost' )
  # socket.bind( sockaddr )
  # ```
  #
  # ### Unix-based Exceptions
  # On unix-based based systems the following system exceptions may be raised if
  # the call to *bind* fails:
  # *   Errno::EACCES - the specified *sockaddr* is protected and the current
  #     user does not have permission to bind to it
  # *   Errno::EADDRINUSE - the specified *sockaddr* is already in use
  # *   Errno::EADDRNOTAVAIL - the specified *sockaddr* is not available from
  #     the local machine
  # *   Errno::EAFNOSUPPORT - the specified *sockaddr* is not a valid address
  #     for the family of the calling `socket`
  # *   Errno::EBADF - the *sockaddr* specified is not a valid file descriptor
  # *   Errno::EFAULT - the *sockaddr* argument cannot be accessed
  # *   Errno::EINVAL - the `socket` is already bound to an address, and the
  #     protocol does not support binding to the new *sockaddr* or the `socket`
  #     has been shut down.
  # *   Errno::EINVAL - the address length is not a valid length for the address
  #     family
  # *   Errno::ENAMETOOLONG - the pathname resolved had a length which exceeded
  #     PATH\_MAX
  # *   Errno::ENOBUFS - no buffer space is available
  # *   Errno::ENOSR - there were insufficient STREAMS resources available to
  #     complete the operation
  # *   Errno::ENOTSOCK - the `socket` does not refer to a socket
  # *   Errno::EOPNOTSUPP - the socket type of the `socket` does not support
  #     binding to an address
  #
  #
  # On unix-based based systems if the address family of the calling `socket` is
  # [`Socket::AF_UNIX`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#AF_UNIX)
  # the follow exceptions may be raised if the call to *bind* fails:
  # *   Errno::EACCES - search permission is denied for a component of the
  #     prefix path or write access to the `socket` is denied
  # *   Errno::EDESTADDRREQ - the *sockaddr* argument is a null pointer
  # *   Errno::EISDIR - same as Errno::EDESTADDRREQ
  # *   Errno::EIO - an i/o error occurred
  # *   Errno::ELOOP - too many symbolic links were encountered in translating
  #     the pathname in *sockaddr*
  # *   Errno::ENAMETOOLLONG - a component of a pathname exceeded NAME\_MAX
  #     characters, or an entire pathname exceeded PATH\_MAX characters
  # *   Errno::ENOENT - a component of the pathname does not name an existing
  #     file or the pathname is an empty string
  # *   Errno::ENOTDIR - a component of the path prefix of the pathname in
  #     *sockaddr* is not a directory
  # *   Errno::EROFS - the name would reside on a read only filesystem
  #
  #
  # ### Windows Exceptions
  # On Windows systems the following system exceptions may be raised if the call
  # to *bind* fails:
  # *   Errno::ENETDOWN-- the network is down
  # *   Errno::EACCES - the attempt to connect the datagram socket to the
  #     broadcast address failed
  # *   Errno::EADDRINUSE - the socket's local address is already in use
  # *   Errno::EADDRNOTAVAIL - the specified address is not a valid address for
  #     this computer
  # *   Errno::EFAULT - the socket's internal address or address length
  #     parameter is too small or is not a valid part of the user space
  #     addressed
  # *   Errno::EINVAL - the `socket` is already bound to an address
  # *   Errno::ENOBUFS - no buffer space is available
  # *   Errno::ENOTSOCK - the `socket` argument does not refer to a socket
  #
  #
  # ### See
  # *   bind manual pages on unix-based systems
  # *   bind function in Microsoft's Winsock functions reference
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def bind(arg0); end

  # Requests a connection to be made on the given `remote_sockaddr`. Returns 0
  # if successful, otherwise an exception is raised.
  #
  # ### Parameter
  # *   `remote_sockaddr` - the `struct` sockaddr contained in a string or
  #     [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object
  #
  #
  # ### Example:
  #
  # ```ruby
  # # Pull down Google's web page
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 80, 'www.google.com' )
  # socket.connect( sockaddr )
  # socket.write( "GET / HTTP/1.0\r\n\r\n" )
  # results = socket.read
  # ```
  #
  # ### Unix-based Exceptions
  # On unix-based systems the following system exceptions may be raised if the
  # call to *connect* fails:
  # *   Errno::EACCES - search permission is denied for a component of the
  #     prefix path or write access to the `socket` is denied
  # *   Errno::EADDRINUSE - the *sockaddr* is already in use
  # *   Errno::EADDRNOTAVAIL - the specified *sockaddr* is not available from
  #     the local machine
  # *   Errno::EAFNOSUPPORT - the specified *sockaddr* is not a valid address
  #     for the address family of the specified `socket`
  # *   Errno::EALREADY - a connection is already in progress for the specified
  #     socket
  # *   Errno::EBADF - the `socket` is not a valid file descriptor
  # *   Errno::ECONNREFUSED - the target *sockaddr* was not listening for
  #     connections refused the connection request
  # *   [`Errno::ECONNRESET`](https://docs.ruby-lang.org/en/2.7.0/Errno/ECONNRESET.html)
  #     - the remote host reset the connection request
  # *   Errno::EFAULT - the *sockaddr* cannot be accessed
  # *   Errno::EHOSTUNREACH - the destination host cannot be reached (probably
  #     because the host is down or a remote router cannot reach it)
  # *   Errno::EINPROGRESS - the O\_NONBLOCK is set for the `socket` and the
  #     connection cannot be immediately established; the connection will be
  #     established asynchronously
  # *   Errno::EINTR - the attempt to establish the connection was interrupted
  #     by delivery of a signal that was caught; the connection will be
  #     established asynchronously
  # *   Errno::EISCONN - the specified `socket` is already connected
  # *   Errno::EINVAL - the address length used for the *sockaddr* is not a
  #     valid length for the address family or there is an invalid family in
  #     *sockaddr*
  # *   Errno::ENAMETOOLONG - the pathname resolved had a length which exceeded
  #     PATH\_MAX
  # *   Errno::ENETDOWN - the local interface used to reach the destination is
  #     down
  # *   Errno::ENETUNREACH - no route to the network is present
  # *   Errno::ENOBUFS - no buffer space is available
  # *   Errno::ENOSR - there were insufficient STREAMS resources available to
  #     complete the operation
  # *   Errno::ENOTSOCK - the `socket` argument does not refer to a socket
  # *   Errno::EOPNOTSUPP - the calling `socket` is listening and cannot be
  #     connected
  # *   Errno::EPROTOTYPE - the *sockaddr* has a different type than the socket
  #     bound to the specified peer address
  # *   Errno::ETIMEDOUT - the attempt to connect time out before a connection
  #     was made.
  #
  #
  # On unix-based systems if the address family of the calling `socket` is
  # [`AF_UNIX`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#AF_UNIX) the
  # follow exceptions may be raised if the call to *connect* fails:
  # *   Errno::EIO - an i/o error occurred while reading from or writing to the
  #     file system
  # *   Errno::ELOOP - too many symbolic links were encountered in translating
  #     the pathname in *sockaddr*
  # *   Errno::ENAMETOOLLONG - a component of a pathname exceeded NAME\_MAX
  #     characters, or an entire pathname exceeded PATH\_MAX characters
  # *   Errno::ENOENT - a component of the pathname does not name an existing
  #     file or the pathname is an empty string
  # *   Errno::ENOTDIR - a component of the path prefix of the pathname in
  #     *sockaddr* is not a directory
  #
  #
  # ### Windows Exceptions
  # On Windows systems the following system exceptions may be raised if the call
  # to *connect* fails:
  # *   Errno::ENETDOWN - the network is down
  # *   Errno::EADDRINUSE - the socket's local address is already in use
  # *   Errno::EINTR - the socket was cancelled
  # *   Errno::EINPROGRESS - a blocking socket is in progress or the service
  #     provider is still processing a callback function. Or a nonblocking
  #     connect call is in progress on the `socket`.
  # *   Errno::EALREADY - see Errno::EINVAL
  # *   Errno::EADDRNOTAVAIL - the remote address is not a valid address, such
  #     as ADDR\_ANY TODO check ADDRANY TO
  #     [`INADDR_ANY`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#INADDR_ANY)
  # *   Errno::EAFNOSUPPORT - addresses in the specified family cannot be used
  #     with with this `socket`
  # *   Errno::ECONNREFUSED - the target *sockaddr* was not listening for
  #     connections refused the connection request
  # *   Errno::EFAULT - the socket's internal address or address length
  #     parameter is too small or is not a valid part of the user space address
  # *   Errno::EINVAL - the `socket` is a listening socket
  # *   Errno::EISCONN - the `socket` is already connected
  # *   Errno::ENETUNREACH - the network cannot be reached from this host at
  #     this time
  # *   Errno::EHOSTUNREACH - no route to the network is present
  # *   Errno::ENOBUFS - no buffer space is available
  # *   Errno::ENOTSOCK - the `socket` argument does not refer to a socket
  # *   Errno::ETIMEDOUT - the attempt to connect time out before a connection
  #     was made.
  # *   Errno::EWOULDBLOCK - the socket is marked as nonblocking and the
  #     connection cannot be completed immediately
  # *   Errno::EACCES - the attempt to connect the datagram socket to the
  #     broadcast address failed
  #
  #
  # ### See
  # *   connect manual pages on unix-based systems
  # *   connect function in Microsoft's Winsock functions reference
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect(arg0); end

  # Requests a connection to be made on the given `remote_sockaddr` after
  # O\_NONBLOCK is set for the underlying file descriptor. Returns 0 if
  # successful, otherwise an exception is raised.
  #
  # ### Parameter
  #
  # ```ruby
  # # +remote_sockaddr+ - the +struct+ sockaddr contained in a string or Addrinfo object
  # ```
  #
  # ### Example:
  #
  # ```ruby
  # # Pull down Google's web page
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new(AF_INET, SOCK_STREAM, 0)
  # sockaddr = Socket.sockaddr_in(80, 'www.google.com')
  # begin # emulate blocking connect
  #   socket.connect_nonblock(sockaddr)
  # rescue IO::WaitWritable
  #   IO.select(nil, [socket]) # wait 3-way handshake completion
  #   begin
  #     socket.connect_nonblock(sockaddr) # check connection failure
  #   rescue Errno::EISCONN
  #   end
  # end
  # socket.write("GET / HTTP/1.0\r\n\r\n")
  # results = socket.read
  # ```
  #
  # Refer to
  # [`Socket#connect`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-connect)
  # for the exceptions that may be thrown if the call to *connect\_nonblock*
  # fails.
  #
  # [`Socket#connect_nonblock`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-connect_nonblock)
  # may raise any error corresponding to connect(2) failure, including
  # Errno::EINPROGRESS.
  #
  # If the exception is Errno::EINPROGRESS, it is extended by
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html).
  # So
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # can be used to rescue the exceptions for retrying connect\_nonblock.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`connect_nonblock`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-connect_nonblock)
  # should not raise an
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # exception, but return the symbol `:wait_writable` instead.
  #
  # ### See
  #
  # ```ruby
  # # Socket#connect
  # ```
  sig do
    params(
      addr: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect_nonblock(addr, exception: T.unsafe(nil)); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # enable the socket option
  # [`IPV6_V6ONLY`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#IPV6_V6ONLY)
  # if
  # [`IPV6_V6ONLY`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#IPV6_V6ONLY)
  # is available.
  sig {returns(::T.untyped)}
  def ipv6only!(); end

  # Listens for connections, using the specified `int` as the backlog. A call to
  # *listen* only applies if the `socket` is of type
  # [`SOCK_STREAM`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#SOCK_STREAM)
  # or
  # [`SOCK_SEQPACKET`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#SOCK_SEQPACKET).
  #
  # ### Parameter
  # *   `backlog` - the maximum length of the queue for pending connections.
  #
  #
  # ### Example 1
  #
  # ```ruby
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 2200, 'localhost' )
  # socket.bind( sockaddr )
  # socket.listen( 5 )
  # ```
  #
  # ### Example 2 (listening on an arbitrary port, unix-based systems only):
  #
  # ```ruby
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # socket.listen( 1 )
  # ```
  #
  # ### Unix-based Exceptions
  # On unix based systems the above will work because a new `sockaddr` struct is
  # created on the address ADDR\_ANY, for an arbitrary port number as handed off
  # by the kernel. It will not work on Windows, because Windows requires that
  # the `socket` is bound by calling *bind* before it can *listen*.
  #
  # If the *backlog* amount exceeds the implementation-dependent maximum queue
  # length, the implementation's maximum queue length will be used.
  #
  # On unix-based based systems the following system exceptions may be raised if
  # the call to *listen* fails:
  # *   Errno::EBADF - the *socket* argument is not a valid file descriptor
  # *   Errno::EDESTADDRREQ - the *socket* is not bound to a local address, and
  #     the protocol does not support listening on an unbound socket
  # *   Errno::EINVAL - the *socket* is already connected
  # *   Errno::ENOTSOCK - the *socket* argument does not refer to a socket
  # *   Errno::EOPNOTSUPP - the *socket* protocol does not support listen
  # *   Errno::EACCES - the calling process does not have appropriate privileges
  # *   Errno::EINVAL - the *socket* has been shut down
  # *   Errno::ENOBUFS - insufficient resources are available in the system to
  #     complete the call
  #
  #
  # ### Windows Exceptions
  # On Windows systems the following system exceptions may be raised if the call
  # to *listen* fails:
  # *   Errno::ENETDOWN - the network is down
  # *   Errno::EADDRINUSE - the socket's local address is already in use. This
  #     usually occurs during the execution of *bind* but could be delayed if
  #     the call to *bind* was to a partially wildcard address (involving
  #     ADDR\_ANY) and if a specific address needs to be committed at the time
  #     of the call to *listen*
  # *   Errno::EINPROGRESS - a Windows Sockets 1.1 call is in progress or the
  #     service provider is still processing a callback function
  # *   Errno::EINVAL - the `socket` has not been bound with a call to *bind*.
  # *   Errno::EISCONN - the `socket` is already connected
  # *   Errno::EMFILE - no more socket descriptors are available
  # *   Errno::ENOBUFS - no buffer space is available
  # *   Errno::ENOTSOC - `socket` is not a socket
  # *   Errno::EOPNOTSUPP - the referenced `socket` is not a type that supports
  #     the *listen* method
  #
  #
  # ### See
  # *   listen manual pages on unix-based systems
  # *   listen function in Microsoft's Winsock functions reference
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(arg0); end

  # Receives up to *maxlen* bytes from `socket`. *flags* is zero or more of the
  # `MSG_` options. The first element of the results, *mesg*, is the data
  # received. The second element, *sender\_addrinfo*, contains protocol-specific
  # address information of the sender.
  #
  # ### Parameters
  # *   `maxlen` - the maximum number of bytes to receive from the socket
  # *   `flags` - zero or more of the `MSG_` options
  #
  #
  # ### Example
  #
  # ```ruby
  # # In one file, start this first
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 2200, 'localhost' )
  # socket.bind( sockaddr )
  # socket.listen( 5 )
  # client, client_addrinfo = socket.accept
  # data = client.recvfrom( 20 )[0].chomp
  # puts "I only received 20 bytes '#{data}'"
  # sleep 1
  # socket.close
  #
  # # In another file, start this second
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 2200, 'localhost' )
  # socket.connect( sockaddr )
  # socket.puts "Watch this get cut short!"
  # socket.close
  # ```
  #
  # ### Unix-based Exceptions
  # On unix-based based systems the following system exceptions may be raised if
  # the call to *recvfrom* fails:
  # *   Errno::EAGAIN - the `socket` file descriptor is marked as O\_NONBLOCK
  #     and no data is waiting to be received; or
  #     [`MSG_OOB`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#MSG_OOB) is
  #     set and no out-of-band data is available and either the `socket` file
  #     descriptor is marked as O\_NONBLOCK or the `socket` does not support
  #     blocking to wait for out-of-band-data
  # *   Errno::EWOULDBLOCK - see Errno::EAGAIN
  # *   Errno::EBADF - the `socket` is not a valid file descriptor
  # *   [`Errno::ECONNRESET`](https://docs.ruby-lang.org/en/2.7.0/Errno/ECONNRESET.html)
  #     - a connection was forcibly closed by a peer
  # *   Errno::EFAULT - the socket's internal buffer, address or address length
  #     cannot be accessed or written
  # *   Errno::EINTR - a signal interrupted *recvfrom* before any data was
  #     available
  # *   Errno::EINVAL - the
  #     [`MSG_OOB`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#MSG_OOB)
  #     flag is set and no out-of-band data is available
  # *   Errno::EIO - an i/o error occurred while reading from or writing to the
  #     filesystem
  # *   Errno::ENOBUFS - insufficient resources were available in the system to
  #     perform the operation
  # *   Errno::ENOMEM - insufficient memory was available to fulfill the request
  # *   Errno::ENOSR - there were insufficient STREAMS resources available to
  #     complete the operation
  # *   Errno::ENOTCONN - a receive is attempted on a connection-mode socket
  #     that is not connected
  # *   Errno::ENOTSOCK - the `socket` does not refer to a socket
  # *   Errno::EOPNOTSUPP - the specified flags are not supported for this
  #     socket type
  # *   Errno::ETIMEDOUT - the connection timed out during connection
  #     establishment or due to a transmission timeout on an active connection
  #
  #
  # ### Windows Exceptions
  # On Windows systems the following system exceptions may be raised if the call
  # to *recvfrom* fails:
  # *   Errno::ENETDOWN - the network is down
  # *   Errno::EFAULT - the internal buffer and from parameters on `socket` are
  #     not part of the user address space, or the internal fromlen parameter is
  #     too small to accommodate the peer address
  # *   Errno::EINTR - the (blocking) call was cancelled by an internal call to
  #     the WinSock function WSACancelBlockingCall
  # *   Errno::EINPROGRESS - a blocking Windows Sockets 1.1 call is in progress
  #     or the service provider is still processing a callback function
  # *   Errno::EINVAL - `socket` has not been bound with a call to *bind*, or an
  #     unknown flag was specified, or
  #     [`MSG_OOB`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#MSG_OOB) was
  #     specified for a socket with
  #     [`SO_OOBINLINE`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#SO_OOBINLINE)
  #     enabled, or (for byte stream-style sockets only) the internal len
  #     parameter on `socket` was zero or negative
  # *   Errno::EISCONN - `socket` is already connected. The call to *recvfrom*
  #     is not permitted with a connected socket on a socket that is connection
  #     oriented or connectionless.
  # *   Errno::ENETRESET - the connection has been broken due to the keep-alive
  #     activity detecting a failure while the operation was in progress.
  # *   Errno::EOPNOTSUPP -
  #     [`MSG_OOB`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#MSG_OOB) was
  #     specified, but `socket` is not stream-style such as type
  #     [`SOCK_STREAM`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#SOCK_STREAM).
  #     OOB data is not supported in the communication domain associated with
  #     `socket`, or `socket` is unidirectional and supports only send
  #     operations
  # *   Errno::ESHUTDOWN - `socket` has been shutdown. It is not possible to
  #     call *recvfrom* on a socket after *shutdown* has been invoked.
  # *   Errno::EWOULDBLOCK - `socket` is marked as nonblocking and a  call to
  #     *recvfrom* would block.
  # *   Errno::EMSGSIZE - the message was too large to fit into the specified
  #     buffer and was truncated.
  # *   Errno::ETIMEDOUT - the connection has been dropped, because of a network
  #     failure or because the system on the other end went down without notice
  # *   [`Errno::ECONNRESET`](https://docs.ruby-lang.org/en/2.7.0/Errno/ECONNRESET.html)
  #     - the virtual circuit was reset by the remote side executing a hard or
  #     abortive close. The application should close the socket; it is no longer
  #     usable. On a UDP-datagram socket this error indicates a previous send
  #     operation resulted in an ICMP Port Unreachable message.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom(*arg0); end

  # Receives up to *maxlen* bytes from `socket` using recvfrom(2) after
  # O\_NONBLOCK is set for the underlying file descriptor. *flags* is zero or
  # more of the `MSG_` options. The first element of the results, *mesg*, is the
  # data received. The second element, *sender\_addrinfo*, contains
  # protocol-specific address information of the sender.
  #
  # When recvfrom(2) returns 0,
  # [`Socket#recvfrom_nonblock`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom_nonblock)
  # returns an empty string as data. The meaning depends on the socket: EOF on
  # TCP, empty packet on UDP, etc.
  #
  # ### Parameters
  # *   `maxlen` - the maximum number of bytes to receive from the socket
  # *   `flags` - zero or more of the `MSG_` options
  # *   `outbuf` - destination
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) buffer
  # *   `opts` - keyword hash, supporting `exception: false`
  #
  #
  # ### Example
  #
  # ```ruby
  # # In one file, start this first
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new(AF_INET, SOCK_STREAM, 0)
  # sockaddr = Socket.sockaddr_in(2200, 'localhost')
  # socket.bind(sockaddr)
  # socket.listen(5)
  # client, client_addrinfo = socket.accept
  # begin # emulate blocking recvfrom
  #   pair = client.recvfrom_nonblock(20)
  # rescue IO::WaitReadable
  #   IO.select([client])
  #   retry
  # end
  # data = pair[0].chomp
  # puts "I only received 20 bytes '#{data}'"
  # sleep 1
  # socket.close
  #
  # # In another file, start this second
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new(AF_INET, SOCK_STREAM, 0)
  # sockaddr = Socket.sockaddr_in(2200, 'localhost')
  # socket.connect(sockaddr)
  # socket.puts "Watch this get cut short!"
  # socket.close
  # ```
  #
  # Refer to
  # [`Socket#recvfrom`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom)
  # for the exceptions that may be thrown if the call to *recvfrom\_nonblock*
  # fails.
  #
  # [`Socket#recvfrom_nonblock`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom_nonblock)
  # may raise any error corresponding to recvfrom(2) failure, including
  # Errno::EWOULDBLOCK.
  #
  # If the exception is Errno::EWOULDBLOCK or Errno::EAGAIN, it is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html).
  # So
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # can be used to rescue the exceptions for retrying recvfrom\_nonblock.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`recvfrom_nonblock`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead.
  #
  # ### See
  # *   [`Socket#recvfrom`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom)
  sig do
    params(
      len: ::T.untyped,
      flag: ::T.untyped,
      str: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom_nonblock(len, flag=T.unsafe(nil), str=T.unsafe(nil), exception: T.unsafe(nil)); end

  # Accepts an incoming connection returning an array containing the (integer)
  # file descriptor for the incoming connection, *client\_socket\_fd*, and an
  # [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html),
  # *client\_addrinfo*.
  #
  # ### Example
  #
  # ```ruby
  # # In one script, start this first
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 2200, 'localhost' )
  # socket.bind( sockaddr )
  # socket.listen( 5 )
  # client_fd, client_addrinfo = socket.sysaccept
  # client_socket = Socket.for_fd( client_fd )
  # puts "The client said, '#{client_socket.readline.chomp}'"
  # client_socket.puts "Hello from script one!"
  # socket.close
  #
  # # In another script, start this second
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 2200, 'localhost' )
  # socket.connect( sockaddr )
  # socket.puts "Hello from script 2."
  # puts "The server said, '#{socket.readline.chomp}'"
  # socket.close
  # ```
  #
  # Refer to
  # [`Socket#accept`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept)
  # for the exceptions that may be thrown if the call to *sysaccept* fails.
  #
  # ### See
  # *   [`Socket#accept`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept)
  sig {returns(::T.untyped)}
  def sysaccept(); end

  # yield socket and client address for each a connection accepted via given
  # sockets.
  #
  # The arguments are a list of sockets. The individual argument should be a
  # socket or an array of sockets.
  #
  # This method yields the block sequentially. It means that the next connection
  # is not accepted until the block returns. So concurrent mechanism, thread for
  # example, should be used to service multiple clients at a time.
  sig do
    params(
      sockets: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.accept_loop(*sockets); end

  # Obtains address information for *nodename*:*servname*.
  #
  # Note that
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo)
  # provides the same functionality in an object oriented style.
  #
  # *family* should be an address family such as: :INET, :INET6, etc.
  #
  # *socktype* should be a socket type such as: :STREAM, :DGRAM, :RAW, etc.
  #
  # *protocol* should be a protocol defined in the family, and defaults to 0 for
  # the family.
  #
  # *flags* should be bitwise OR of Socket::AI\_\* constants.
  #
  # ```ruby
  # Socket.getaddrinfo("www.ruby-lang.org", "http", nil, :STREAM)
  # #=> [["AF_INET", 80, "carbon.ruby-lang.org", "221.186.184.68", 2, 1, 6]] # PF_INET/SOCK_STREAM/IPPROTO_TCP
  #
  # Socket.getaddrinfo("localhost", nil)
  # #=> [["AF_INET", 0, "localhost", "127.0.0.1", 2, 1, 6],  # PF_INET/SOCK_STREAM/IPPROTO_TCP
  # #    ["AF_INET", 0, "localhost", "127.0.0.1", 2, 2, 17], # PF_INET/SOCK_DGRAM/IPPROTO_UDP
  # #    ["AF_INET", 0, "localhost", "127.0.0.1", 2, 3, 0]]  # PF_INET/SOCK_RAW/IPPROTO_IP
  # ```
  #
  # *reverse\_lookup* directs the form of the third element, and has to be one
  # of below. If *reverse\_lookup* is omitted, the default value is `nil`.
  #
  # ```
  # +true+, +:hostname+:  hostname is obtained from numeric address using reverse lookup, which may take a time.
  # +false+, +:numeric+:  hostname is same as numeric address.
  # +nil+:              obey to the current +do_not_reverse_lookup+ flag.
  # ```
  #
  # If [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object is
  # preferred, use
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getaddrinfo(*arg0); end

  # Use
  # [`Addrinfo#getnameinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-i-getnameinfo)
  # instead. This method is deprecated for the following reasons:
  #
  # *   Uncommon address representation: 4/16-bytes binary string to represent
  #     IPv4/IPv6 address.
  # *   gethostbyaddr() may take a long time and it may block other threads.
  #     (GVL cannot be released since gethostbyname() is not thread safe.)
  # *   This method uses gethostbyname() function already removed from POSIX.
  #
  #
  # This method obtains the host information for *address*.
  #
  # ```
  # p Socket.gethostbyaddr([221,186,184,68].pack("CCCC"))
  # #=> ["carbon.ruby-lang.org", [], 2, "\xDD\xBA\xB8D"]
  #
  # p Socket.gethostbyaddr([127,0,0,1].pack("CCCC"))
  # ["localhost", [], 2, "\x7F\x00\x00\x01"]
  # p Socket.gethostbyaddr(([0]*15+[1]).pack("C"*16))
  # #=> ["localhost", ["ip6-localhost", "ip6-loopback"], 10,
  #      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.gethostbyaddr(*arg0); end

  # Use
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo)
  # instead. This method is deprecated for the following reasons:
  #
  # *   The 3rd element of the result is the address family of the first
  #     address. The address families of the rest of the addresses are not
  #     returned.
  # *   Uncommon address representation: 4/16-bytes binary string to represent
  #     IPv4/IPv6 address.
  # *   gethostbyname() may take a long time and it may block other threads.
  #     (GVL cannot be released since gethostbyname() is not thread safe.)
  # *   This method uses gethostbyname() function already removed from POSIX.
  #
  #
  # This method obtains the host information for *hostname*.
  #
  # ```ruby
  # p Socket.gethostbyname("hal") #=> ["localhost", ["hal"], 2, "\x7F\x00\x00\x01"]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.gethostbyname(arg0); end

  # Returns the hostname.
  #
  # ```ruby
  # p Socket.gethostname #=> "hal"
  # ```
  #
  # Note that it is not guaranteed to be able to convert to IP address using
  # gethostbyname, getaddrinfo, etc. If you need local IP address, use
  # [`Socket.ip_address_list`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-ip_address_list).
  sig {returns(::T.untyped)}
  def self.gethostname(); end

  # Returns an array of interface addresses. An element of the array is an
  # instance of
  # [`Socket::Ifaddr`](https://docs.ruby-lang.org/en/2.7.0/Socket/Ifaddr.html).
  #
  # This method can be used to find multicast-enabled interfaces:
  #
  # ```ruby
  # pp Socket.getifaddrs.reject {|ifaddr|
  #   !ifaddr.addr.ip? || (ifaddr.flags & Socket::IFF_MULTICAST == 0)
  # }.map {|ifaddr| [ifaddr.name, ifaddr.ifindex, ifaddr.addr] }
  # #=> [["eth0", 2, #<Addrinfo: 221.186.184.67>],
  # #    ["eth0", 2, #<Addrinfo: fe80::216:3eff:fe95:88bb%eth0>]]
  # ```
  #
  # Example result on GNU/Linux:
  #
  # ```ruby
  # pp Socket.getifaddrs
  # #=> [#<Socket::Ifaddr lo UP,LOOPBACK,RUNNING,0x10000 PACKET[protocol=0 lo hatype=772 HOST hwaddr=00:00:00:00:00:00]>,
  # #    #<Socket::Ifaddr eth0 UP,BROADCAST,RUNNING,MULTICAST,0x10000 PACKET[protocol=0 eth0 hatype=1 HOST hwaddr=00:16:3e:95:88:bb] broadcast=PACKET[protocol=0 eth0 hatype=1 HOST hwaddr=ff:ff:ff:ff:ff:ff]>,
  # #    #<Socket::Ifaddr sit0 NOARP PACKET[protocol=0 sit0 hatype=776 HOST hwaddr=00:00:00:00]>,
  # #    #<Socket::Ifaddr lo UP,LOOPBACK,RUNNING,0x10000 127.0.0.1 netmask=255.0.0.0>,
  # #    #<Socket::Ifaddr eth0 UP,BROADCAST,RUNNING,MULTICAST,0x10000 221.186.184.67 netmask=255.255.255.240 broadcast=221.186.184.79>,
  # #    #<Socket::Ifaddr lo UP,LOOPBACK,RUNNING,0x10000 ::1 netmask=ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff>,
  # #    #<Socket::Ifaddr eth0 UP,BROADCAST,RUNNING,MULTICAST,0x10000 fe80::216:3eff:fe95:88bb%eth0 netmask=ffff:ffff:ffff:ffff::>]
  # ```
  #
  # Example result on FreeBSD:
  #
  # ```ruby
  # pp Socket.getifaddrs
  # #=> [#<Socket::Ifaddr usbus0 UP,0x10000 LINK[usbus0]>,
  # #    #<Socket::Ifaddr re0 UP,BROADCAST,RUNNING,MULTICAST,0x800 LINK[re0 3a:d0:40:9a:fe:e8]>,
  # #    #<Socket::Ifaddr re0 UP,BROADCAST,RUNNING,MULTICAST,0x800 10.250.10.18 netmask=255.255.255.? (7 bytes for 16 bytes sockaddr_in) broadcast=10.250.10.255>,
  # #    #<Socket::Ifaddr re0 UP,BROADCAST,RUNNING,MULTICAST,0x800 fe80:2::38d0:40ff:fe9a:fee8 netmask=ffff:ffff:ffff:ffff::>,
  # #    #<Socket::Ifaddr re0 UP,BROADCAST,RUNNING,MULTICAST,0x800 2001:2e8:408:10::12 netmask=UNSPEC>,
  # #    #<Socket::Ifaddr plip0 POINTOPOINT,MULTICAST,0x800 LINK[plip0]>,
  # #    #<Socket::Ifaddr lo0 UP,LOOPBACK,RUNNING,MULTICAST LINK[lo0]>,
  # #    #<Socket::Ifaddr lo0 UP,LOOPBACK,RUNNING,MULTICAST ::1 netmask=ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff>,
  # #    #<Socket::Ifaddr lo0 UP,LOOPBACK,RUNNING,MULTICAST fe80:4::1 netmask=ffff:ffff:ffff:ffff::>,
  # #    #<Socket::Ifaddr lo0 UP,LOOPBACK,RUNNING,MULTICAST 127.0.0.1 netmask=255.?.?.? (5 bytes for 16 bytes sockaddr_in)>]
  # ```
  sig {returns(::T.untyped)}
  def self.getifaddrs(); end

  # Obtains name information for *sockaddr*.
  #
  # *sockaddr* should be one of follows.
  # *   packed sockaddr string such as
  #     [`Socket.sockaddr_in`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-sockaddr_in)(80,
  #     "127.0.0.1")
  # *   3-elements array such as ["AF\_INET", 80, "127.0.0.1"]
  # *   4-elements array such as ["AF\_INET", 80, ignored, "127.0.0.1"]
  #
  #
  # *flags* should be bitwise OR of Socket::NI\_\* constants.
  #
  # Note: The last form is compatible with
  # [`IPSocket#addr`](https://docs.ruby-lang.org/en/2.7.0/IPSocket.html#method-i-addr)
  # and
  # [`IPSocket#peeraddr`](https://docs.ruby-lang.org/en/2.7.0/IPSocket.html#method-i-peeraddr).
  #
  # ```ruby
  # Socket.getnameinfo(Socket.sockaddr_in(80, "127.0.0.1"))       #=> ["localhost", "www"]
  # Socket.getnameinfo(["AF_INET", 80, "127.0.0.1"])              #=> ["localhost", "www"]
  # Socket.getnameinfo(["AF_INET", 80, "localhost", "127.0.0.1"]) #=> ["localhost", "www"]
  # ```
  #
  # If [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object is
  # preferred, use
  # [`Addrinfo#getnameinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-i-getnameinfo).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getnameinfo(*arg0); end

  # Obtains the port number for *service\_name*.
  #
  # If *protocol\_name* is not given, "tcp" is assumed.
  #
  # ```ruby
  # Socket.getservbyname("smtp")          #=> 25
  # Socket.getservbyname("shell")         #=> 514
  # Socket.getservbyname("syslog", "udp") #=> 514
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getservbyname(*arg0); end

  # Obtains the port number for *port*.
  #
  # If *protocol\_name* is not given, "tcp" is assumed.
  #
  # ```ruby
  # Socket.getservbyport(80)         #=> "www"
  # Socket.getservbyport(514, "tcp") #=> "shell"
  # Socket.getservbyport(514, "udp") #=> "syslog"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.getservbyport(*arg0); end

  # Returns local IP addresses as an array.
  #
  # The array contains
  # [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) objects.
  #
  # ```
  # pp Socket.ip_address_list
  # #=> [#<Addrinfo: 127.0.0.1>,
  #      #<Addrinfo: 192.168.0.128>,
  #      #<Addrinfo: ::1>,
  #      ...]
  # ```
  sig {returns(::T.untyped)}
  def self.ip_address_list(); end

  # Packs *port* and *host* as an AF\_INET/AF\_INET6 sockaddr string.
  #
  # ```ruby
  # Socket.sockaddr_in(80, "127.0.0.1")
  # #=> "\x02\x00\x00P\x7F\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
  #
  # Socket.sockaddr_in(80, "::1")
  # #=> "\n\x00\x00P\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pack_sockaddr_in(arg0, arg1); end

  # Packs *path* as an
  # [`AF_UNIX`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#AF_UNIX)
  # sockaddr string.
  #
  # ```ruby
  # Socket.sockaddr_un("/tmp/sock") #=> "\x01\x00/tmp/sock\x00\x00..."
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pack_sockaddr_un(arg0); end

  # Creates a pair of sockets connected each other.
  #
  # *domain* should be a communications domain such as: :INET, :INET6, :UNIX,
  # etc.
  #
  # *socktype* should be a socket type such as: :STREAM, :DGRAM, :RAW, etc.
  #
  # *protocol* should be a protocol defined in the domain, defaults to 0 for the
  # domain.
  #
  # ```ruby
  # s1, s2 = Socket.pair(:UNIX, :STREAM, 0)
  # s1.send "a", 0
  # s1.send "b", 0
  # s1.close
  # p s2.recv(10) #=> "ab"
  # p s2.recv(10) #=> ""
  # p s2.recv(10) #=> ""
  #
  # s1, s2 = Socket.pair(:UNIX, :DGRAM, 0)
  # s1.send "a", 0
  # s1.send "b", 0
  # p s2.recv(10) #=> "a"
  # p s2.recv(10) #=> "b"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pair(*arg0); end

  # Packs *port* and *host* as an AF\_INET/AF\_INET6 sockaddr string.
  #
  # ```ruby
  # Socket.sockaddr_in(80, "127.0.0.1")
  # #=> "\x02\x00\x00P\x7F\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
  #
  # Socket.sockaddr_in(80, "::1")
  # #=> "\n\x00\x00P\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.sockaddr_in(arg0, arg1); end

  # Packs *path* as an
  # [`AF_UNIX`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#AF_UNIX)
  # sockaddr string.
  #
  # ```ruby
  # Socket.sockaddr_un("/tmp/sock") #=> "\x01\x00/tmp/sock\x00\x00..."
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.sockaddr_un(arg0); end

  # Creates a pair of sockets connected each other.
  #
  # *domain* should be a communications domain such as: :INET, :INET6, :UNIX,
  # etc.
  #
  # *socktype* should be a socket type such as: :STREAM, :DGRAM, :RAW, etc.
  #
  # *protocol* should be a protocol defined in the domain, defaults to 0 for the
  # domain.
  #
  # ```ruby
  # s1, s2 = Socket.pair(:UNIX, :STREAM, 0)
  # s1.send "a", 0
  # s1.send "b", 0
  # s1.close
  # p s2.recv(10) #=> "ab"
  # p s2.recv(10) #=> ""
  # p s2.recv(10) #=> ""
  #
  # s1, s2 = Socket.pair(:UNIX, :DGRAM, 0)
  # s1.send "a", 0
  # s1.send "b", 0
  # p s2.recv(10) #=> "a"
  # p s2.recv(10) #=> "b"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socketpair(*arg0); end

  # creates a new socket object connected to host:port using TCP/IP.
  #
  # If local\_host:local\_port is given, the socket is bound to it.
  #
  # The optional last argument *opts* is options represented by a hash. *opts*
  # may have following options:
  #
  # :connect\_timeout
  # :   specify the timeout in seconds.
  # :resolv\_timeout
  # :   specify the name resolution timeout in seconds.
  #
  #
  # If a block is given, the block is called with the socket. The value of the
  # block is returned. The socket is closed when this method returns.
  #
  # If no block is given, the socket is returned.
  #
  # ```ruby
  # Socket.tcp("www.ruby-lang.org", 80) {|sock|
  #   sock.print "GET / HTTP/1.0\r\nHost: www.ruby-lang.org\r\n\r\n"
  #   sock.close_write
  #   puts sock.read
  # }
  # ```
  sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
      local_host: ::T.untyped,
      local_port: ::T.untyped,
      connect_timeout: ::T.untyped,
      blk: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.tcp(host, port, local_host=T.unsafe(nil), local_port=T.unsafe(nil), connect_timeout: T.unsafe(nil), &blk); end

  # creates a TCP/IP server on *port* and calls the block for each connection
  # accepted. The block is called with a socket and a client\_address as an
  # [`Addrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html) object.
  #
  # If *host* is specified, it is used with *port* to determine the server
  # addresses.
  #
  # The socket is **not** closed when the block returns. So application should
  # close it explicitly.
  #
  # This method calls the block sequentially. It means that the next connection
  # is not accepted until the block returns. So concurrent mechanism, thread for
  # example, should be used to service multiple clients at a time.
  #
  # Note that
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo)
  # is used to determine the server socket addresses. When
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo)
  # returns two or more addresses, IPv4 and IPv6 address for example, all of
  # them are used.
  # [`Socket.tcp_server_loop`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-tcp_server_loop)
  # succeeds if one socket can be used at least.
  #
  # ```ruby
  # # Sequential echo server.
  # # It services only one client at a time.
  # Socket.tcp_server_loop(16807) {|sock, client_addrinfo|
  #   begin
  #     IO.copy_stream(sock, sock)
  #   ensure
  #     sock.close
  #   end
  # }
  #
  # # Threaded echo server
  # # It services multiple clients at a time.
  # # Note that it may accept connections too much.
  # Socket.tcp_server_loop(16807) {|sock, client_addrinfo|
  #   Thread.new {
  #     begin
  #       IO.copy_stream(sock, sock)
  #     ensure
  #       sock.close
  #     end
  #   }
  # }
  # ```
  sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.tcp_server_loop(host=T.unsafe(nil), port, &b); end

  # creates TCP/IP server sockets for *host* and *port*. *host* is optional.
  #
  # If no block given, it returns an array of listening sockets.
  #
  # If a block is given, the block is called with the sockets. The value of the
  # block is returned. The socket is closed when this method returns.
  #
  # If *port* is 0, actual port number is chosen dynamically. However all
  # sockets in the result has same port number.
  #
  # ```ruby
  # # tcp_server_sockets returns two sockets.
  # sockets = Socket.tcp_server_sockets(1296)
  # p sockets #=> [#<Socket:fd 3>, #<Socket:fd 4>]
  #
  # # The sockets contains IPv6 and IPv4 sockets.
  # sockets.each {|s| p s.local_address }
  # #=> #<Addrinfo: [::]:1296 TCP>
  # #   #<Addrinfo: 0.0.0.0:1296 TCP>
  #
  # # IPv6 and IPv4 socket has same port number, 53114, even if it is chosen dynamically.
  # sockets = Socket.tcp_server_sockets(0)
  # sockets.each {|s| p s.local_address }
  # #=> #<Addrinfo: [::]:53114 TCP>
  # #   #<Addrinfo: 0.0.0.0:53114 TCP>
  #
  # # The block is called with the sockets.
  # Socket.tcp_server_sockets(0) {|sockets|
  #   p sockets #=> [#<Socket:fd 3>, #<Socket:fd 4>]
  # }
  # ```
  sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.tcp_server_sockets(host=T.unsafe(nil), port); end

  # creates a UDP/IP server on *port* and calls the block for each message
  # arrived. The block is called with the message and its source information.
  #
  # This method allocates sockets internally using *port*. If *host* is
  # specified, it is used conjunction with *port* to determine the server
  # addresses.
  #
  # The *msg* is a string.
  #
  # The *msg\_src* is a
  # [`Socket::UDPSource`](https://docs.ruby-lang.org/en/2.7.0/Socket/UDPSource.html)
  # object. It is used for reply.
  #
  # ```ruby
  # # UDP/IP echo server.
  # Socket.udp_server_loop(9261) {|msg, msg_src|
  #   msg_src.reply msg
  # }
  # ```
  sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp_server_loop(host=T.unsafe(nil), port, &b); end

  # Run UDP/IP server loop on the given sockets.
  #
  # The return value of
  # [`Socket.udp_server_sockets`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-udp_server_sockets)
  # is appropriate for the argument.
  #
  # It calls the block for each message received.
  sig do
    params(
      sockets: ::T.untyped,
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp_server_loop_on(sockets, &b); end

  # Receive UDP/IP packets from the given *sockets*. For each packet received,
  # the block is called.
  #
  # The block receives *msg* and *msg\_src*. *msg* is a string which is the
  # payload of the received packet. *msg\_src* is a
  # [`Socket::UDPSource`](https://docs.ruby-lang.org/en/2.7.0/Socket/UDPSource.html)
  # object which is used for reply.
  #
  # [`Socket.udp_server_loop`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-udp_server_loop)
  # can be implemented using this method as follows.
  #
  # ```
  # udp_server_sockets(host, port) {|sockets|
  #   loop {
  #     readable, _, _ = IO.select(sockets)
  #     udp_server_recv(readable) {|msg, msg_src| ... }
  #   }
  # }
  # ```
  sig do
    params(
      sockets: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp_server_recv(sockets); end

  # Creates UDP/IP sockets for a UDP server.
  #
  # If no block given, it returns an array of sockets.
  #
  # If a block is given, the block is called with the sockets. The value of the
  # block is returned. The sockets are closed when this method returns.
  #
  # If *port* is zero, some port is chosen. But the chosen port is used for the
  # all sockets.
  #
  # ```ruby
  # # UDP/IP echo server
  # Socket.udp_server_sockets(0) {|sockets|
  #   p sockets.first.local_address.ip_port     #=> 32963
  #   Socket.udp_server_loop_on(sockets) {|msg, msg_src|
  #     msg_src.reply msg
  #   }
  # }
  # ```
  sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.udp_server_sockets(host=T.unsafe(nil), port); end

  # creates a new socket connected to path using UNIX socket socket.
  #
  # If a block is given, the block is called with the socket. The value of the
  # block is returned. The socket is closed when this method returns.
  #
  # If no block is given, the socket is returned.
  #
  # ```ruby
  # # talk to /tmp/sock socket.
  # Socket.unix("/tmp/sock") {|sock|
  #   t = Thread.new { IO.copy_stream(sock, STDOUT) }
  #   IO.copy_stream(STDIN, sock)
  #   t.join
  # }
  # ```
  sig do
    params(
      path: ::T.untyped,
      blk: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix(path, &blk); end

  # creates a UNIX socket server on *path*. It calls the block for each socket
  # accepted.
  #
  # If *host* is specified, it is used with *port* to determine the server
  # ports.
  #
  # The socket is **not** closed when the block returns. So application should
  # close it.
  #
  # This method deletes the socket file pointed by *path* at first if the file
  # is a socket file and it is owned by the user of the application. This is
  # safe only if the directory of *path* is not changed by a malicious user. So
  # don't use /tmp/malicious-users-directory/socket. Note that /tmp/socket and
  # /tmp/your-private-directory/socket is safe assuming that /tmp has sticky
  # bit.
  #
  # ```ruby
  # # Sequential echo server.
  # # It services only one client at a time.
  # Socket.unix_server_loop("/tmp/sock") {|sock, client_addrinfo|
  #   begin
  #     IO.copy_stream(sock, sock)
  #   ensure
  #     sock.close
  #   end
  # }
  # ```
  sig do
    params(
      path: ::T.untyped,
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix_server_loop(path, &b); end

  # creates a UNIX server socket on *path*
  #
  # If no block given, it returns a listening socket.
  #
  # If a block is given, it is called with the socket and the block value is
  # returned. When the block exits, the socket is closed and the socket file is
  # removed.
  #
  # ```ruby
  # socket = Socket.unix_server_socket("/tmp/s")
  # p socket                  #=> #<Socket:fd 3>
  # p socket.local_address    #=> #<Addrinfo: /tmp/s SOCK_STREAM>
  #
  # Socket.unix_server_socket("/tmp/sock") {|s|
  #   p s                     #=> #<Socket:fd 3>
  #   p s.local_address       #=> # #<Addrinfo: /tmp/sock SOCK_STREAM>
  # }
  # ```
  sig do
    params(
      path: ::T.untyped,
      blk: T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix_server_socket(path, &blk); end

  # Unpacks *sockaddr* into port and ip\_address.
  #
  # *sockaddr* should be a string or an addrinfo for AF\_INET/AF\_INET6.
  #
  # ```ruby
  # sockaddr = Socket.sockaddr_in(80, "127.0.0.1")
  # p sockaddr #=> "\x02\x00\x00P\x7F\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
  # p Socket.unpack_sockaddr_in(sockaddr) #=> [80, "127.0.0.1"]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unpack_sockaddr_in(arg0); end

  # Unpacks *sockaddr* into path.
  #
  # *sockaddr* should be a string or an addrinfo for
  # [`AF_UNIX`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#AF_UNIX).
  #
  # ```ruby
  # sockaddr = Socket.sockaddr_un("/tmp/sock")
  # p Socket.unpack_sockaddr_un(sockaddr) #=> "/tmp/sock"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unpack_sockaddr_un(arg0); end
end

# [`Socket::AncillaryData`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html)
# represents the ancillary data (control information) used by sendmsg and
# recvmsg system call. It contains socket
# [`family`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html#method-i-family),
# control message (cmsg)
# [`level`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html#method-i-level),
# cmsg
# [`type`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html#method-i-type)
# and cmsg
# [`data`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html#method-i-data).
class Socket::AncillaryData
  # tests the level and type of *ancillarydata*.
  #
  # ```ruby
  # ancdata = Socket::AncillaryData.new(:INET6, :IPV6, :PKTINFO, "")
  # ancdata.cmsg_is?(Socket::IPPROTO_IPV6, Socket::IPV6_PKTINFO) #=> true
  # ancdata.cmsg_is?(:IPV6, :PKTINFO)       #=> true
  # ancdata.cmsg_is?(:IP, :PKTINFO)         #=> false
  # ancdata.cmsg_is?(:SOCKET, :RIGHTS)      #=> false
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cmsg_is?(arg0, arg1); end

  # returns the cmsg data as a string.
  #
  # ```ruby
  # p Socket::AncillaryData.new(:INET6, :IPV6, :PKTINFO, "").data
  # #=> ""
  # ```
  sig {returns(::T.untyped)}
  def data(); end

  # returns the socket family as an integer.
  #
  # ```ruby
  # p Socket::AncillaryData.new(:INET6, :IPV6, :PKTINFO, "").family
  # #=> 10
  # ```
  sig {returns(::T.untyped)}
  def family(); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
    )
    .void
  end
  def initialize(arg0, arg1, arg2, arg3); end

  # returns a string which shows ancillarydata in human-readable form.
  #
  # ```ruby
  # p Socket::AncillaryData.new(:INET6, :IPV6, :PKTINFO, "").inspect
  # #=> "#<Socket::AncillaryData: INET6 IPV6 PKTINFO \"\">"
  # ```
  sig {returns(::T.untyped)}
  def inspect(); end

  # Returns the data in *ancillarydata* as an int.
  #
  # The size and endian is dependent on the host.
  #
  # ```ruby
  # ancdata = Socket::AncillaryData.int(:UNIX, :SOCKET, :RIGHTS, STDERR.fileno)
  # p ancdata.int #=> 2
  # ```
  sig {returns(::T.untyped)}
  def int(); end

  # Extracts addr, ifindex and spec\_dst from IP\_PKTINFO ancillary data.
  #
  # IP\_PKTINFO is not standard.
  #
  # Supported platform: GNU/Linux
  #
  # ```ruby
  # addr = Addrinfo.ip("127.0.0.1")
  # ifindex = 0
  # spec_dest = Addrinfo.ip("127.0.0.1")
  # ancdata = Socket::AncillaryData.ip_pktinfo(addr, ifindex, spec_dest)
  # p ancdata.ip_pktinfo
  # #=> [#<Addrinfo: 127.0.0.1>, 0, #<Addrinfo: 127.0.0.1>]
  # ```
  sig {returns(::T.untyped)}
  def ip_pktinfo(); end

  # Extracts addr and ifindex from IPV6\_PKTINFO ancillary data.
  #
  # IPV6\_PKTINFO is defined by RFC 3542.
  #
  # ```ruby
  # addr = Addrinfo.ip("::1")
  # ifindex = 0
  # ancdata = Socket::AncillaryData.ipv6_pktinfo(addr, ifindex)
  # p ancdata.ipv6_pktinfo #=> [#<Addrinfo: ::1>, 0]
  # ```
  sig {returns(::T.untyped)}
  def ipv6_pktinfo(); end

  # Extracts addr from IPV6\_PKTINFO ancillary data.
  #
  # IPV6\_PKTINFO is defined by RFC 3542.
  #
  # ```ruby
  # addr = Addrinfo.ip("::1")
  # ifindex = 0
  # ancdata = Socket::AncillaryData.ipv6_pktinfo(addr, ifindex)
  # p ancdata.ipv6_pktinfo_addr #=> #<Addrinfo: ::1>
  # ```
  sig {returns(::T.untyped)}
  def ipv6_pktinfo_addr(); end

  # Extracts ifindex from IPV6\_PKTINFO ancillary data.
  #
  # IPV6\_PKTINFO is defined by RFC 3542.
  #
  # ```ruby
  # addr = Addrinfo.ip("::1")
  # ifindex = 0
  # ancdata = Socket::AncillaryData.ipv6_pktinfo(addr, ifindex)
  # p ancdata.ipv6_pktinfo_ifindex #=> 0
  # ```
  sig {returns(::T.untyped)}
  def ipv6_pktinfo_ifindex(); end

  # returns the cmsg level as an integer.
  #
  # ```ruby
  # p Socket::AncillaryData.new(:INET6, :IPV6, :PKTINFO, "").level
  # #=> 41
  # ```
  sig {returns(::T.untyped)}
  def level(); end

  # returns the timestamp as a time object.
  #
  # *ancillarydata* should be one of following type:
  # *   SOL\_SOCKET/SCM\_TIMESTAMP (microsecond) GNU/Linux, FreeBSD, NetBSD,
  #     OpenBSD, Solaris, MacOS X
  # *   SOL\_SOCKET/SCM\_TIMESTAMPNS (nanosecond) GNU/Linux
  # *   SOL\_SOCKET/SCM\_BINTIME (2\*\*(-64) second) FreeBSD
  #
  #     [`Addrinfo.udp`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-udp)("127.0.0.1",
  #     0).bind {|s1|
  #
  # ```ruby
  # Addrinfo.udp("127.0.0.1", 0).bind {|s2|
  #   s1.setsockopt(:SOCKET, :TIMESTAMP, true)
  #   s2.send "a", 0, s1.local_address
  #   ctl = s1.recvmsg.last
  #   p ctl    #=> #<Socket::AncillaryData: INET SOCKET TIMESTAMP 2009-02-24 17:35:46.775581>
  #   t = ctl.timestamp
  #   p t      #=> 2009-02-24 17:35:46 +0900
  #   p t.usec #=> 775581
  #   p t.nsec #=> 775581000
  # }
  # ```
  #
  #     }
  sig {returns(::T.untyped)}
  def timestamp(); end

  # returns the cmsg type as an integer.
  #
  # ```ruby
  # p Socket::AncillaryData.new(:INET6, :IPV6, :PKTINFO, "").type
  # #=> 2
  # ```
  sig {returns(::T.untyped)}
  def type(); end

  # returns the array of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # objects for SCM\_RIGHTS control message in UNIX domain socket.
  #
  # The class of the [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects
  # in the array is [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) or
  # [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html).
  #
  # The array is attached to *ancillarydata* when it is instantiated. For
  # example,
  # [`BasicSocket#recvmsg`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-recvmsg)
  # attach the array when receives a SCM\_RIGHTS control message and
  # :scm\_rights=>true option is given.
  #
  # ```ruby
  # # recvmsg needs :scm_rights=>true for unix_rights
  # s1, s2 = UNIXSocket.pair
  # p s1                                         #=> #<UNIXSocket:fd 3>
  # s1.sendmsg "stdin and a socket", 0, nil, Socket::AncillaryData.unix_rights(STDIN, s1)
  # _, _, _, ctl = s2.recvmsg(:scm_rights=>true)
  # p ctl                                        #=> #<Socket::AncillaryData: UNIX SOCKET RIGHTS 6 7>
  # p ctl.unix_rights                            #=> [#<IO:fd 6>, #<Socket:fd 7>]
  # p File.identical?(STDIN, ctl.unix_rights[0]) #=> true
  # p File.identical?(s1, ctl.unix_rights[1])    #=> true
  #
  # # If :scm_rights=>true is not given, unix_rights returns nil
  # s1, s2 = UNIXSocket.pair
  # s1.sendmsg "stdin and a socket", 0, nil, Socket::AncillaryData.unix_rights(STDIN, s1)
  # _, _, _, ctl = s2.recvmsg
  # p ctl #=> #<Socket::AncillaryData: UNIX SOCKET RIGHTS 6 7>
  # p ctl.unix_rights #=> nil
  # ```
  sig {returns(::T.untyped)}
  def unix_rights(); end

  # Creates a new
  # [`Socket::AncillaryData`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html)
  # object which contains a int as data.
  #
  # The size and endian is dependent on the host.
  #
  # ```ruby
  # require 'socket'
  #
  # p Socket::AncillaryData.int(:UNIX, :SOCKET, :RIGHTS, STDERR.fileno)
  # #=> #<Socket::AncillaryData: UNIX SOCKET RIGHTS 2>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.int(arg0, arg1, arg2, arg3); end

  # Returns new ancillary data for IP\_PKTINFO.
  #
  # If spec\_dst is not given, addr is used.
  #
  # IP\_PKTINFO is not standard.
  #
  # Supported platform: GNU/Linux
  #
  # ```ruby
  # addr = Addrinfo.ip("127.0.0.1")
  # ifindex = 0
  # spec_dst = Addrinfo.ip("127.0.0.1")
  # p Socket::AncillaryData.ip_pktinfo(addr, ifindex, spec_dst)
  # #=> #<Socket::AncillaryData: INET IP PKTINFO 127.0.0.1 ifindex:0 spec_dst:127.0.0.1>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ip_pktinfo(*arg0); end

  # Returns new ancillary data for IPV6\_PKTINFO.
  #
  # IPV6\_PKTINFO is defined by RFC 3542.
  #
  # ```ruby
  # addr = Addrinfo.ip("::1")
  # ifindex = 0
  # p Socket::AncillaryData.ipv6_pktinfo(addr, ifindex)
  # #=> #<Socket::AncillaryData: INET6 IPV6 PKTINFO ::1 ifindex:0>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ipv6_pktinfo(arg0, arg1); end

  # Creates a new
  # [`Socket::AncillaryData`](https://docs.ruby-lang.org/en/2.7.0/Socket/AncillaryData.html)
  # object which contains file descriptors as data.
  #
  # ```ruby
  # p Socket::AncillaryData.unix_rights(STDERR)
  # #=> #<Socket::AncillaryData: UNIX SOCKET RIGHTS 2>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unix_rights(*arg0); end
end

# [`Socket::Constants`](https://docs.ruby-lang.org/en/2.7.0/Socket/Constants.html)
# provides socket-related constants. All possible socket constants are listed in
# the documentation but they may not all be present on your platform.
#
# If the underlying platform doesn't define a constant the corresponding Ruby
# constant is not defined.
module Socket::Constants
  # AppleTalk protocol
  AF_APPLETALK = ::T.let(nil, ::T.untyped)
  # AX.25 protocol
  AF_AX25 = ::T.let(nil, ::T.untyped)
  # IPv4 protocol
  AF_INET = ::T.let(nil, ::T.untyped)
  # IPv6 protocol
  AF_INET6 = ::T.let(nil, ::T.untyped)
  # IPX protocol
  AF_IPX = ::T.let(nil, ::T.untyped)
  # Integrated Services Digital Network
  AF_ISDN = ::T.let(nil, ::T.untyped)
  # Host-internal protocols
  AF_LOCAL = ::T.let(nil, ::T.untyped)
  # Maximum address family for this platform
  AF_MAX = ::T.let(nil, ::T.untyped)
  # Direct link-layer access
  AF_PACKET = ::T.let(nil, ::T.untyped)
  # Internal routing protocol
  AF_ROUTE = ::T.let(nil, ::T.untyped)
  # IBM SNA protocol
  AF_SNA = ::T.let(nil, ::T.untyped)
  # UNIX sockets
  AF_UNIX = ::T.let(nil, ::T.untyped)
  # Unspecified protocol, any supported address family
  AF_UNSPEC = ::T.let(nil, ::T.untyped)
  # Accept only if any address is assigned
  AI_ADDRCONFIG = ::T.let(nil, ::T.untyped)
  # Allow all addresses
  AI_ALL = ::T.let(nil, ::T.untyped)
  # Fill in the canonical name
  AI_CANONNAME = ::T.let(nil, ::T.untyped)
  # Prevent host name resolution
  AI_NUMERICHOST = ::T.let(nil, ::T.untyped)
  # Prevent service name resolution
  AI_NUMERICSERV = ::T.let(nil, ::T.untyped)
  # Get address to use with bind()
  AI_PASSIVE = ::T.let(nil, ::T.untyped)
  # Accept IPv4-mapped IPv6 addresses
  AI_V4MAPPED = ::T.let(nil, ::T.untyped)
  # Address family for hostname not supported
  EAI_ADDRFAMILY = ::T.let(nil, ::T.untyped)
  # Temporary failure in name resolution
  EAI_AGAIN = ::T.let(nil, ::T.untyped)
  # Invalid flags
  EAI_BADFLAGS = ::T.let(nil, ::T.untyped)
  # Non-recoverable failure in name resolution
  EAI_FAIL = ::T.let(nil, ::T.untyped)
  # Address family not supported
  EAI_FAMILY = ::T.let(nil, ::T.untyped)
  # Memory allocation failure
  EAI_MEMORY = ::T.let(nil, ::T.untyped)
  # No address associated with hostname
  EAI_NODATA = ::T.let(nil, ::T.untyped)
  # Hostname nor servname, or not known
  EAI_NONAME = ::T.let(nil, ::T.untyped)
  # Argument buffer overflow
  EAI_OVERFLOW = ::T.let(nil, ::T.untyped)
  # Servname not supported for socket type
  EAI_SERVICE = ::T.let(nil, ::T.untyped)
  # [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html) type not
  # supported
  EAI_SOCKTYPE = ::T.let(nil, ::T.untyped)
  # System error returned in errno
  EAI_SYSTEM = ::T.let(nil, ::T.untyped)
  # receive all multicast packets
  IFF_ALLMULTI = ::T.let(nil, ::T.untyped)
  # auto media select active
  IFF_AUTOMEDIA = ::T.let(nil, ::T.untyped)
  # broadcast address valid
  IFF_BROADCAST = ::T.let(nil, ::T.untyped)
  # turn on debugging
  IFF_DEBUG = ::T.let(nil, ::T.untyped)
  # dialup device with changing addresses
  IFF_DYNAMIC = ::T.let(nil, ::T.untyped)
  # loopback net
  IFF_LOOPBACK = ::T.let(nil, ::T.untyped)
  # master of a load balancer
  IFF_MASTER = ::T.let(nil, ::T.untyped)
  # supports multicast
  IFF_MULTICAST = ::T.let(nil, ::T.untyped)
  # no address resolution protocol
  IFF_NOARP = ::T.let(nil, ::T.untyped)
  # avoid use of trailers
  IFF_NOTRAILERS = ::T.let(nil, ::T.untyped)
  # point-to-point link
  IFF_POINTOPOINT = ::T.let(nil, ::T.untyped)
  # can set media type
  IFF_PORTSEL = ::T.let(nil, ::T.untyped)
  # receive all packets
  IFF_PROMISC = ::T.let(nil, ::T.untyped)
  # resources allocated
  IFF_RUNNING = ::T.let(nil, ::T.untyped)
  # slave of a load balancer
  IFF_SLAVE = ::T.let(nil, ::T.untyped)
  # interface is up
  IFF_UP = ::T.let(nil, ::T.untyped)
  # Maximum interface name size
  IFNAMSIZ = ::T.let(nil, ::T.untyped)
  # Maximum interface name size
  IF_NAMESIZE = ::T.let(nil, ::T.untyped)
  # Multicast group for all systems on this subset
  INADDR_ALLHOSTS_GROUP = ::T.let(nil, ::T.untyped)
  # A socket bound to
  # [`INADDR_ANY`](https://docs.ruby-lang.org/en/2.7.0/Socket/Constants.html#INADDR_ANY)
  # receives packets from all interfaces and sends from the default IP address
  INADDR_ANY = ::T.let(nil, ::T.untyped)
  # The network broadcast address
  INADDR_BROADCAST = ::T.let(nil, ::T.untyped)
  # The loopback address
  INADDR_LOOPBACK = ::T.let(nil, ::T.untyped)
  # The last local network multicast group
  INADDR_MAX_LOCAL_GROUP = ::T.let(nil, ::T.untyped)
  # A bitmask for matching no valid IP address
  INADDR_NONE = ::T.let(nil, ::T.untyped)
  # The reserved multicast group
  INADDR_UNSPEC_GROUP = ::T.let(nil, ::T.untyped)
  # Maximum length of an IPv6 address string
  INET6_ADDRSTRLEN = ::T.let(nil, ::T.untyped)
  # Maximum length of an IPv4 address string
  INET_ADDRSTRLEN = ::T.let(nil, ::T.untyped)
  # Default minimum address for bind or connect
  IPPORT_RESERVED = ::T.let(nil, ::T.untyped)
  # Default maximum address for bind or connect
  IPPORT_USERRESERVED = ::T.let(nil, ::T.untyped)
  # IP6 auth header
  IPPROTO_AH = ::T.let(nil, ::T.untyped)
  # IP6 destination option
  IPPROTO_DSTOPTS = ::T.let(nil, ::T.untyped)
  # Exterior Gateway Protocol
  IPPROTO_EGP = ::T.let(nil, ::T.untyped)
  # IP6 Encapsulated Security Payload
  IPPROTO_ESP = ::T.let(nil, ::T.untyped)
  # IP6 fragmentation header
  IPPROTO_FRAGMENT = ::T.let(nil, ::T.untyped)
  # IP6 hop-by-hop options
  IPPROTO_HOPOPTS = ::T.let(nil, ::T.untyped)
  # Control message protocol
  IPPROTO_ICMP = ::T.let(nil, ::T.untyped)
  # ICMP6
  IPPROTO_ICMPV6 = ::T.let(nil, ::T.untyped)
  # XNS IDP
  IPPROTO_IDP = ::T.let(nil, ::T.untyped)
  # Group Management Protocol
  IPPROTO_IGMP = ::T.let(nil, ::T.untyped)
  # Dummy protocol for IP
  IPPROTO_IP = ::T.let(nil, ::T.untyped)
  # IP6 header
  IPPROTO_IPV6 = ::T.let(nil, ::T.untyped)
  # IP6 no next header
  IPPROTO_NONE = ::T.let(nil, ::T.untyped)
  # PARC Universal Packet protocol
  IPPROTO_PUP = ::T.let(nil, ::T.untyped)
  # Raw IP packet
  IPPROTO_RAW = ::T.let(nil, ::T.untyped)
  # IP6 routing header
  IPPROTO_ROUTING = ::T.let(nil, ::T.untyped)
  # TCP
  IPPROTO_TCP = ::T.let(nil, ::T.untyped)
  # ISO transport protocol class 4
  IPPROTO_TP = ::T.let(nil, ::T.untyped)
  # UDP
  IPPROTO_UDP = ::T.let(nil, ::T.untyped)
  # Checksum offset for raw sockets
  IPV6_CHECKSUM = ::T.let(nil, ::T.untyped)
  # Destination option
  IPV6_DSTOPTS = ::T.let(nil, ::T.untyped)
  # Hop limit
  IPV6_HOPLIMIT = ::T.let(nil, ::T.untyped)
  # Hop-by-hop option
  IPV6_HOPOPTS = ::T.let(nil, ::T.untyped)
  # Join a group membership
  IPV6_JOIN_GROUP = ::T.let(nil, ::T.untyped)
  # Leave a group membership
  IPV6_LEAVE_GROUP = ::T.let(nil, ::T.untyped)
  # IP6 multicast hops
  IPV6_MULTICAST_HOPS = ::T.let(nil, ::T.untyped)
  # IP6 multicast interface
  IPV6_MULTICAST_IF = ::T.let(nil, ::T.untyped)
  # IP6 multicast loopback
  IPV6_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  # Next hop address
  IPV6_NEXTHOP = ::T.let(nil, ::T.untyped)
  # Receive packet information with datagram
  IPV6_PKTINFO = ::T.let(nil, ::T.untyped)
  # Receive all IP6 options for response
  IPV6_RECVDSTOPTS = ::T.let(nil, ::T.untyped)
  # Receive hop limit with datagram
  IPV6_RECVHOPLIMIT = ::T.let(nil, ::T.untyped)
  # Receive hop-by-hop options
  IPV6_RECVHOPOPTS = ::T.let(nil, ::T.untyped)
  # Receive destination IP address and incoming interface
  IPV6_RECVPKTINFO = ::T.let(nil, ::T.untyped)
  # Receive routing header
  IPV6_RECVRTHDR = ::T.let(nil, ::T.untyped)
  # Receive traffic class
  IPV6_RECVTCLASS = ::T.let(nil, ::T.untyped)
  # Allows removal of sticky routing headers
  IPV6_RTHDR = ::T.let(nil, ::T.untyped)
  # Allows removal of sticky destination options header
  IPV6_RTHDRDSTOPTS = ::T.let(nil, ::T.untyped)
  # Routing header type 0
  IPV6_RTHDR_TYPE_0 = ::T.let(nil, ::T.untyped)
  # Specify the traffic class
  IPV6_TCLASS = ::T.let(nil, ::T.untyped)
  # IP6 unicast hops
  IPV6_UNICAST_HOPS = ::T.let(nil, ::T.untyped)
  # Only bind IPv6 with a wildcard bind
  IPV6_V6ONLY = ::T.let(nil, ::T.untyped)
  # Add a multicast group membership
  IP_ADD_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  # Add a multicast group membership
  IP_ADD_SOURCE_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  # Block IPv4 multicast packets with a give source address
  IP_BLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  # Default multicast loopback
  IP_DEFAULT_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  # Default multicast TTL
  IP_DEFAULT_MULTICAST_TTL = ::T.let(nil, ::T.untyped)
  # Drop a multicast group membership
  IP_DROP_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  # Drop a multicast group membership
  IP_DROP_SOURCE_MEMBERSHIP = ::T.let(nil, ::T.untyped)
  # Allow binding to nonexistent IP addresses
  IP_FREEBIND = ::T.let(nil, ::T.untyped)
  # Header is included with data
  IP_HDRINCL = ::T.let(nil, ::T.untyped)
  # IPsec security policy
  IP_IPSEC_POLICY = ::T.let(nil, ::T.untyped)
  # Maximum number multicast groups a socket can join
  IP_MAX_MEMBERSHIPS = ::T.let(nil, ::T.untyped)
  # Minimum TTL allowed for received packets
  IP_MINTTL = ::T.let(nil, ::T.untyped)
  # Multicast source filtering
  IP_MSFILTER = ::T.let(nil, ::T.untyped)
  # The Maximum Transmission Unit of the socket
  IP_MTU = ::T.let(nil, ::T.untyped)
  # Path MTU discovery
  IP_MTU_DISCOVER = ::T.let(nil, ::T.untyped)
  # IP multicast interface
  IP_MULTICAST_IF = ::T.let(nil, ::T.untyped)
  # IP multicast loopback
  IP_MULTICAST_LOOP = ::T.let(nil, ::T.untyped)
  # IP multicast TTL
  IP_MULTICAST_TTL = ::T.let(nil, ::T.untyped)
  # IP options to be included in packets
  IP_OPTIONS = ::T.let(nil, ::T.untyped)
  # Retrieve security context with datagram
  IP_PASSSEC = ::T.let(nil, ::T.untyped)
  # Receive packet information with datagrams
  IP_PKTINFO = ::T.let(nil, ::T.untyped)
  # Receive packet options with datagrams
  IP_PKTOPTIONS = ::T.let(nil, ::T.untyped)
  # Always send DF frames
  IP_PMTUDISC_DO = ::T.let(nil, ::T.untyped)
  # Never send DF frames
  IP_PMTUDISC_DONT = ::T.let(nil, ::T.untyped)
  # Use per-route hints
  IP_PMTUDISC_WANT = ::T.let(nil, ::T.untyped)
  # Enable extended reliable error message passing
  IP_RECVERR = ::T.let(nil, ::T.untyped)
  # Receive all IP options with datagram
  IP_RECVOPTS = ::T.let(nil, ::T.untyped)
  # Receive all IP options for response
  IP_RECVRETOPTS = ::T.let(nil, ::T.untyped)
  # Receive TOS with incoming packets
  IP_RECVTOS = ::T.let(nil, ::T.untyped)
  # Receive IP TTL with datagrams
  IP_RECVTTL = ::T.let(nil, ::T.untyped)
  # IP options to be included in datagrams
  IP_RETOPTS = ::T.let(nil, ::T.untyped)
  # Notify transit routers to more closely examine the contents of an IP packet
  IP_ROUTER_ALERT = ::T.let(nil, ::T.untyped)
  # IP type-of-service
  IP_TOS = ::T.let(nil, ::T.untyped)
  # Transparent proxy
  IP_TRANSPARENT = ::T.let(nil, ::T.untyped)
  # IP time-to-live
  IP_TTL = ::T.let(nil, ::T.untyped)
  # Unblock IPv4 multicast packets with a give source address
  IP_UNBLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  IP_XFRM_POLICY = ::T.let(nil, ::T.untyped)
  # Block multicast packets from this source
  MCAST_BLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  # Exclusive multicast source filter
  MCAST_EXCLUDE = ::T.let(nil, ::T.untyped)
  # Inclusive multicast source filter
  MCAST_INCLUDE = ::T.let(nil, ::T.untyped)
  # Join a multicast group
  MCAST_JOIN_GROUP = ::T.let(nil, ::T.untyped)
  # Join a multicast source group
  MCAST_JOIN_SOURCE_GROUP = ::T.let(nil, ::T.untyped)
  # Leave a multicast group
  MCAST_LEAVE_GROUP = ::T.let(nil, ::T.untyped)
  # Leave a multicast source group
  MCAST_LEAVE_SOURCE_GROUP = ::T.let(nil, ::T.untyped)
  # Multicast source filtering
  MCAST_MSFILTER = ::T.let(nil, ::T.untyped)
  # Unblock multicast packets from this source
  MCAST_UNBLOCK_SOURCE = ::T.let(nil, ::T.untyped)
  # Confirm path validity
  MSG_CONFIRM = ::T.let(nil, ::T.untyped)
  # Control data lost before delivery
  MSG_CTRUNC = ::T.let(nil, ::T.untyped)
  # Send without using the routing tables
  MSG_DONTROUTE = ::T.let(nil, ::T.untyped)
  # This message should be non-blocking
  MSG_DONTWAIT = ::T.let(nil, ::T.untyped)
  # [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) completes record
  MSG_EOR = ::T.let(nil, ::T.untyped)
  # Fetch message from error queue
  MSG_ERRQUEUE = ::T.let(nil, ::T.untyped)
  # Reduce step of the handshake process
  MSG_FASTOPEN = ::T.let(nil, ::T.untyped)
  MSG_FIN = ::T.let(nil, ::T.untyped)
  # Sender will send more
  MSG_MORE = ::T.let(nil, ::T.untyped)
  # Do not generate SIGPIPE
  MSG_NOSIGNAL = ::T.let(nil, ::T.untyped)
  # [`Process`](https://docs.ruby-lang.org/en/2.7.0/Process.html) out-of-band
  # data
  MSG_OOB = ::T.let(nil, ::T.untyped)
  # Peek at incoming message
  MSG_PEEK = ::T.let(nil, ::T.untyped)
  # Wait for full request
  MSG_PROXY = ::T.let(nil, ::T.untyped)
  MSG_RST = ::T.let(nil, ::T.untyped)
  MSG_SYN = ::T.let(nil, ::T.untyped)
  # [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) discarded before
  # delivery
  MSG_TRUNC = ::T.let(nil, ::T.untyped)
  # Wait for full request or error
  MSG_WAITALL = ::T.let(nil, ::T.untyped)
  # The service specified is a datagram service (looks up UDP ports)
  NI_DGRAM = ::T.let(nil, ::T.untyped)
  # Maximum length of a hostname
  NI_MAXHOST = ::T.let(nil, ::T.untyped)
  # Maximum length of a service name
  NI_MAXSERV = ::T.let(nil, ::T.untyped)
  # A name is required
  NI_NAMEREQD = ::T.let(nil, ::T.untyped)
  # An FQDN is not required for local hosts, return only the local part
  NI_NOFQDN = ::T.let(nil, ::T.untyped)
  # Return a numeric address
  NI_NUMERICHOST = ::T.let(nil, ::T.untyped)
  # Return the service name as a digit string
  NI_NUMERICSERV = ::T.let(nil, ::T.untyped)
  # AppleTalk protocol
  PF_APPLETALK = ::T.let(nil, ::T.untyped)
  # AX.25 protocol
  PF_AX25 = ::T.let(nil, ::T.untyped)
  # IPv4 protocol
  PF_INET = ::T.let(nil, ::T.untyped)
  # IPv6 protocol
  PF_INET6 = ::T.let(nil, ::T.untyped)
  # IPX protocol
  PF_IPX = ::T.let(nil, ::T.untyped)
  # Integrated Services Digital Network
  PF_ISDN = ::T.let(nil, ::T.untyped)
  PF_KEY = ::T.let(nil, ::T.untyped)
  # Host-internal protocols
  PF_LOCAL = ::T.let(nil, ::T.untyped)
  # Maximum address family for this platform
  PF_MAX = ::T.let(nil, ::T.untyped)
  # Direct link-layer access
  PF_PACKET = ::T.let(nil, ::T.untyped)
  # Internal routing protocol
  PF_ROUTE = ::T.let(nil, ::T.untyped)
  # IBM SNA protocol
  PF_SNA = ::T.let(nil, ::T.untyped)
  # UNIX sockets
  PF_UNIX = ::T.let(nil, ::T.untyped)
  # Unspecified protocol, any supported address family
  PF_UNSPEC = ::T.let(nil, ::T.untyped)
  # The sender's credentials
  SCM_CREDENTIALS = ::T.let(nil, ::T.untyped)
  # Access rights
  SCM_RIGHTS = ::T.let(nil, ::T.untyped)
  # Timestamp (timeval)
  SCM_TIMESTAMP = ::T.let(nil, ::T.untyped)
  # Timestamp (timespec list) (Linux 2.6.30)
  SCM_TIMESTAMPING = ::T.let(nil, ::T.untyped)
  # Timespec (timespec)
  SCM_TIMESTAMPNS = ::T.let(nil, ::T.untyped)
  # Wifi status (Linux 3.3)
  SCM_WIFI_STATUS = ::T.let(nil, ::T.untyped)
  # Shut down the reading side of the socket
  SHUT_RD = ::T.let(nil, ::T.untyped)
  # Shut down the both sides of the socket
  SHUT_RDWR = ::T.let(nil, ::T.untyped)
  # Shut down the writing side of the socket
  SHUT_WR = ::T.let(nil, ::T.untyped)
  # A datagram socket provides connectionless, unreliable messaging
  SOCK_DGRAM = ::T.let(nil, ::T.untyped)
  # Device-level packet access
  SOCK_PACKET = ::T.let(nil, ::T.untyped)
  # A raw socket provides low-level access for direct access or implementing
  # network protocols
  SOCK_RAW = ::T.let(nil, ::T.untyped)
  # A reliable datagram socket provides reliable delivery of messages
  SOCK_RDM = ::T.let(nil, ::T.untyped)
  # A sequential packet socket provides sequenced, reliable two-way connection
  # for datagrams
  SOCK_SEQPACKET = ::T.let(nil, ::T.untyped)
  # A stream socket provides a sequenced, reliable two-way connection for a byte
  # stream
  SOCK_STREAM = ::T.let(nil, ::T.untyped)
  # IP socket options
  SOL_IP = ::T.let(nil, ::T.untyped)
  # Socket-level options
  SOL_SOCKET = ::T.let(nil, ::T.untyped)
  # TCP socket options
  SOL_TCP = ::T.let(nil, ::T.untyped)
  # UDP socket options
  SOL_UDP = ::T.let(nil, ::T.untyped)
  # Maximum connection requests that may be queued for a socket
  SOMAXCONN = ::T.let(nil, ::T.untyped)
  # [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html) has had listen()
  # called on it
  SO_ACCEPTCONN = ::T.let(nil, ::T.untyped)
  # Attach an accept filter
  SO_ATTACH_FILTER = ::T.let(nil, ::T.untyped)
  # Only send packets from the given interface
  SO_BINDTODEVICE = ::T.let(nil, ::T.untyped)
  # Permit sending of broadcast messages
  SO_BROADCAST = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the threshold in
  # microseconds for low latency polling (Linux 3.11)
  SO_BUSY_POLL = ::T.let(nil, ::T.untyped)
  # Debug info recording
  SO_DEBUG = ::T.let(nil, ::T.untyped)
  # Detach an accept filter
  SO_DETACH_FILTER = ::T.let(nil, ::T.untyped)
  # Domain given for socket() (Linux 2.6.32)
  SO_DOMAIN = ::T.let(nil, ::T.untyped)
  # Use interface addresses
  SO_DONTROUTE = ::T.let(nil, ::T.untyped)
  # Get and clear the error status
  SO_ERROR = ::T.let(nil, ::T.untyped)
  # Obtain filter set by
  # [`SO_ATTACH_FILTER`](https://docs.ruby-lang.org/en/2.7.0/Socket/Constants.html#SO_ATTACH_FILTER)
  # (Linux 3.8)
  SO_GET_FILTER = ::T.let(nil, ::T.untyped)
  # Keep connections alive
  SO_KEEPALIVE = ::T.let(nil, ::T.untyped)
  # Linger on close if data is present
  SO_LINGER = ::T.let(nil, ::T.untyped)
  # Lock the filter attached to a socket (Linux 3.9)
  SO_LOCK_FILTER = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the mark for
  # mark-based routing (Linux 2.6.25)
  SO_MARK = ::T.let(nil, ::T.untyped)
  # Cap the rate computed by transport layer. [bytes per second] (Linux 3.13)
  SO_MAX_PACING_RATE = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) netns of a socket
  # (Linux 3.4)
  SO_NOFCS = ::T.let(nil, ::T.untyped)
  # Disable checksums
  SO_NO_CHECK = ::T.let(nil, ::T.untyped)
  # Leave received out-of-band data in-line
  SO_OOBINLINE = ::T.let(nil, ::T.untyped)
  # Receive
  # [`SCM_CREDENTIALS`](https://docs.ruby-lang.org/en/2.7.0/Socket/Constants.html#SCM_CREDENTIALS)
  # messages
  SO_PASSCRED = ::T.let(nil, ::T.untyped)
  # Toggle security context passing (Linux 2.6.18)
  SO_PASSSEC = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the peek offset (Linux
  # 3.4)
  SO_PEEK_OFF = ::T.let(nil, ::T.untyped)
  # The credentials of the foreign process connected to this socket
  SO_PEERCRED = ::T.let(nil, ::T.untyped)
  # Name of the connecting user
  SO_PEERNAME = ::T.let(nil, ::T.untyped)
  # Obtain the security credentials (Linux 2.6.2)
  SO_PEERSEC = ::T.let(nil, ::T.untyped)
  # The protocol-defined priority for all packets on this socket
  SO_PRIORITY = ::T.let(nil, ::T.untyped)
  # Protocol given for socket() (Linux 2.6.32)
  SO_PROTOCOL = ::T.let(nil, ::T.untyped)
  # Receive buffer size
  SO_RCVBUF = ::T.let(nil, ::T.untyped)
  # Receive buffer size without rmem\_max limit (Linux 2.6.14)
  SO_RCVBUFFORCE = ::T.let(nil, ::T.untyped)
  # Receive low-water mark
  SO_RCVLOWAT = ::T.let(nil, ::T.untyped)
  # Receive timeout
  SO_RCVTIMEO = ::T.let(nil, ::T.untyped)
  # Allow local address reuse
  SO_REUSEADDR = ::T.let(nil, ::T.untyped)
  # Allow local address and port reuse
  SO_REUSEPORT = ::T.let(nil, ::T.untyped)
  # Toggle cmsg for number of packets dropped (Linux 2.6.33)
  SO_RXQ_OVFL = ::T.let(nil, ::T.untyped)
  SO_SECURITY_AUTHENTICATION = ::T.let(nil, ::T.untyped)
  SO_SECURITY_ENCRYPTION_NETWORK = ::T.let(nil, ::T.untyped)
  SO_SECURITY_ENCRYPTION_TRANSPORT = ::T.let(nil, ::T.untyped)
  # Make select() detect socket error queue with errorfds (Linux 3.10)
  SO_SELECT_ERR_QUEUE = ::T.let(nil, ::T.untyped)
  # Send buffer size
  SO_SNDBUF = ::T.let(nil, ::T.untyped)
  # Send buffer size without wmem\_max limit (Linux 2.6.14)
  SO_SNDBUFFORCE = ::T.let(nil, ::T.untyped)
  # Send low-water mark
  SO_SNDLOWAT = ::T.let(nil, ::T.untyped)
  # Send timeout
  SO_SNDTIMEO = ::T.let(nil, ::T.untyped)
  # Receive timestamp with datagrams (timeval)
  SO_TIMESTAMP = ::T.let(nil, ::T.untyped)
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) stamping of incoming
  # and outgoing packets (Linux 2.6.30)
  SO_TIMESTAMPING = ::T.let(nil, ::T.untyped)
  # Receive nanosecond timestamp with datagrams (timespec)
  SO_TIMESTAMPNS = ::T.let(nil, ::T.untyped)
  # Get the socket type
  SO_TYPE = ::T.let(nil, ::T.untyped)
  # Toggle cmsg for wifi status (Linux 3.3)
  SO_WIFI_STATUS = ::T.let(nil, ::T.untyped)
  # TCP congestion control algorithm (Linux 2.6.13, glibc 2.6)
  TCP_CONGESTION = ::T.let(nil, ::T.untyped)
  # TCP Cookie Transactions (Linux 2.6.33, glibc 2.18)
  TCP_COOKIE_TRANSACTIONS = ::T.let(nil, ::T.untyped)
  # Don't send partial frames (Linux 2.2, glibc 2.2)
  TCP_CORK = ::T.let(nil, ::T.untyped)
  # Don't notify a listening socket until data is ready (Linux 2.4, glibc 2.2)
  TCP_DEFER_ACCEPT = ::T.let(nil, ::T.untyped)
  # Reduce step of the handshake process (Linux 3.7, glibc 2.18)
  TCP_FASTOPEN = ::T.let(nil, ::T.untyped)
  # Retrieve information about this socket (Linux 2.4, glibc 2.2)
  TCP_INFO = ::T.let(nil, ::T.untyped)
  # Maximum number of keepalive probes allowed before dropping a connection
  # (Linux 2.4, glibc 2.2)
  TCP_KEEPCNT = ::T.let(nil, ::T.untyped)
  # Idle time before keepalive probes are sent (Linux 2.4, glibc 2.2)
  TCP_KEEPIDLE = ::T.let(nil, ::T.untyped)
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) between keepalive
  # probes (Linux 2.4, glibc 2.2)
  TCP_KEEPINTVL = ::T.let(nil, ::T.untyped)
  # Lifetime of orphaned FIN\_WAIT2 sockets (Linux 2.4, glibc 2.2)
  TCP_LINGER2 = ::T.let(nil, ::T.untyped)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) maximum segment size
  TCP_MAXSEG = ::T.let(nil, ::T.untyped)
  # Use MD5 digests (RFC2385, Linux 2.6.20, glibc 2.7)
  TCP_MD5SIG = ::T.let(nil, ::T.untyped)
  # Don't delay sending to coalesce packets
  TCP_NODELAY = ::T.let(nil, ::T.untyped)
  # Sequence of a queue for repair mode (Linux 3.5, glibc 2.18)
  TCP_QUEUE_SEQ = ::T.let(nil, ::T.untyped)
  # Enable quickack mode (Linux 2.4.4, glibc 2.3)
  TCP_QUICKACK = ::T.let(nil, ::T.untyped)
  # Repair mode (Linux 3.5, glibc 2.18)
  TCP_REPAIR = ::T.let(nil, ::T.untyped)
  # Options for repair mode (Linux 3.5, glibc 2.18)
  TCP_REPAIR_OPTIONS = ::T.let(nil, ::T.untyped)
  # [`Queue`](https://docs.ruby-lang.org/en/2.7.0/Queue.html) for repair mode
  # (Linux 3.5, glibc 2.18)
  TCP_REPAIR_QUEUE = ::T.let(nil, ::T.untyped)
  # Number of SYN retransmits before a connection is dropped (Linux 2.4, glibc
  # 2.2)
  TCP_SYNCNT = ::T.let(nil, ::T.untyped)
  # Duplicated acknowledgments handling for thin-streams (Linux 2.6.34, glibc
  # 2.18)
  TCP_THIN_DUPACK = ::T.let(nil, ::T.untyped)
  # Linear timeouts for thin-streams (Linux 2.6.34, glibc 2.18)
  TCP_THIN_LINEAR_TIMEOUTS = ::T.let(nil, ::T.untyped)
  # TCP timestamp (Linux 3.9, glibc 2.18)
  TCP_TIMESTAMP = ::T.let(nil, ::T.untyped)
  # Max timeout before a TCP connection is aborted (Linux 2.6.37, glibc 2.18)
  TCP_USER_TIMEOUT = ::T.let(nil, ::T.untyped)
  # Clamp the size of the advertised window (Linux 2.4, glibc 2.2)
  TCP_WINDOW_CLAMP = ::T.let(nil, ::T.untyped)
  # Don't send partial frames (Linux 2.5.44, glibc 2.11)
  UDP_CORK = ::T.let(nil, ::T.untyped)

end

# [`Socket::Ifaddr`](https://docs.ruby-lang.org/en/2.7.0/Socket/Ifaddr.html)
# represents a result of getifaddrs() function.
class Socket::Ifaddr < Data
  # Returns the address of *ifaddr*. nil is returned if address is not available
  # in *ifaddr*.
  sig {returns(::T.untyped)}
  def addr(); end

  # Returns the broadcast address of *ifaddr*. nil is returned if the flags
  # doesn't have IFF\_BROADCAST.
  sig {returns(::T.untyped)}
  def broadaddr(); end

  # Returns the destination address of *ifaddr*. nil is returned if the flags
  # doesn't have IFF\_POINTOPOINT.
  sig {returns(::T.untyped)}
  def dstaddr(); end

  # Returns the flags of *ifaddr*.
  sig {returns(::T.untyped)}
  def flags(); end

  # Returns the interface index of *ifaddr*.
  sig {returns(::T.untyped)}
  def ifindex(); end

  # Returns a string to show contents of *ifaddr*.
  sig {returns(::T.untyped)}
  def inspect(); end

  # Returns the interface name of *ifaddr*.
  sig {returns(::T.untyped)}
  def name(); end

  # Returns the netmask address of *ifaddr*. nil is returned if netmask is not
  # available in *ifaddr*.
  sig {returns(::T.untyped)}
  def netmask(); end
end

# [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
# represents a socket option used by
# [`BasicSocket#getsockopt`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-getsockopt)
# and
# [`BasicSocket#setsockopt`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-setsockopt).
# A socket option contains the socket
# [`family`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html#method-i-family),
# protocol
# [`level`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html#method-i-level),
# option name
# [`optname`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html#method-i-optname)
# and option value
# [`data`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html#method-i-data).
class Socket::Option
  # Returns the data in *sockopt* as an boolean value.
  #
  # ```ruby
  # sockopt = Socket::Option.int(:INET, :SOCKET, :KEEPALIVE, 1)
  # p sockopt.bool => true
  # ```
  sig {returns(::T.untyped)}
  def bool(); end

  # Returns the data in *sockopt* as an byte.
  #
  # ```ruby
  # sockopt = Socket::Option.byte(:INET, :SOCKET, :KEEPALIVE, 1)
  # p sockopt.byte => 1
  # ```
  sig {returns(::T.untyped)}
  def byte(); end

  # returns the socket option data as a string.
  #
  # ```ruby
  # p Socket::Option.new(:INET6, :IPV6, :RECVPKTINFO, [1].pack("i!")).data
  # #=> "\x01\x00\x00\x00"
  # ```
  sig {returns(::T.untyped)}
  def data(); end

  # returns the socket family as an integer.
  #
  # ```ruby
  # p Socket::Option.new(:INET6, :IPV6, :RECVPKTINFO, [1].pack("i!")).family
  # #=> 10
  # ```
  sig {returns(::T.untyped)}
  def family(); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
    )
    .void
  end
  def initialize(arg0, arg1, arg2, arg3); end

  # Returns a string which shows sockopt in human-readable form.
  #
  # ```ruby
  # p Socket::Option.new(:INET, :SOCKET, :KEEPALIVE, [1].pack("i")).inspect
  # #=> "#<Socket::Option: INET SOCKET KEEPALIVE 1>"
  # ```
  sig {returns(::T.untyped)}
  def inspect(); end

  # Returns the data in *sockopt* as an int.
  #
  # The size and endian is dependent on the platform.
  #
  # ```ruby
  # sockopt = Socket::Option.int(:INET, :SOCKET, :KEEPALIVE, 1)
  # p sockopt.int => 1
  # ```
  sig {returns(::T.untyped)}
  def int(); end

  # Returns the
  # [`ipv4_multicast_loop`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html#method-c-ipv4_multicast_loop)
  # data in *sockopt* as an integer.
  #
  # ```ruby
  # sockopt = Socket::Option.ipv4_multicast_loop(10)
  # p sockopt.ipv4_multicast_loop => 10
  # ```
  sig {returns(::T.untyped)}
  def ipv4_multicast_loop(); end

  # Returns the
  # [`ipv4_multicast_ttl`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html#method-c-ipv4_multicast_ttl)
  # data in *sockopt* as an integer.
  #
  # ```ruby
  # sockopt = Socket::Option.ipv4_multicast_ttl(10)
  # p sockopt.ipv4_multicast_ttl => 10
  # ```
  sig {returns(::T.untyped)}
  def ipv4_multicast_ttl(); end

  # returns the socket level as an integer.
  #
  # ```ruby
  # p Socket::Option.new(:INET6, :IPV6, :RECVPKTINFO, [1].pack("i!")).level
  # #=> 41
  # ```
  sig {returns(::T.untyped)}
  def level(); end

  # Returns the linger data in *sockopt* as a pair of boolean and integer.
  #
  # ```ruby
  # sockopt = Socket::Option.linger(true, 10)
  # p sockopt.linger => [true, 10]
  # ```
  sig {returns(::T.untyped)}
  def linger(); end

  # returns the socket option name as an integer.
  #
  # ```ruby
  # p Socket::Option.new(:INET6, :IPV6, :RECVPKTINFO, [1].pack("i!")).optname
  # #=> 2
  # ```
  sig {returns(::T.untyped)}
  def optname(); end

  # returns the socket option data as a string.
  #
  # ```ruby
  # p Socket::Option.new(:INET6, :IPV6, :RECVPKTINFO, [1].pack("i!")).data
  # #=> "\x01\x00\x00\x00"
  # ```
  sig {returns(::T.untyped)}
  def to_s(); end

  # Calls
  # [`String#unpack`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-unpack)
  # on sockopt.data.
  #
  # ```ruby
  # sockopt = Socket::Option.new(:INET, :SOCKET, :KEEPALIVE, [1].pack("i"))
  # p sockopt.unpack("i")      #=> [1]
  # p sockopt.data.unpack("i") #=> [1]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unpack(arg0); end

  # Creates a new
  # [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
  # object which contains boolean as data. Actually 0 or 1 as int is used.
  #
  # ```ruby
  # require 'socket'
  #
  # p Socket::Option.bool(:INET, :SOCKET, :KEEPALIVE, true)
  # #=> #<Socket::Option: INET SOCKET KEEPALIVE 1>
  #
  # p Socket::Option.bool(:INET, :SOCKET, :KEEPALIVE, false)
  # #=> #<Socket::Option: AF_INET SOCKET KEEPALIVE 0>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.bool(arg0, arg1, arg2, arg3); end

  # Creates a new
  # [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
  # object which contains a byte as data.
  #
  # ```ruby
  # p Socket::Option.byte(:INET, :SOCKET, :KEEPALIVE, 1)
  # #=> #<Socket::Option: INET SOCKET KEEPALIVE 1>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.byte(arg0, arg1, arg2, arg3); end

  # Creates a new
  # [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
  # object which contains an int as data.
  #
  # The size and endian is dependent on the platform.
  #
  # ```ruby
  # p Socket::Option.int(:INET, :SOCKET, :KEEPALIVE, 1)
  # #=> #<Socket::Option: INET SOCKET KEEPALIVE 1>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.int(arg0, arg1, arg2, arg3); end

  # Creates a new
  # [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
  # object for IP\_MULTICAST\_LOOP.
  #
  # The size is dependent on the platform.
  #
  # ```ruby
  # sockopt = Socket::Option.int(:INET, :IPPROTO_IP, :IP_MULTICAST_LOOP, 1)
  # p sockopt.int => 1
  #
  # p Socket::Option.ipv4_multicast_loop(10)
  # #=> #<Socket::Option: INET IP MULTICAST_LOOP 10>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ipv4_multicast_loop(arg0); end

  # Creates a new
  # [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
  # object for IP\_MULTICAST\_TTL.
  #
  # The size is dependent on the platform.
  #
  # ```ruby
  # p Socket::Option.ipv4_multicast_ttl(10)
  # #=> #<Socket::Option: INET IP MULTICAST_TTL 10>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ipv4_multicast_ttl(arg0); end

  # Creates a new
  # [`Socket::Option`](https://docs.ruby-lang.org/en/2.7.0/Socket/Option.html)
  # object for SOL\_SOCKET/SO\_LINGER.
  #
  # *onoff* should be an integer or a boolean.
  #
  # *secs* should be the number of seconds.
  #
  # ```ruby
  # p Socket::Option.linger(true, 10)
  # #=> #<Socket::Option: UNSPEC SOCKET LINGER on 10sec>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.linger(arg0, arg1); end
end

# [`SocketError`](https://docs.ruby-lang.org/en/2.7.0/SocketError.html) is the
# error class for socket.
class SocketError < StandardError
end


# UDP/IP address information used by
# [`Socket.udp_server_loop`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-c-udp_server_loop).
class Socket::UDPSource
  sig do
    params(
      remote_address: ::T.untyped,
      local_address: ::T.untyped,
      reply_proc: ::T.untyped,
    )
    .void
  end
  def initialize(remote_address, local_address, &reply_proc); end

  sig {returns(::T.untyped)}
  def inspect(); end

  # Local address
  sig {returns(::T.untyped)}
  def local_address(); end

  # Address of the source
  sig {returns(::T.untyped)}
  def remote_address(); end

  # Sends the [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) `msg`
  # to the source
  sig do
    params(
      msg: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def reply(msg); end
end

# [`TCPServer`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html) represents a
# TCP/IP server socket.
#
# A simple TCP server may look like:
#
# ```ruby
# require 'socket'
#
# server = TCPServer.new 2000 # Server bind to port 2000
# loop do
#   client = server.accept    # Wait for a client to connect
#   client.puts "Hello !"
#   client.puts "Time is #{Time.now}"
#   client.close
# end
# ```
#
# A more usable server (serving multiple clients):
#
# ```ruby
# require 'socket'
#
# server = TCPServer.new 2000
# loop do
#   Thread.start(server.accept) do |client|
#     client.puts "Hello !"
#     client.puts "Time is #{Time.now}"
#     client.close
#   end
# end
# ```
class TCPServer < TCPSocket
  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  # Accepts an incoming connection. It returns a new
  # [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html) object.
  #
  # ```ruby
  # TCPServer.open("127.0.0.1", 14641) {|serv|
  #   s = serv.accept
  #   s.puts Time.now
  #   s.close
  # }
  # ```
  sig {returns(::T.untyped)}
  def accept(); end

  # Accepts an incoming connection using accept(2) after O\_NONBLOCK is set for
  # the underlying file descriptor. It returns an accepted
  # [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html) for the
  # incoming connection.
  #
  # ### Example
  #
  # ```ruby
  # require 'socket'
  # serv = TCPServer.new(2202)
  # begin # emulate blocking accept
  #   sock = serv.accept_nonblock
  # rescue IO::WaitReadable, Errno::EINTR
  #   IO.select([serv])
  #   retry
  # end
  # # sock is an accepted socket.
  # ```
  #
  # Refer to
  # [`Socket#accept`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept)
  # for the exceptions that may be thrown if the call to
  # [`TCPServer#accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html#method-i-accept_nonblock)
  # fails.
  #
  # [`TCPServer#accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html#method-i-accept_nonblock)
  # may raise any error corresponding to accept(2) failure, including
  # Errno::EWOULDBLOCK.
  #
  # If the exception is Errno::EWOULDBLOCK, Errno::EAGAIN,
  # [`Errno::ECONNABORTED`](https://docs.ruby-lang.org/en/2.7.0/Errno/ECONNABORTED.html),
  # [`Errno::EPROTO`](https://docs.ruby-lang.org/en/2.7.0/Errno/EPROTO.html), it
  # is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html).
  # So
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # can be used to rescue the exceptions for retrying accept\_nonblock.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html#method-i-accept_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead.
  #
  # ### See
  # *   [`TCPServer#accept`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html#method-i-accept)
  # *   [`Socket#accept`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept)
  sig do
    params(
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def accept_nonblock(exception: T.unsafe(nil)); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Listens for connections, using the specified `int` as the backlog. A call to
  # *listen* only applies if the `socket` is of type SOCK\_STREAM or
  # SOCK\_SEQPACKET.
  #
  # ### Parameter
  # *   `backlog` - the maximum length of the queue for pending connections.
  #
  #
  # ### Example 1
  #
  # ```ruby
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 2200, 'localhost' )
  # socket.bind( sockaddr )
  # socket.listen( 5 )
  # ```
  #
  # ### Example 2 (listening on an arbitrary port, unix-based systems only):
  #
  # ```ruby
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # socket.listen( 1 )
  # ```
  #
  # ### Unix-based Exceptions
  # On unix based systems the above will work because a new `sockaddr` struct is
  # created on the address ADDR\_ANY, for an arbitrary port number as handed off
  # by the kernel. It will not work on Windows, because Windows requires that
  # the `socket` is bound by calling *bind* before it can *listen*.
  #
  # If the *backlog* amount exceeds the implementation-dependent maximum queue
  # length, the implementation's maximum queue length will be used.
  #
  # On unix-based based systems the following system exceptions may be raised if
  # the call to *listen* fails:
  # *   Errno::EBADF - the *socket* argument is not a valid file descriptor
  # *   Errno::EDESTADDRREQ - the *socket* is not bound to a local address, and
  #     the protocol does not support listening on an unbound socket
  # *   Errno::EINVAL - the *socket* is already connected
  # *   Errno::ENOTSOCK - the *socket* argument does not refer to a socket
  # *   Errno::EOPNOTSUPP - the *socket* protocol does not support listen
  # *   Errno::EACCES - the calling process does not have appropriate privileges
  # *   Errno::EINVAL - the *socket* has been shut down
  # *   Errno::ENOBUFS - insufficient resources are available in the system to
  #     complete the call
  #
  #
  # ### Windows Exceptions
  # On Windows systems the following system exceptions may be raised if the call
  # to *listen* fails:
  # *   Errno::ENETDOWN - the network is down
  # *   Errno::EADDRINUSE - the socket's local address is already in use. This
  #     usually occurs during the execution of *bind* but could be delayed if
  #     the call to *bind* was to a partially wildcard address (involving
  #     ADDR\_ANY) and if a specific address needs to be committed at the time
  #     of the call to *listen*
  # *   Errno::EINPROGRESS - a Windows Sockets 1.1 call is in progress or the
  #     service provider is still processing a callback function
  # *   Errno::EINVAL - the `socket` has not been bound with a call to *bind*.
  # *   Errno::EISCONN - the `socket` is already connected
  # *   Errno::EMFILE - no more socket descriptors are available
  # *   Errno::ENOBUFS - no buffer space is available
  # *   Errno::ENOTSOC - `socket` is not a socket
  # *   Errno::EOPNOTSUPP - the referenced `socket` is not a type that supports
  #     the *listen* method
  #
  #
  # ### See
  # *   listen manual pages on unix-based systems
  # *   listen function in Microsoft's Winsock functions reference
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(arg0); end

  # Returns a file descriptor of a accepted connection.
  #
  # ```ruby
  # TCPServer.open("127.0.0.1", 28561) {|serv|
  #   fd = serv.sysaccept
  #   s = IO.for_fd(fd)
  #   s.puts Time.now
  #   s.close
  # }
  # ```
  sig {returns(::T.untyped)}
  def sysaccept(); end
end

# [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html) represents a
# TCP/IP client socket.
#
# A simple client may look like:
#
# ```ruby
# require 'socket'
#
# s = TCPSocket.new 'localhost', 2000
#
# while line = s.gets # Read lines from socket
#   puts line         # and print them
# end
#
# s.close             # close socket when done
# ```
class TCPSocket < IPSocket
  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
      local_host: ::T.untyped,
      local_port: ::T.untyped,
    )
    .void
  end
  def initialize(host=T.unsafe(nil), port=T.unsafe(nil), local_host=T.unsafe(nil), local_port=T.unsafe(nil)); end

  sig do
    params(
      host: String,
      port: Integer,
      local_host: T.nilable(String),
      local_port: T.nilable(Integer),
    )
    .returns(::T.untyped)
  end
  def self.open(host, port, local_host=nil, local_port=nil); end

  sig {returns(::T.untyped)}
  def socks_authenticate(); end

  sig do
    params(
      host: ::T.untyped,
      port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def socks_connect(host, port); end

  sig {returns(::T.untyped)}
  def socks_receive_reply(); end

  # Use
  # [`Addrinfo.getaddrinfo`](https://docs.ruby-lang.org/en/2.7.0/Addrinfo.html#method-c-getaddrinfo)
  # instead. This method is deprecated for the following reasons:
  #
  # *   The 3rd element of the result is the address family of the first
  #     address. The address families of the rest of the addresses are not
  #     returned.
  # *   gethostbyname() may take a long time and it may block other threads.
  #     (GVL cannot be released since gethostbyname() is not thread safe.)
  # *   This method uses gethostbyname() function already removed from POSIX.
  #
  #
  # This method lookups host information by *hostname*.
  #
  # ```ruby
  # TCPSocket.gethostbyname("localhost")
  # #=> ["localhost", ["hal"], 2, "127.0.0.1"]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.gethostbyname(arg0); end

  sig {returns(::T.untyped)}
  def self.socks_ignores(); end

  sig do
    params(
      ignores: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_ignores=(ignores); end

  sig {returns(::T.untyped)}
  def self.socks_password(); end

  sig do
    params(
      password: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_password=(password); end

  sig {returns(::T.untyped)}
  def self.socks_port(); end

  sig do
    params(
      port: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_port=(port); end

  sig {returns(::T.untyped)}
  def self.socks_server(); end

  sig do
    params(
      host: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_server=(host); end

  sig {returns(::T.untyped)}
  def self.socks_username(); end

  sig do
    params(
      username: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_username=(username); end

  sig {returns(::T.untyped)}
  def self.socks_version(); end

  sig do
    params(
      version: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socks_version=(version); end
end

# [`UDPSocket`](https://docs.ruby-lang.org/en/2.7.0/UDPSocket.html) represents a
# UDP/IP socket.
class UDPSocket < IPSocket
  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  # Binds *udpsocket* to *host*:*port*.
  #
  # ```ruby
  # u1 = UDPSocket.new
  # u1.bind("127.0.0.1", 4913)
  # u1.send "message-to-self", 0, "127.0.0.1", 4913
  # p u1.recvfrom(10) #=> ["message-to", ["AF_INET", 4913, "localhost", "127.0.0.1"]]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def bind(arg0, arg1); end

  # Connects *udpsocket* to *host*:*port*.
  #
  # This makes possible to send without destination address.
  #
  # ```ruby
  # u1 = UDPSocket.new
  # u1.bind("127.0.0.1", 4913)
  # u2 = UDPSocket.new
  # u2.connect("127.0.0.1", 4913)
  # u2.send "uuuu", 0
  # p u1.recvfrom(10) #=> ["uuuu", ["AF_INET", 33230, "localhost", "127.0.0.1"]]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect(arg0, arg1); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Receives up to *maxlen* bytes from `udpsocket` using recvfrom(2) after
  # O\_NONBLOCK is set for the underlying file descriptor. *flags* is zero or
  # more of the `MSG_` options. The first element of the results, *mesg*, is the
  # data received. The second element, *sender\_inet\_addr*, is an array to
  # represent the sender address.
  #
  # When recvfrom(2) returns 0,
  # [`Socket#recvfrom_nonblock`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom_nonblock)
  # returns an empty string as data. It means an empty packet.
  #
  # ### Parameters
  # *   `maxlen` - the number of bytes to receive from the socket
  # *   `flags` - zero or more of the `MSG_` options
  # *   `outbuf` - destination
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) buffer
  # *   `options` - keyword hash, supporting `exception: false`
  #
  #
  # ### Example
  #
  # ```ruby
  # require 'socket'
  # s1 = UDPSocket.new
  # s1.bind("127.0.0.1", 0)
  # s2 = UDPSocket.new
  # s2.bind("127.0.0.1", 0)
  # s2.connect(*s1.addr.values_at(3,1))
  # s1.connect(*s2.addr.values_at(3,1))
  # s1.send "aaa", 0
  # begin # emulate blocking recvfrom
  #   p s2.recvfrom_nonblock(10)  #=> ["aaa", ["AF_INET", 33302, "localhost.localdomain", "127.0.0.1"]]
  # rescue IO::WaitReadable
  #   IO.select([s2])
  #   retry
  # end
  # ```
  #
  # Refer to
  # [`Socket#recvfrom`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom)
  # for the exceptions that may be thrown if the call to *recvfrom\_nonblock*
  # fails.
  #
  # [`UDPSocket#recvfrom_nonblock`](https://docs.ruby-lang.org/en/2.7.0/UDPSocket.html#method-i-recvfrom_nonblock)
  # may raise any error corresponding to recvfrom(2) failure, including
  # Errno::EWOULDBLOCK.
  #
  # If the exception is Errno::EWOULDBLOCK or Errno::EAGAIN, it is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html).
  # So
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # can be used to rescue the exceptions for retrying recvfrom\_nonblock.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`recvfrom_nonblock`](https://docs.ruby-lang.org/en/2.7.0/UDPSocket.html#method-i-recvfrom_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead.
  #
  # ### See
  # *   [`Socket#recvfrom`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-recvfrom)
  sig do
    params(
      len: ::T.untyped,
      flag: ::T.untyped,
      outbuf: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom_nonblock(len, flag=T.unsafe(nil), outbuf=T.unsafe(nil), exception: T.unsafe(nil)); end

  # Sends *mesg* via *udpsocket*.
  #
  # *flags* should be a bitwise OR of Socket::MSG\_\* constants.
  #
  # ```ruby
  # u1 = UDPSocket.new
  # u1.bind("127.0.0.1", 4913)
  #
  # u2 = UDPSocket.new
  # u2.send "hi", 0, "127.0.0.1", 4913
  #
  # mesg, addr = u1.recvfrom(10)
  # u1.send mesg, 0, addr[3], addr[1]
  #
  # p u2.recv(100) #=> "hi"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def send(*arg0); end
end

# [`UNIXServer`](https://docs.ruby-lang.org/en/2.7.0/UNIXServer.html) represents
# a UNIX domain stream server socket.
class UNIXServer < UNIXSocket
  Elem = type_member(:out) {{fixed: String}}

  # Accepts an incoming connection. It returns a new
  # [`UNIXSocket`](https://docs.ruby-lang.org/en/2.7.0/UNIXSocket.html) object.
  #
  # ```ruby
  # UNIXServer.open("/tmp/sock") {|serv|
  #   UNIXSocket.open("/tmp/sock") {|c|
  #     s = serv.accept
  #     s.puts "hi"
  #     s.close
  #     p c.read #=> "hi\n"
  #   }
  # }
  # ```
  sig {returns(::T.untyped)}
  def accept(); end

  # Accepts an incoming connection using accept(2) after O\_NONBLOCK is set for
  # the underlying file descriptor. It returns an accepted
  # [`UNIXSocket`](https://docs.ruby-lang.org/en/2.7.0/UNIXSocket.html) for the
  # incoming connection.
  #
  # ### Example
  #
  # ```ruby
  # require 'socket'
  # serv = UNIXServer.new("/tmp/sock")
  # begin # emulate blocking accept
  #   sock = serv.accept_nonblock
  # rescue IO::WaitReadable, Errno::EINTR
  #   IO.select([serv])
  #   retry
  # end
  # # sock is an accepted socket.
  # ```
  #
  # Refer to
  # [`Socket#accept`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept)
  # for the exceptions that may be thrown if the call to
  # [`UNIXServer#accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/UNIXServer.html#method-i-accept_nonblock)
  # fails.
  #
  # [`UNIXServer#accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/UNIXServer.html#method-i-accept_nonblock)
  # may raise any error corresponding to accept(2) failure, including
  # Errno::EWOULDBLOCK.
  #
  # If the exception is Errno::EWOULDBLOCK, Errno::EAGAIN,
  # [`Errno::ECONNABORTED`](https://docs.ruby-lang.org/en/2.7.0/Errno/ECONNABORTED.html)
  # or [`Errno::EPROTO`](https://docs.ruby-lang.org/en/2.7.0/Errno/EPROTO.html),
  # it is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html).
  # So
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # can be used to rescue the exceptions for retrying accept\_nonblock.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/UNIXServer.html#method-i-accept_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead.
  #
  # ### See
  # *   [`UNIXServer#accept`](https://docs.ruby-lang.org/en/2.7.0/UNIXServer.html#method-i-accept)
  # *   [`Socket#accept`](https://docs.ruby-lang.org/en/2.7.0/Socket.html#method-i-accept)
  sig do
    params(
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def accept_nonblock(exception: T.unsafe(nil)); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(arg0); end

  # Listens for connections, using the specified `int` as the backlog. A call to
  # *listen* only applies if the `socket` is of type SOCK\_STREAM or
  # SOCK\_SEQPACKET.
  #
  # ### Parameter
  # *   `backlog` - the maximum length of the queue for pending connections.
  #
  #
  # ### Example 1
  #
  # ```ruby
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # sockaddr = Socket.pack_sockaddr_in( 2200, 'localhost' )
  # socket.bind( sockaddr )
  # socket.listen( 5 )
  # ```
  #
  # ### Example 2 (listening on an arbitrary port, unix-based systems only):
  #
  # ```ruby
  # require 'socket'
  # include Socket::Constants
  # socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  # socket.listen( 1 )
  # ```
  #
  # ### Unix-based Exceptions
  # On unix based systems the above will work because a new `sockaddr` struct is
  # created on the address ADDR\_ANY, for an arbitrary port number as handed off
  # by the kernel. It will not work on Windows, because Windows requires that
  # the `socket` is bound by calling *bind* before it can *listen*.
  #
  # If the *backlog* amount exceeds the implementation-dependent maximum queue
  # length, the implementation's maximum queue length will be used.
  #
  # On unix-based based systems the following system exceptions may be raised if
  # the call to *listen* fails:
  # *   Errno::EBADF - the *socket* argument is not a valid file descriptor
  # *   Errno::EDESTADDRREQ - the *socket* is not bound to a local address, and
  #     the protocol does not support listening on an unbound socket
  # *   Errno::EINVAL - the *socket* is already connected
  # *   Errno::ENOTSOCK - the *socket* argument does not refer to a socket
  # *   Errno::EOPNOTSUPP - the *socket* protocol does not support listen
  # *   Errno::EACCES - the calling process does not have appropriate privileges
  # *   Errno::EINVAL - the *socket* has been shut down
  # *   Errno::ENOBUFS - insufficient resources are available in the system to
  #     complete the call
  #
  #
  # ### Windows Exceptions
  # On Windows systems the following system exceptions may be raised if the call
  # to *listen* fails:
  # *   Errno::ENETDOWN - the network is down
  # *   Errno::EADDRINUSE - the socket's local address is already in use. This
  #     usually occurs during the execution of *bind* but could be delayed if
  #     the call to *bind* was to a partially wildcard address (involving
  #     ADDR\_ANY) and if a specific address needs to be committed at the time
  #     of the call to *listen*
  # *   Errno::EINPROGRESS - a Windows Sockets 1.1 call is in progress or the
  #     service provider is still processing a callback function
  # *   Errno::EINVAL - the `socket` has not been bound with a call to *bind*.
  # *   Errno::EISCONN - the `socket` is already connected
  # *   Errno::EMFILE - no more socket descriptors are available
  # *   Errno::ENOBUFS - no buffer space is available
  # *   Errno::ENOTSOC - `socket` is not a socket
  # *   Errno::EOPNOTSUPP - the referenced `socket` is not a type that supports
  #     the *listen* method
  #
  #
  # ### See
  # *   listen manual pages on unix-based systems
  # *   listen function in Microsoft's Winsock functions reference
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(arg0); end

  # Accepts a new connection. It returns the new file descriptor which is an
  # integer.
  #
  # ```ruby
  # UNIXServer.open("/tmp/sock") {|serv|
  #   UNIXSocket.open("/tmp/sock") {|c|
  #     fd = serv.sysaccept
  #     s = IO.new(fd)
  #     s.puts "hi"
  #     s.close
  #     p c.read #=> "hi\n"
  #   }
  # }
  # ```
  sig {returns(::T.untyped)}
  def sysaccept(); end
end

# [`UNIXSocket`](https://docs.ruby-lang.org/en/2.7.0/UNIXSocket.html) represents
# a UNIX domain stream client socket.
class UNIXSocket < BasicSocket
  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  # Returns the local address as an array which contains address\_family and
  # unix\_path.
  #
  # Example
  #
  # ```ruby
  # serv = UNIXServer.new("/tmp/sock")
  # p serv.addr #=> ["AF_UNIX", "/tmp/sock"]
  # ```
  sig {returns(::T.untyped)}
  def addr(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(arg0); end

  # Returns the path of the local address of unixsocket.
  #
  # ```ruby
  # s = UNIXServer.new("/tmp/sock")
  # p s.path #=> "/tmp/sock"
  # ```
  sig {returns(::T.untyped)}
  def path(); end

  # Returns the remote address as an array which contains address\_family and
  # unix\_path.
  #
  # Example
  #
  # ```ruby
  # serv = UNIXServer.new("/tmp/sock")
  # c = UNIXSocket.new("/tmp/sock")
  # p c.peeraddr #=> ["AF_UNIX", "/tmp/sock"]
  # ```
  sig {returns(::T.untyped)}
  def peeraddr(); end

  # Example
  #
  # ```ruby
  # UNIXServer.open("/tmp/sock") {|serv|
  #   UNIXSocket.open("/tmp/sock") {|c|
  #     s = serv.accept
  #
  #     c.send_io STDOUT
  #     stdout = s.recv_io
  #
  #     p STDOUT.fileno #=> 1
  #     p stdout.fileno #=> 7
  #
  #     stdout.puts "hello" # outputs "hello\n" to standard output.
  #   }
  # }
  # ```
  #
  # *klass* will determine the class of *io* returned (using the
  # [`IO.for_fd`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-for_fd)
  # singleton method or similar). If *klass* is `nil`, an integer file
  # descriptor is returned.
  #
  # *mode* is the same as the argument passed to
  # [`IO.for_fd`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-for_fd)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recv_io(*arg0); end

  # Receives a message via *unixsocket*.
  #
  # *maxlen* is the maximum number of bytes to receive.
  #
  # *flags* should be a bitwise OR of Socket::MSG\_\* constants.
  #
  # *outbuf* will contain only the received data after the method call even if
  # it is not empty at the beginning.
  #
  # ```ruby
  # s1 = Socket.new(:UNIX, :DGRAM, 0)
  # s1_ai = Addrinfo.unix("/tmp/sock1")
  # s1.bind(s1_ai)
  #
  # s2 = Socket.new(:UNIX, :DGRAM, 0)
  # s2_ai = Addrinfo.unix("/tmp/sock2")
  # s2.bind(s2_ai)
  # s3 = UNIXSocket.for_fd(s2.fileno)
  #
  # s1.send "a", 0, s2_ai
  # p s3.recvfrom(10) #=> ["a", ["AF_UNIX", "/tmp/sock1"]]
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def recvfrom(*arg0); end

  # Sends *io* as file descriptor passing.
  #
  # ```ruby
  # s1, s2 = UNIXSocket.pair
  #
  # s1.send_io STDOUT
  # stdout = s2.recv_io
  #
  # p STDOUT.fileno #=> 1
  # p stdout.fileno #=> 6
  #
  # stdout.puts "hello" # outputs "hello\n" to standard output.
  # ```
  #
  # *io* may be any kind of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object or integer file descriptor.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def send_io(arg0); end

  # Creates a pair of sockets connected to each other.
  #
  # *socktype* should be a socket type such as: :STREAM, :DGRAM, :RAW, etc.
  #
  # *protocol* should be a protocol defined in the domain. 0 is default protocol
  # for the domain.
  #
  # ```ruby
  # s1, s2 = UNIXSocket.pair
  # s1.send "a", 0
  # s1.send "b", 0
  # p s2.recv(10) #=> "ab"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pair(*arg0); end

  # Creates a pair of sockets connected to each other.
  #
  # *socktype* should be a socket type such as: :STREAM, :DGRAM, :RAW, etc.
  #
  # *protocol* should be a protocol defined in the domain. 0 is default protocol
  # for the domain.
  #
  # ```ruby
  # s1, s2 = UNIXSocket.pair
  # s1.send "a", 0
  # s1.send "b", 0
  # p s2.recv(10) #=> "ab"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.socketpair(*arg0); end
end
