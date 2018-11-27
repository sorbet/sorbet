# typed: true

class Resolv
  AddressRegex = T.let(T.unsafe(nil), Regexp)

  class ResolvTimeout < ::Timeout::Error

  end

  class IPv4
    Regex = T.let(T.unsafe(nil), Regexp)

    Regex256 = T.let(T.unsafe(nil), Regexp)

    def to_name()
    end

    def eql?(other)
    end

    def address()
    end

    def ==(other)
    end

    def inspect()
    end

    def to_s()
    end

    def hash()
    end
  end

  class DNS
    class Config
      InitialTimeout = T.let(T.unsafe(nil), Integer)

      class NXDomain < ::Resolv::ResolvError

      end

      class OtherResolvError < ::Resolv::ResolvError

      end

      def resolv(name)
      end

      def timeouts=(values)
      end

      def single?()
      end

      def generate_candidates(name)
      end

      def nameserver_port()
      end

      def lazy_initialize()
      end

      def generate_timeouts()
      end
    end

    class Message
      class MessageEncoder
        def put_string(d)
        end

        def put_string_list(ds)
        end

        def put_pack(template, *d)
        end

        def put_name(d)
        end

        def put_label(d)
        end

        def put_labels(d)
        end

        def put_length16()
        end

        def to_s()
        end

        def put_bytes(d)
        end
      end

      class MessageDecoder
        def get_label()
        end

        def inspect()
        end

        def get_unpack(template)
        end

        def get_question()
        end

        def get_rr()
        end

        def get_length16()
        end

        def get_bytes(len = _)
        end

        def get_string()
        end

        def get_string_list()
        end

        def get_name()
        end

        def get_labels()
        end
      end

      def qr()
      end

      def opcode()
      end

      def aa()
      end

      def rd=(_)
      end

      def rd()
      end

      def ==(other)
      end

      def rcode()
      end

      def answer()
      end

      def tc()
      end

      def ra()
      end

      def authority()
      end

      def additional()
      end

      def each_question()
      end

      def add_question(name, typeclass)
      end

      def each_answer()
      end

      def add_authority(name, ttl, data)
      end

      def each_authority()
      end

      def add_answer(name, ttl, data)
      end

      def each_additional()
      end

      def add_additional(name, ttl, data)
      end

      def encode()
      end

      def id()
      end

      def each_resource()
      end

      def id=(_)
      end

      def qr=(_)
      end

      def opcode=(_)
      end

      def aa=(_)
      end

      def tc=(_)
      end

      def ra=(_)
      end

      def rcode=(_)
      end

      def question()
      end
    end

    class Query
      def encode_rdata(msg)
      end
    end

    class Resource < ::Resolv::DNS::Query
      class MX < ::Resolv::DNS::Resource
        TypeValue = T.let(T.unsafe(nil), Integer)

        def preference()
        end

        def exchange()
        end

        def encode_rdata(msg)
        end
      end

      class TXT < ::Resolv::DNS::Resource
        TypeValue = T.let(T.unsafe(nil), Integer)

        def encode_rdata(msg)
        end

        def data()
        end

        def strings()
        end
      end

      ClassHash = T.let(T.unsafe(nil), Hash)

      module IN
        class MX < ::Resolv::DNS::Resource::MX
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class SRV < ::Resolv::DNS::Resource
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)

          def priority()
          end

          def encode_rdata(msg)
          end

          def port()
          end

          def weight()
          end

          def target()
          end
        end

        class TXT < ::Resolv::DNS::Resource::TXT
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class A < ::Resolv::DNS::Resource
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)

          def address()
          end

          def encode_rdata(msg)
          end
        end

        class LOC < ::Resolv::DNS::Resource::LOC
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class AAAA < ::Resolv::DNS::Resource
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)

          def address()
          end

          def encode_rdata(msg)
          end
        end

        class NS < ::Resolv::DNS::Resource::NS
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class SOA < ::Resolv::DNS::Resource::SOA
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class ANY < ::Resolv::DNS::Resource::ANY
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        ClassValue = T.let(T.unsafe(nil), Integer)

        class PTR < ::Resolv::DNS::Resource::PTR
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class CNAME < ::Resolv::DNS::Resource::CNAME
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class HINFO < ::Resolv::DNS::Resource::HINFO
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class MINFO < ::Resolv::DNS::Resource::MINFO
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)
        end

        class WKS < ::Resolv::DNS::Resource
          TypeValue = T.let(T.unsafe(nil), Integer)

          ClassValue = T.let(T.unsafe(nil), Integer)

          def bitmap()
          end

          def address()
          end

          def encode_rdata(msg)
          end

          def protocol()
          end
        end
      end

      class Generic < ::Resolv::DNS::Resource
        def data()
        end

        def encode_rdata(msg)
        end
      end

      class LOC < ::Resolv::DNS::Resource
        TypeValue = T.let(T.unsafe(nil), Integer)

        def altitude()
        end

        def longitude()
        end

        def encode_rdata(msg)
        end

        def ssize()
        end

        def hprecision()
        end

        def version()
        end

        def vprecision()
        end

        def latitude()
        end
      end

      class DomainName < ::Resolv::DNS::Resource
        def name()
        end

        def encode_rdata(msg)
        end
      end

      class NS < ::Resolv::DNS::Resource::DomainName
        TypeValue = T.let(T.unsafe(nil), Integer)
      end

      class SOA < ::Resolv::DNS::Resource
        TypeValue = T.let(T.unsafe(nil), Integer)

        def serial()
        end

        def rname()
        end

        def expire()
        end

        def refresh()
        end

        def retry()
        end

        def encode_rdata(msg)
        end

        def minimum()
        end

        def mname()
        end
      end

      class PTR < ::Resolv::DNS::Resource::DomainName
        TypeValue = T.let(T.unsafe(nil), Integer)
      end

      class CNAME < ::Resolv::DNS::Resource::DomainName
        TypeValue = T.let(T.unsafe(nil), Integer)
      end

      class HINFO < ::Resolv::DNS::Resource
        TypeValue = T.let(T.unsafe(nil), Integer)

        def encode_rdata(msg)
        end

        def cpu()
        end

        def os()
        end
      end

      class MINFO < ::Resolv::DNS::Resource
        TypeValue = T.let(T.unsafe(nil), Integer)

        def encode_rdata(msg)
        end

        def rmailbx()
        end

        def emailbx()
        end
      end

      class ANY < ::Resolv::DNS::Query
        TypeValue = T.let(T.unsafe(nil), Integer)
      end

      ClassInsensitiveTypes = T.let(T.unsafe(nil), Array)

      def ttl()
      end

      def encode_rdata(msg)
      end

      def ==(other)
      end

      def hash()
      end

      def eql?(other)
      end
    end

    class DecodeError < ::StandardError

    end

    module RCode
      FormErr = T.let(T.unsafe(nil), Integer)

      ServFail = T.let(T.unsafe(nil), Integer)

      NotImp = T.let(T.unsafe(nil), Integer)

      Refused = T.let(T.unsafe(nil), Integer)

      YXDomain = T.let(T.unsafe(nil), Integer)

      YXRRSet = T.let(T.unsafe(nil), Integer)

      NXRRSet = T.let(T.unsafe(nil), Integer)

      NotAuth = T.let(T.unsafe(nil), Integer)

      NotZone = T.let(T.unsafe(nil), Integer)

      BADVERS = T.let(T.unsafe(nil), Integer)

      BADSIG = T.let(T.unsafe(nil), Integer)

      BADKEY = T.let(T.unsafe(nil), Integer)

      NoError = T.let(T.unsafe(nil), Integer)

      BADTIME = T.let(T.unsafe(nil), Integer)

      BADMODE = T.let(T.unsafe(nil), Integer)

      BADNAME = T.let(T.unsafe(nil), Integer)

      BADALG = T.let(T.unsafe(nil), Integer)

      NXDomain = T.let(T.unsafe(nil), Integer)
    end

    module Label
      class Str
        def string()
        end

        def eql?(other)
        end

        def downcase()
        end

        def inspect()
        end

        def to_s()
        end

        def hash()
        end

        def ==(other)
        end
      end
    end

    class Requester
      class MDNSOneShot < ::Resolv::DNS::Requester::UnconnectedUDP
        def sender(msg, data, host, port = _)
        end

        def sender_for(addr, msg)
        end
      end

      class RequestError < ::StandardError

      end

      class ConnectedUDP < ::Resolv::DNS::Requester
        class Sender < ::Resolv::DNS::Requester::Sender
          def send()
          end

          def data()
          end
        end

        def recv_reply(readable_socks)
        end

        def sender(msg, data, host = _, port = _)
        end

        def close()
        end
      end

      class Sender

      end

      class UnconnectedUDP < ::Resolv::DNS::Requester
        class Sender < ::Resolv::DNS::Requester::Sender
          def data()
          end

          def send()
          end
        end

        def recv_reply(readable_socks)
        end

        def sender(msg, data, host, port = _)
        end

        def close()
        end
      end

      class TCP < ::Resolv::DNS::Requester
        class Sender < ::Resolv::DNS::Requester::Sender
          def send()
          end

          def data()
          end
        end

        def recv_reply(readable_socks)
        end

        def sender(msg, data, host = _, port = _)
        end

        def close()
        end
      end

      def sender_for(addr, msg)
      end

      def close()
      end

      def request(sender, tout)
      end
    end

    class EncodeError < ::StandardError

    end

    class Name
      def [](i)
      end

      def length()
      end

      def subdomain_of?(other)
      end

      def eql?(other)
      end

      def absolute?()
      end

      def inspect()
      end

      def ==(other)
      end

      def hash()
      end

      def to_a()
      end

      def to_s()
      end
    end

    module OpCode
      Status = T.let(T.unsafe(nil), Integer)

      Query = T.let(T.unsafe(nil), Integer)

      IQuery = T.let(T.unsafe(nil), Integer)

      Notify = T.let(T.unsafe(nil), Integer)

      Update = T.let(T.unsafe(nil), Integer)
    end

    RequestID = T.let(T.unsafe(nil), Hash)

    RequestIDMutex = T.let(T.unsafe(nil), Thread::Mutex)

    Port = T.let(T.unsafe(nil), Integer)

    UDPSize = T.let(T.unsafe(nil), Integer)

    def make_udp_requester()
    end

    def timeouts=(values)
    end

    def each_resource(name, typeclass, &proc)
    end

    def getaddresses(name)
    end

    def each_address(name)
    end

    def getname(address)
    end

    def each_name(address)
    end

    def getnames(address)
    end

    def getaddress(name)
    end

    def make_tcp_requester(host, port)
    end

    def close()
    end

    def getresource(name, typeclass)
    end

    def lazy_initialize()
    end

    def getresources(name, typeclass)
    end

    def fetch_resource(name, typeclass)
    end

    def extract_resources(msg, name, typeclass)
    end
  end

  class IPv6
    Regex_CompressedHex = T.let(T.unsafe(nil), Regexp)

    Regex_6Hex4Dec = T.let(T.unsafe(nil), Regexp)

    Regex_CompressedHex4Dec = T.let(T.unsafe(nil), Regexp)

    Regex = T.let(T.unsafe(nil), Regexp)

    Regex_8Hex = T.let(T.unsafe(nil), Regexp)

    def to_name()
    end

    def eql?(other)
    end

    def address()
    end

    def ==(other)
    end

    def inspect()
    end

    def to_s()
    end

    def hash()
    end
  end

  DefaultResolver = T.let(T.unsafe(nil), Resolv)

  module LOC
    class Alt
      Regex = T.let(T.unsafe(nil), Regexp)

      def altitude()
      end

      def eql?(other)
      end

      def inspect()
      end

      def to_s()
      end

      def hash()
      end

      def ==(other)
      end
    end

    class Size
      Regex = T.let(T.unsafe(nil), Regexp)

      def eql?(other)
      end

      def scalar()
      end

      def inspect()
      end

      def to_s()
      end

      def hash()
      end

      def ==(other)
      end
    end

    class Coord
      Regex = T.let(T.unsafe(nil), Regexp)

      def orientation()
      end

      def eql?(other)
      end

      def ==(other)
      end

      def coordinates()
      end

      def to_s()
      end

      def inspect()
      end

      def hash()
      end
    end
  end

  class Hosts
    DefaultFileName = T.let(T.unsafe(nil), String)

    def getaddress(name)
    end

    def getaddresses(name)
    end

    def each_address(name, &proc)
    end

    def getname(address)
    end

    def lazy_initialize()
    end

    def getnames(address)
    end

    def each_name(address, &proc)
    end
  end

  class MDNS < ::Resolv::DNS
    AddressV6 = T.let(T.unsafe(nil), String)

    Addresses = T.let(T.unsafe(nil), Array)

    Port = T.let(T.unsafe(nil), Integer)

    AddressV4 = T.let(T.unsafe(nil), String)

    def each_address(name)
    end

    def make_udp_requester()
    end
  end

  class ResolvError < ::StandardError

  end

  def getaddress(name)
  end

  def getaddresses(name)
  end

  def each_address(name)
  end

  def getname(address)
  end

  def getnames(address)
  end

  def each_name(address)
  end
end
