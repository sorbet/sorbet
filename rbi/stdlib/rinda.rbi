# typed: __STDLIB_INTERNAL

# A module to implement the Linda distributed computing paradigm in Ruby.
#
# [`Rinda`](https://docs.ruby-lang.org/en/2.6.0/Rinda.html) is part of
# [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html) (dRuby).
#
# ## Example(s)
#
# See the sample/drb/ directory in the Ruby distribution, from 1.8.2 onwards.
module Rinda; end

# *Documentation?*
class Rinda::DRbObjectTemplate
  # Creates a new
  # [`DRbObjectTemplate`](https://docs.ruby-lang.org/en/2.6.0/Rinda/DRbObjectTemplate.html)
  # that will match against `uri` and `ref`.
  def self.new(uri = _, ref = _); end

  # This
  # [`DRbObjectTemplate`](https://docs.ruby-lang.org/en/2.6.0/Rinda/DRbObjectTemplate.html)
  # matches `ro` if the remote object's drburi and drbref are the same. `nil` is
  # used as a wildcard.
  def ===(ro); end
end

# Raised when a hash-based tuple has an invalid key.
class Rinda::InvalidHashTupleKey < ::Rinda::RindaError; end

# A
# [`NotifyTemplateEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/NotifyTemplateEntry.html)
# is returned by TupleSpace#notify and is notified of TupleSpace changes. You
# may receive either your subscribed event or the 'close' event when iterating
# over notifications.
#
# See TupleSpace#notify\_event for valid notification types.
#
# ## Example
#
# ```ruby
# ts = Rinda::TupleSpace.new
# observer = ts.notify 'write', [nil]
#
# Thread.start do
#   observer.each { |t| p t }
# end
#
# 3.times { |i| ts.write [i] }
# ```
#
# Outputs:
#
# ```ruby
# ['write', [0]]
# ['write', [1]]
# ['write', [2]]
# ```
class Rinda::NotifyTemplateEntry < ::Rinda::TemplateEntry
  # Creates a new
  # [`NotifyTemplateEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/NotifyTemplateEntry.html)
  # that watches `place` for +event+s that match `tuple`.
  def self.new(place, event, tuple, expires = _); end

  # Yields event/tuple pairs until this
  # [`NotifyTemplateEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/NotifyTemplateEntry.html)
  # expires.
  def each; end

  # Called by TupleSpace to notify this
  # [`NotifyTemplateEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/NotifyTemplateEntry.html)
  # of a new event.
  def notify(ev); end

  # Retrieves a notification. Raises RequestExpiredError when this
  # [`NotifyTemplateEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/NotifyTemplateEntry.html)
  # expires.
  def pop; end
end

# Raised when trying to use a canceled tuple.
class Rinda::RequestCanceledError < ::ThreadError; end

# Raised when trying to use an expired tuple.
class Rinda::RequestExpiredError < ::ThreadError; end

# [`Rinda`](https://docs.ruby-lang.org/en/2.6.0/Rinda.html) error base class
class Rinda::RindaError < ::RuntimeError; end

# [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html) is
# used by RingServer clients to discover the RingServer's TupleSpace. Typically,
# all a client needs to do is call
# [`RingFinger.primary`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html#method-c-primary)
# to retrieve the remote TupleSpace, which it can then begin using.
#
# To find the first available remote TupleSpace:
#
# ```ruby
# Rinda::RingFinger.primary
# ```
#
# To create a
# [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html) that
# broadcasts to a custom list:
#
# ```ruby
# rf = Rinda::RingFinger.new  ['localhost', '192.0.2.1']
# rf.primary
# ```
#
# [`Rinda::RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html)
# also understands multicast addresses and sets them up properly. This allows
# you to run multiple RingServers on the same host:
#
# ```ruby
# rf = Rinda::RingFinger.new ['239.0.0.1']
# rf.primary
# ```
#
# You can set the hop count (or TTL) for multicast searches using
# [`multicast_hops`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html#attribute-i-multicast_hops).
#
# If you use IPv6 multicast you may need to set both an address and the outbound
# interface index:
#
# ```ruby
# rf = Rinda::RingFinger.new ['ff02::1']
# rf.multicast_interface = 1
# rf.primary
# ```
#
# At this time there is no easy way to get an interface index by name.
class Rinda::RingFinger
  # Creates a new
  # [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html)
  # that will look for RingServers at `port` on the addresses in
  # `broadcast_list`.
  #
  # If `broadcast_list` contains a multicast address then multicast queries will
  # be made using the given
  # [`multicast_hops`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html#attribute-i-multicast_hops)
  # and multicast\_interface.
  def self.new(broadcast_list = _, port = _); end

  # The list of addresses where
  # [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html)
  # will send query packets.
  def broadcast_list; end

  # The list of addresses where
  # [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html)
  # will send query packets.
  def broadcast_list=(_); end

  # Iterates over all discovered TupleSpaces starting with the primary.
  def each; end

  # Looks up RingServers waiting `timeout` seconds. RingServers will be given
  # `block` as a callback, which will be called with the remote TupleSpace.
  def lookup_ring(timeout = _, &block); end

  # Returns the first found remote TupleSpace. Any further recovered TupleSpaces
  # can be found by calling `to_a`.
  def lookup_ring_any(timeout = _); end

  def make_socket(address); end

  # Maximum number of hops for sent multicast packets (if using a multicast
  # address in the broadcast list). The default is 1 (same as UDP broadcast).
  def multicast_hops; end

  # Maximum number of hops for sent multicast packets (if using a multicast
  # address in the broadcast list). The default is 1 (same as UDP broadcast).
  def multicast_hops=(_); end

  # The interface index to send IPv6 multicast packets from.
  def multicast_interface; end

  # The interface index to send IPv6 multicast packets from.
  def multicast_interface=(_); end

  # The port that
  # [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html)
  # will send query packets to.
  def port; end

  # The port that
  # [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html)
  # will send query packets to.
  def port=(_); end

  # Contain the first advertised TupleSpace after
  # [`lookup_ring_any`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html#method-i-lookup_ring_any)
  # is called.
  def primary; end

  # Contain the first advertised TupleSpace after
  # [`lookup_ring_any`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html#method-i-lookup_ring_any)
  # is called.
  def primary=(_); end

  def send_message(address, message); end

  # Contains all discovered TupleSpaces except for the primary.
  def to_a; end

  # Creates a singleton
  # [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html)
  # and looks for a RingServer. Returns the created
  # [`RingFinger`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingFinger.html).
  def self.finger; end

  # Returns the first advertised TupleSpace.
  def self.primary; end

  # Contains all discovered TupleSpaces except for the primary.
  def self.to_a; end
end

# [`RingProvider`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingProvider.html)
# uses a RingServer advertised TupleSpace as a name service. TupleSpace clients
# can register themselves with the remote TupleSpace and look up other provided
# services via the remote TupleSpace.
#
# Services are registered with a tuple of the format [:name, klass, DRbObject,
# description].
class Rinda::RingProvider
  # Creates a
  # [`RingProvider`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingProvider.html)
  # that will provide a `klass` service running on `front`, with a
  # `description`. `renewer` is optional.
  def self.new(klass, front, desc, renewer = _); end

  # Advertises this service on the primary remote TupleSpace.
  def provide; end
end

# A [`RingServer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingServer.html)
# allows a
# [`Rinda::TupleSpace`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleSpace.html)
# to be located via UDP broadcasts. Default service location uses the following
# steps:
#
# 1.  A
#     [`RingServer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingServer.html)
#     begins listening on the network broadcast UDP address.
# 2.  A RingFinger sends a UDP packet containing the
#     [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html)
#     [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) where it will listen
#     for a reply.
# 3.  The
#     [`RingServer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingServer.html)
#     receives the UDP packet and connects back to the provided
#     [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html)
#     [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) with the
#     [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html) service.
#
#
# A [`RingServer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingServer.html)
# requires a TupleSpace:
#
# ```ruby
# ts = Rinda::TupleSpace.new
# rs = Rinda::RingServer.new
# ```
#
# [`RingServer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingServer.html) can
# also listen on multicast addresses for announcements. This allows multiple
# RingServers to run on the same host. To use network broadcast and multicast:
#
# ```ruby
# ts = Rinda::TupleSpace.new
# rs = Rinda::RingServer.new ts, %w[Socket::INADDR_ANY, 239.0.0.1 ff02::1]
# ```
class Rinda::RingServer
  include(::DRb::DRbUndumped)

  # Advertises `ts` on the given `addresses` at `port`.
  #
  # If `addresses` is omitted only the UDP broadcast address is used.
  #
  # `addresses` can contain multiple addresses. If a multicast address is given
  # in `addresses` then the
  # [`RingServer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingServer.html)
  # will listen for multicast queries.
  #
  # If you use IPv4 multicast you may need to set an address of the inbound
  # interface which joins a multicast group.
  #
  # ```ruby
  # ts = Rinda::TupleSpace.new
  # rs = Rinda::RingServer.new(ts, [['239.0.0.1', '9.5.1.1']])
  # ```
  #
  # You can set addresses as an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
  # [`Object`](https://docs.ruby-lang.org/en/2.6.0/Object.html). The first
  # element of the [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) is
  # a multicast address and the second is an inbound interface address. If the
  # second is omitted then '0.0.0.0' is used.
  #
  # If you use IPv6 multicast you may need to set both the local interface
  # address and the inbound interface index:
  #
  # ```ruby
  # rs = Rinda::RingServer.new(ts, [['ff02::1', '::1', 1]])
  # ```
  #
  # The first element is a multicast address and the second is an inbound
  # interface address. The third is an inbound interface index.
  #
  # At this time there is no easy way to get an interface index by name.
  #
  # If the second is omitted then '::1' is used. If the third is omitted then 0
  # (default interface) is used.
  def self.new(ts, addresses = _, port = _); end

  # Pulls lookup tuples out of the TupleSpace and sends their
  # [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html) object the address of
  # the local TupleSpace.
  def do_reply; end

  # Extracts the response [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html)
  # from `msg` and adds it to TupleSpace where it will be picked up by
  # `reply_service` for notification.
  def do_write(msg); end

  # Creates a socket at `address`
  #
  # If `address` is multicast address then `interface_address` and
  # `multicast_interface` can be set as optional.
  #
  # A created socket is bound to `interface_address`. If you use IPv4 multicast
  # then the interface of `interface_address` is used as the inbound interface.
  # If `interface_address` is omitted or nil then '0.0.0.0' or '::1' is used.
  #
  # If you use IPv6 multicast then `multicast_interface` is used as the inbound
  # interface. `multicast_interface` is a network interface index. If
  # `multicast_interface` is omitted then 0 (default interface) is used.
  def make_socket(address, interface_address = _, multicast_interface = _); end

  # Creates a thread that notifies waiting clients from the TupleSpace.
  def reply_service; end

  # Shuts down the
  # [`RingServer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingServer.html)
  def shutdown; end

  # Creates threads that pick up UDP packets and passes them to
  # [`do_write`](https://docs.ruby-lang.org/en/2.6.0/Rinda/RingServer.html#method-i-do_write)
  # for decoding.
  def write_services; end
end

class Rinda::RingServer::Renewer
  include(::DRb::DRbUndumped)

  def self.new; end

  def renew; end

  def renew=(_); end
end

# The default port Ring discovery will use.
Rinda::Ring_PORT = T.let(T.unsafe(nil), Integer)

# An
# [`SimpleRenewer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/SimpleRenewer.html)
# allows a TupleSpace to check if a TupleEntry is still alive.
class Rinda::SimpleRenewer
  include(::DRb::DRbUndumped)

  # Creates a new
  # [`SimpleRenewer`](https://docs.ruby-lang.org/en/2.6.0/Rinda/SimpleRenewer.html)
  # that keeps an object alive for another `sec` seconds.
  def self.new(sec = _); end

  # Called by the TupleSpace to check if the object is still alive.
  def renew; end
end

# Templates are used to match tuples in
# [`Rinda`](https://docs.ruby-lang.org/en/2.6.0/Rinda.html).
class Rinda::Template < ::Rinda::Tuple
  # Alias for
  # [`match`](https://docs.ruby-lang.org/en/2.6.0/Rinda/Template.html#method-i-match).
  def ===(tuple); end

  # Matches this template against `tuple`. The `tuple` must be the same size as
  # the template. An element with a `nil` value in a template acts as a
  # wildcard, matching any value in the corresponding position in the tuple.
  # Elements of the template match the `tuple` if the are #== or
  # [`===`](https://docs.ruby-lang.org/en/2.6.0/Rinda/Template.html#method-i-3D-3D-3D).
  #
  # ```ruby
  # Template.new([:foo, 5]).match   Tuple.new([:foo, 5]) # => true
  # Template.new([:foo, nil]).match Tuple.new([:foo, 5]) # => true
  # Template.new([String]).match    Tuple.new(['hello']) # => true
  #
  # Template.new([:foo]).match      Tuple.new([:foo, 5]) # => false
  # Template.new([:foo, 6]).match   Tuple.new([:foo, 5]) # => false
  # Template.new([:foo, nil]).match Tuple.new([:foo])    # => false
  # Template.new([:foo, 6]).match   Tuple.new([:foo])    # => false
  # ```
  def match(tuple); end
end

# A
# [`TemplateEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TemplateEntry.html)
# is a Template together with expiry and cancellation data.
class Rinda::TemplateEntry < ::Rinda::TupleEntry
  # Alias for:
  # [`match`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TemplateEntry.html#method-i-match)
  def ===(tuple); end

  def make_tuple(ary); end

  # Matches this
  # [`TemplateEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TemplateEntry.html)
  # against `tuple`. See Template#match for details on how a Template matches a
  # Tuple.
  #
  # Also aliased as:
  # [`===`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TemplateEntry.html#method-i-3D-3D-3D)
  def match(tuple); end
end

# A tuple is the elementary object in
# [`Rinda`](https://docs.ruby-lang.org/en/2.6.0/Rinda.html) programming. Tuples
# may be matched against templates if the tuple and the template are the same
# size.
class Rinda::Tuple
  # Creates a new
  # [`Tuple`](https://docs.ruby-lang.org/en/2.6.0/Rinda/Tuple.html) from
  # `ary_or_hash` which must be an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) or
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
  def self.new(ary_or_hash); end

  # Accessor method for elements of the tuple.
  def [](k); end

  # Iterate through the tuple, yielding the index or key, and the value, thus
  # ensuring arrays are iterated similarly to hashes.
  def each; end

  # Fetches item `k` from the tuple.
  def fetch(k); end

  # The number of elements in the tuple.
  def size; end

  # Return the tuple itself
  def value; end
end

# [`TupleBag`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleBag.html) is an
# unordered collection of tuples. It is the basis of Tuplespace.
class Rinda::TupleBag
  def self.new; end

  # Removes `tuple` from the
  # [`TupleBag`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleBag.html).
  def delete(tuple); end

  # Delete tuples which dead tuples from the
  # [`TupleBag`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleBag.html),
  # returning the deleted tuples.
  def delete_unless_alive; end

  # Finds a live tuple that matches `template`.
  def find(template); end

  # Finds all live tuples that match `template`.
  def find_all(template); end

  # Finds all tuples in the
  # [`TupleBag`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleBag.html) which
  # when treated as templates, match `tuple` and are alive.
  def find_all_template(tuple); end

  # `true` if the
  # [`TupleBag`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleBag.html) to see
  # if it has any expired entries.
  def has_expires?; end

  # Add `tuple` to the
  # [`TupleBag`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleBag.html).
  def push(tuple); end
end

class Rinda::TupleBag::TupleBin
  extend(::Forwardable)

  def self.new; end

  def add(tuple); end

  def delete(tuple); end

  def delete_if(*args, &block); end

  def each(*args, &block); end

  def empty?(*args, &block); end

  def find; end

  def find_all(*args, &block); end
end

# A [`TupleEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleEntry.html) is
# a Tuple (i.e. a possible entry in some Tuplespace) together with expiry and
# cancellation data.
class Rinda::TupleEntry
  include(::DRb::DRbUndumped)

  # Creates a
  # [`TupleEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleEntry.html)
  # based on `ary` with an optional renewer or expiry time `sec`.
  #
  # A renewer must implement the `renew` method which returns a Numeric, nil, or
  # true to indicate when the tuple has expired.
  def self.new(ary, sec = _); end

  # Retrieves `key` from the tuple.
  def [](key); end

  # A [`TupleEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleEntry.html)
  # is dead when it is canceled or expired.
  def alive?; end

  # Marks this
  # [`TupleEntry`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleEntry.html) as
  # canceled.
  def cancel; end

  # Returns the canceled status.
  def canceled?; end

  # Has this tuple expired? (true/false).
  #
  # A tuple has expired when its expiry timer based on the `sec` argument to
  # initialize runs out.
  def expired?; end

  def expires; end

  def expires=(_); end

  # Fetches `key` from the tuple.
  def fetch(key); end

  # Returns an expiry [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html)
  # based on `sec` which can be one of:
  # Numeric
  # :   `sec` seconds into the future
  # `true`
  # :   the expiry time is the start of 1970 (i.e. expired)
  # `nil`
  # :   it is  Tue Jan 19 03:14:07 GMT Standard
  #     [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) 2038 (i.e. when
  #     UNIX clocks will die)
  def make_expires(sec = _); end

  # Creates a
  # [`Rinda::Tuple`](https://docs.ruby-lang.org/en/2.6.0/Rinda/Tuple.html) for
  # `ary`.
  def make_tuple(ary); end

  # Reset the expiry time according to `sec_or_renewer`.
  #
  # `nil`
  # :   it is set to expire in the far future.
  # `true`
  # :   it has expired.
  # Numeric
  # :   it will expire in that many seconds.
  #
  #
  # Otherwise the argument refers to some kind of renewer object which will
  # reset its expiry time.
  def renew(sec_or_renewer); end

  # The size of the tuple.
  def size; end

  # Return the object which makes up the tuple itself: the
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) or
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
  def value; end
end

# The Tuplespace manages access to the tuples it contains, ensuring mutual
# exclusion requirements are met.
#
# The `sec` option for the write, take, move, read and notify methods may either
# be a number of seconds or a Renewer object.
class Rinda::TupleSpace
  include(::MonitorMixin)
  include(::DRb::DRbUndumped)

  # Creates a new
  # [`TupleSpace`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleSpace.html).
  # `period` is used to control how often to look for dead tuples after
  # modifications to the
  # [`TupleSpace`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleSpace.html).
  #
  # If no dead tuples are found `period` seconds after the last modification,
  # the
  # [`TupleSpace`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleSpace.html)
  # will stop looking for dead tuples.
  def self.new(period = _); end

  # Moves `tuple` to `port`.
  def move(port, tuple, sec = _); end

  # Registers for notifications of `event`. Returns a NotifyTemplateEntry. See
  # NotifyTemplateEntry for examples of how to listen for notifications.
  #
  # `event` can be:
  # 'write'
  # :   A tuple was added
  # 'take'
  # :   A tuple was taken or moved
  # 'delete'
  # :   A tuple was lost after being overwritten or expiring
  #
  #
  # The
  # [`TupleSpace`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleSpace.html)
  # will also notify you of the 'close' event when the NotifyTemplateEntry has
  # expired.
  def notify(event, tuple, sec = _); end

  # Reads `tuple`, but does not remove it.
  def read(tuple, sec = _); end

  # Returns all tuples matching `tuple`. Does not remove the found tuples.
  def read_all(tuple); end

  # Removes `tuple`
  def take(tuple, sec = _, &block); end

  # Adds `tuple`
  def write(tuple, sec = _); end
end

# [`TupleSpaceProxy`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleSpaceProxy.html)
# allows a remote Tuplespace to appear as local.
class Rinda::TupleSpaceProxy
  # Creates a new
  # [`TupleSpaceProxy`](https://docs.ruby-lang.org/en/2.6.0/Rinda/TupleSpaceProxy.html)
  # to wrap `ts`.
  def self.new(ts); end

  # Registers for notifications of event `ev` on the proxied TupleSpace. See
  # TupleSpace#notify
  def notify(ev, tuple, sec = _); end

  # Reads `tuple` from the proxied TupleSpace. See TupleSpace#read.
  def read(tuple, sec = _, &block); end

  # Reads all tuples matching `tuple` from the proxied TupleSpace. See
  # TupleSpace#read\_all.
  def read_all(tuple); end

  # Takes `tuple` from the proxied TupleSpace. See TupleSpace#take.
  def take(tuple, sec = _, &block); end

  # Adds `tuple` to the proxied TupleSpace. See TupleSpace#write.
  def write(tuple, sec = _); end
end

class Rinda::TupleSpaceProxy::Port
  def self.new; end

  def close; end

  def push(value); end

  def value; end

  def self.deliver; end
end

# *Documentation?*
class Rinda::WaitTemplateEntry < ::Rinda::TemplateEntry
  def self.new(place, ary, expires = _); end

  def cancel; end

  def found; end

  def read(tuple); end

  def signal; end

  def wait; end
end
