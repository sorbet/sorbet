# typed: true

# ## Overview
#
# dRuby is a distributed object system for Ruby. It is written in pure Ruby and
# uses its own protocol. No add-in services are needed beyond those provided by
# the Ruby runtime, such as TCP sockets. It does not rely on or interoperate
# with other distributed object systems such as CORBA, RMI, or .NET.
#
# dRuby allows methods to be called in one Ruby process upon a Ruby object
# located in another Ruby process, even on another machine. References to
# objects can be passed between processes.
# [`Method`](https://docs.ruby-lang.org/en/2.6.0/Method.html) arguments and
# return values are dumped and loaded in marshalled format. All of this is done
# transparently to both the caller of the remote method and the object that it
# is called upon.
#
# An object in a remote process is locally represented by a
# [`DRb::DRbObject`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbObject.html)
# instance. This acts as a sort of proxy for the remote object. Methods called
# upon this DRbObject instance are forwarded to its remote object. This is
# arranged dynamically at run time. There are no statically declared interfaces
# for remote objects, such as CORBA's IDL.
#
# dRuby calls made into a process are handled by a
# [`DRb::DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html)
# instance within that process. This reconstitutes the method call, invokes it
# upon the specified local object, and returns the value to the remote caller.
# Any object can receive calls over dRuby. There is no need to implement a
# special interface, or mixin special functionality. Nor, in the general case,
# does an object need to explicitly register itself with a DRbServer in order to
# receive dRuby calls.
#
# One process wishing to make dRuby calls upon another process must somehow
# obtain an initial reference to an object in the remote process by some means
# other than as the return value of a remote method call, as there is initially
# no remote object reference it can invoke a method upon. This is done by
# attaching to the server by
# [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html). Each DRbServer binds
# itself to a [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) such as
# 'druby://example.com:8787'. A DRbServer can have an object attached to it that
# acts as the server's **front** **object**. A DRbObject can be explicitly
# created from the server's
# [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html). This DRbObject's remote
# object will be the server's front object. This front object can then return
# references to other Ruby objects in the DRbServer's process.
#
# [`Method`](https://docs.ruby-lang.org/en/2.6.0/Method.html) calls made over
# dRuby behave largely the same as normal Ruby method calls made within a
# process. [`Method`](https://docs.ruby-lang.org/en/2.6.0/Method.html) calls
# with blocks are supported, as are raising exceptions. In addition to a
# method's standard errors, a dRuby call may also raise one of the
# dRuby-specific errors, all of which are subclasses of
# [`DRb::DRbError`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbError.html).
#
# Any type of object can be passed as an argument to a dRuby call or returned as
# its return value. By default, such objects are dumped or marshalled at the
# local end, then loaded or unmarshalled at the remote end. The remote end
# therefore receives a copy of the local object, not a distributed reference to
# it; methods invoked upon this copy are executed entirely in the remote
# process, not passed on to the local original. This has semantics similar to
# pass-by-value.
#
# However, if an object cannot be marshalled, a dRuby reference to it is passed
# or returned instead. This will turn up at the remote end as a DRbObject
# instance. All methods invoked upon this remote proxy are forwarded to the
# local object, as described in the discussion of DRbObjects. This has semantics
# similar to the normal Ruby pass-by-reference.
#
# The easiest way to signal that we want an otherwise marshallable object to be
# passed or returned as a DRbObject reference, rather than marshalled and sent
# as a copy, is to include the
# [`DRb::DRbUndumped`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUndumped.html)
# mixin module.
#
# dRuby supports calling remote methods with blocks. As blocks (or rather the
# [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) objects that represent
# them) are not marshallable, the block executes in the local, not the remote,
# context. Each value yielded to the block is passed from the remote object to
# the local block, then the value returned by each block invocation is passed
# back to the remote execution context to be collected, before the collected
# values are finally returned to the local context as the return value of the
# method invocation.
#
# ## Examples of usage
#
# For more dRuby samples, see the `samples` directory in the full dRuby
# distribution.
#
# ### dRuby in client/server mode
#
# This illustrates setting up a simple client-server drb system. Run the server
# and client code in different terminals, starting the server code first.
#
# #### Server code
#
# ```ruby
# require 'drb/drb'
#
# # The URI for the server to connect to
# URI="druby://localhost:8787"
#
# class TimeServer
#
#   def get_current_time
#     return Time.now
#   end
#
# end
#
# # The object that handles requests on the server
# FRONT_OBJECT=TimeServer.new
#
# $SAFE = 1   # disable eval() and friends
#
# DRb.start_service(URI, FRONT_OBJECT)
# # Wait for the drb server thread to finish before exiting.
# DRb.thread.join
# ```
#
# #### Client code
#
# ```ruby
# require 'drb/drb'
#
# # The URI to connect to
# SERVER_URI="druby://localhost:8787"
#
# # Start a local DRbServer to handle callbacks.
# #
# # Not necessary for this small example, but will be required
# # as soon as we pass a non-marshallable object as an argument
# # to a dRuby call.
# #
# # Note: this must be called at least once per process to take any effect.
# # This is particularly important if your application forks.
# DRb.start_service
#
# timeserver = DRbObject.new_with_uri(SERVER_URI)
# puts timeserver.get_current_time
# ```
#
# ### Remote objects under dRuby
#
# This example illustrates returning a reference to an object from a dRuby call.
# The [`Logger`](https://docs.ruby-lang.org/en/2.6.0/Logger.html) instances live
# in the server process. References to them are returned to the client process,
# where methods can be invoked upon them. These methods are executed in the
# server process.
#
# #### Server code
#
# ```ruby
# require 'drb/drb'
#
# URI="druby://localhost:8787"
#
# class Logger
#
#     # Make dRuby send Logger instances as dRuby references,
#     # not copies.
#     include DRb::DRbUndumped
#
#     def initialize(n, fname)
#         @name = n
#         @filename = fname
#     end
#
#     def log(message)
#         File.open(@filename, "a") do |f|
#             f.puts("#{Time.now}: #{@name}: #{message}")
#         end
#     end
#
# end
#
# # We have a central object for creating and retrieving loggers.
# # This retains a local reference to all loggers created.  This
# # is so an existing logger can be looked up by name, but also
# # to prevent loggers from being garbage collected.  A dRuby
# # reference to an object is not sufficient to prevent it being
# # garbage collected!
# class LoggerFactory
#
#     def initialize(bdir)
#         @basedir = bdir
#         @loggers = {}
#     end
#
#     def get_logger(name)
#         if !@loggers.has_key? name
#             # make the filename safe, then declare it to be so
#             fname = name.gsub(/[.\/\\\:]/, "_").untaint
#             @loggers[name] = Logger.new(name, @basedir + "/" + fname)
#         end
#         return @loggers[name]
#     end
#
# end
#
# FRONT_OBJECT=LoggerFactory.new("/tmp/dlog")
#
# $SAFE = 1   # disable eval() and friends
#
# DRb.start_service(URI, FRONT_OBJECT)
# DRb.thread.join
# ```
#
# #### Client code
#
# ```ruby
# require 'drb/drb'
#
# SERVER_URI="druby://localhost:8787"
#
# DRb.start_service
#
# log_service=DRbObject.new_with_uri(SERVER_URI)
#
# ["loga", "logb", "logc"].each do |logname|
#
#     logger=log_service.get_logger(logname)
#
#     logger.log("Hello, world!")
#     logger.log("Goodbye, world!")
#     logger.log("=== EOT ===")
#
# end
# ```
#
# ## Security
#
# As with all network services, security needs to be considered when using
# dRuby. By allowing external access to a Ruby object, you are not only allowing
# outside clients to call the methods you have defined for that object, but by
# default to execute arbitrary Ruby code on your server. Consider the following:
#
# ```ruby
# # !!! UNSAFE CODE !!!
# ro = DRbObject::new_with_uri("druby://your.server.com:8989")
# class << ro
#   undef :instance_eval  # force call to be passed to remote object
# end
# ro.instance_eval("`rm -rf *`")
# ```
#
# The dangers posed by instance\_eval and friends are such that a DRbServer
# should generally be run with $SAFE set to at least level 1. This will disable
# eval() and related calls on strings passed across the wire. The sample usage
# code given above follows this practice.
#
# A DRbServer can be configured with an access control list to selectively allow
# or deny access from specified IP addresses. The main druby distribution
# provides the [`ACL`](https://docs.ruby-lang.org/en/2.6.0/ACL.html) class for
# this purpose. In general, this mechanism should only be used alongside, rather
# than as a replacement for, a good firewall.
#
# ## dRuby internals
#
# dRuby is implemented using three main components: a remote method call
# marshaller/unmarshaller; a transport protocol; and an ID-to-object mapper. The
# latter two can be directly, and the first indirectly, replaced, in order to
# provide different behaviour and capabilities.
#
# Marshalling and unmarshalling of remote method calls is performed by a
# [`DRb::DRbMessage`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbMessage.html)
# instance. This uses the
# [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html) module to dump
# the method call before sending it over the transport layer, then reconstitute
# it at the other end. There is normally no need to replace this component, and
# no direct way is provided to do so. However, it is possible to implement an
# alternative marshalling scheme as part of an implementation of the transport
# layer.
#
# The transport layer is responsible for opening client and server network
# connections and forwarding dRuby request across them. Normally, it uses
# [`DRb::DRbMessage`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbMessage.html)
# internally to manage marshalling and unmarshalling. The transport layer is
# managed by
# [`DRb::DRbProtocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html).
# Multiple protocols can be installed in DRbProtocol at the one time; selection
# between them is determined by the scheme of a dRuby
# [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html). The default transport
# protocol is selected by the scheme 'druby:', and implemented by
# [`DRb::DRbTCPSocket`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbTCPSocket.html).
# This uses plain TCP/IP sockets for communication. An alternative protocol,
# using UNIX domain sockets, is implemented by
# [`DRb::DRbUNIXSocket`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUNIXSocket.html)
# in the file drb/unix.rb, and selected by the scheme 'drbunix:'. A sample
# implementation over HTTP can be found in the samples accompanying the main
# dRuby distribution.
#
# The ID-to-object mapping component maps dRuby object ids to the objects they
# refer to, and vice versa. The implementation to use can be specified as part
# of a DRb::DRbServer's configuration. The default implementation is provided by
# [`DRb::DRbIdConv`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbIdConv.html). It
# uses an object's
# [`ObjectSpace`](https://docs.ruby-lang.org/en/2.6.0/ObjectSpace.html) id as
# its dRuby id. This means that the dRuby reference to that object only remains
# meaningful for the lifetime of the object's process and the lifetime of the
# object within that process. A modified implementation is provided by
# [`DRb::TimerIdConv`](https://docs.ruby-lang.org/en/2.6.0/DRb/TimerIdConv.html)
# in the file drb/timeridconv.rb.  This implementation retains a local reference
# to all objects exported over dRuby for a configurable period of time
# (defaulting to ten minutes), to prevent them being garbage-collected within
# this time. Another sample implementation is provided in sample/name.rb in the
# main dRuby distribution. This allows objects to specify their own id or
# "name". A dRuby reference can be made persistent across processes by having
# each process register an object using the same dRuby name.
module DRb
  # Get the configuration of the current server.
  #
  # If there is no current server, this returns the default configuration. See
  # [`current_server`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-current_server)
  # and DRbServer::make\_config.
  sig { returns(T::Hash[Symbol, T.untyped]) }
  def self.config; end

  # Get the 'current' server.
  #
  # In the context of execution taking place within the main thread of a dRuby
  # server (typically, as a result of a remote call on the server or one of its
  # objects), the current server is that server. Otherwise, the current server
  # is the primary server.
  #
  # If the above rule fails to find a server, a DRbServerNotFound error is
  # raised.
  sig { returns(DRb::DRbServer) }
  def self.current_server; end

  # Retrieves the server with the given `uri`.
  #
  # See also
  # [`regist_server`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-c-regist_server)
  # and remove\_server.
  sig { params(uri: String).returns(T.nilable(DRb::DRbServer)) }
  def self.fetch_server(uri); end

  # Get the front object of the current server.
  #
  # This raises a DRbServerNotFound error if there is no current server. See
  # [`current_server`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-current_server).
  sig { returns(T.nilable(Object)) }
  def self.front; end

  # Is `uri` the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) for the
  # current local server?
  sig { params(uri: String).returns(T::Boolean) }
  def self.here?(uri); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default
  # [`ACL`](https://docs.ruby-lang.org/en/2.6.0/ACL.html) to `acl`.
  #
  # See
  # [`DRb::DRbServer.default_acl`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html#method-c-default_acl).
  sig { params(acl: T.nilable(Object)).returns(T.nilable(Object)) }
  def self.install_acl(acl); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default id
  # conversion object.
  #
  # This is expected to be an instance such as
  # [`DRb::DRbIdConv`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbIdConv.html)
  # that responds to
  # [`to_id`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-to_id) and
  # [`to_obj`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-to_obj)
  # that can convert objects to and from
  # [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html) references.
  #
  # See DRbServer#default\_id\_conv.
  sig { params(idconv: T.nilable(Object)).returns(T.nilable(Object)) }
  def self.install_id_conv(idconv); end

  # The primary local dRuby server.
  #
  # This is the server created by the
  # [`start_service`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-start_service)
  # call.
  sig { returns(T.nilable(DRb::DRbServer)) }
  def self.primary_server; end

  # The primary local dRuby server.
  #
  # This is the server created by the
  # [`start_service`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-start_service)
  # call.
  sig { params(server: T.nilable(DRb::DRbServer)).returns(T.nilable(DRb::DRbServer)) }
  def self.primary_server=(server); end

  # Registers `server` with
  # [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html).
  #
  # This is called when a new
  # [`DRb::DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html)
  # is created.
  #
  # If there is no primary server then `server` becomes the primary server.
  #
  # Example:
  #
  # ```ruby
  # require 'drb'
  #
  # s = DRb::DRbServer.new # automatically calls regist_server
  # DRb.fetch_server s.uri #=> #<DRb::DRbServer:0x...>
  # ```
  sig { params(server: DRb::DRbServer).void }
  def self.regist_server(server); end

  # Removes `server` from the list of registered servers.
  sig { params(server: DRb::DRbServer).void }
  def self.remove_server(server); end

  # Start a dRuby server locally.
  #
  # The new dRuby server will become the primary server, even if another server
  # is currently the primary server.
  #
  # `uri` is the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) for the
  # server to bind to. If nil, the server will bind to random port on the
  # default local host name and use the default dRuby protocol.
  #
  # `front` is the server's front object. This may be nil.
  #
  # `config` is the configuration for the new server. This may be nil.
  #
  # See DRbServer::new.
  sig do
    params(
      uri: T.nilable(String),
      front: T.nilable(Object),
      config: T.nilable(T::Hash[Symbol, T.untyped])
    ).returns(DRb::DRbServer)
  end
  def self.start_service(uri = _, front = _, config = _); end

  # Stop the local dRuby server.
  #
  # This operates on the primary server. If there is no primary server currently
  # running, it is a noop.
  sig { void }
  def self.stop_service; end

  # Get the thread of the primary server.
  #
  # This returns nil if there is no primary server. See
  # [`primary_server`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#attribute-i-primary_server).
  sig { returns(T.nilable(Thread)) }
  def self.thread; end

  # Get a reference id for an object using the current server.
  #
  # This raises a DRbServerNotFound error if there is no current server. See
  # [`current_server`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-current_server).
  sig { params(obj: T.nilable(Object)).returns(T.nilable(Integer)) }
  def self.to_id(obj); end

  # Convert a reference into an object using the current server.
  #
  # This raises a DRbServerNotFound error if there is no current server. See
  # [`current_server`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-current_server).
  sig { params(ref: T.nilable(Integer)).returns(T.nilable(Object)) }
  def self.to_obj(ref); end

  # Get the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) defining the
  # local dRuby space.
  #
  # This is the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) of the
  # current server. See
  # [`current_server`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-i-current_server).
  sig { returns(String) }
  def self.uri; end
end

# An [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) wrapper that can
# be sent to another server via
# [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html).
#
# All entries in the array will be dumped or be references that point to the
# local server.
class DRb::DRbArray
  sig { params(ary: T::Array[T.untyped]).void }
  def initialize(ary); end
end

# Error raised by a dRuby protocol when it doesn't support the scheme specified
# in a [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html). See
# [`DRb::DRbProtocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html).
class DRb::DRbBadScheme < ::DRb::DRbError; end

# Error raised by the DRbProtocol module when it cannot find any protocol
# implementation support the scheme specified in a
# [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html).
class DRb::DRbBadURI < ::DRb::DRbError; end

# [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) handling the
# connection between a DRbObject and the server the real object lives on.
#
# This class maintains a pool of connections, to reduce the overhead of starting
# and closing down connections for each method call.
#
# This class is used internally by DRbObject. The user does not normally need to
# deal with it directly.
class DRb::DRbConn
  sig { void }
  def make_pool; end

  sig { void }
  def stop_pool; end
end

DRb::DRbConn::POOL_SIZE = T.let(T.unsafe(nil), Integer)

# Error raised when an error occurs on the underlying communication protocol.
class DRb::DRbConnError < ::DRb::DRbError; end

# Superclass of all errors raised in the
# [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html) module.
class DRb::DRbError < ::RuntimeError; end

# [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) responsible for
# converting between an object and its id.
#
# This, the default implementation, uses an object's local
# [`ObjectSpace`](https://docs.ruby-lang.org/en/2.6.0/ObjectSpace.html)
# \_\_id\_\_ as its id. This means that an object's identification over drb
# remains valid only while that object instance remains alive within the server
# runtime.
#
# For alternative mechanisms, see
# [`DRb::TimerIdConv`](https://docs.ruby-lang.org/en/2.6.0/DRb/TimerIdConv.html)
# in drb/timeridconv.rb and DRbNameIdConv in sample/name.rb in the full drb
# distribution.
class DRb::DRbIdConv
  # Convert an object into a reference id.
  #
  # This implementation returns the object's \_\_id\_\_ in the local object
  # space.
  sig { params(obj: T.nilable(Object)).returns(T.nilable(Integer)) }
  def to_id(obj); end

  # Convert an object reference id to an object.
  #
  # This implementation looks up the reference id in the local object space and
  # returns the object it refers to.
  sig { params(ref: T.nilable(Integer)).returns(T.nilable(Object)) }
  def to_obj(ref); end
end

# Handler for sending and receiving drb messages.
#
# This takes care of the low-level marshalling and unmarshalling of drb requests
# and responses sent over the wire between server and client. This relieves the
# implementor of a new drb protocol layer with having to deal with these
# details.
#
# The user does not have to directly deal with this object in normal use.
# Handler for sending and receiving drb messages.
#
# This takes care of the low-level marshalling and unmarshalling of drb requests
# and responses sent over the wire between server and client. This relieves the
# implementor of a new drb protocol layer with having to deal with these
# details.
#
# The user does not have to directly deal with this object in normal use.
# Handler for sending and receiving drb messages.
#
# This takes care of the low-level marshalling and unmarshalling of drb requests
# and responses sent over the wire between server and client. This relieves the
# implementor of a new drb protocol layer with having to deal with these
# details.
#
# The user does not have to directly deal with this object in normal use.
# Handler for sending and receiving drb messages.
#
# This takes care of the low-level marshalling and unmarshalling of drb requests
# and responses sent over the wire between server and client. This relieves the
# implementor of a new drb protocol layer with having to deal with these
# details.
#
# The user does not have to directly deal with this object in normal use.
class DRb::DRbMessage; end

# [`Object`](https://docs.ruby-lang.org/en/2.6.0/Object.html) wrapping a
# reference to a remote drb object.
#
# [`Method`](https://docs.ruby-lang.org/en/2.6.0/Method.html) calls on this
# object are relayed to the remote object that this object is a stub for.
class DRb::DRbObject
  # Get the reference of the object, if local.
  sig { returns(T.nilable(Integer)) }
  def __drbref; end

  # Get the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) of the remote
  # object.
  sig { returns(T.nilable(String)) }
  def __drburi; end

  # Marshall this object.
  #
  # The [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) and ref of the
  # object are marshalled.
  sig { params(lv: T.nilable(Object)).void }
  def _dump(lv); end

  # Routes method calls to the referenced remote object.
  sig do
    params(
      msg_id: T.any(String, Symbol),
      a: T.nilable(Object),
      b: T.untyped
    ).returns(T::Boolean)
  end
  def method_missing(msg_id, *a, &b); end

  # Routes respond\_to? to the referenced remote object.
  sig do
    params(
      msg_id: T.any(String, Symbol),
      priv: T::Boolean
    ).returns(T::Boolean)
  end
  def respond_to?(msg_id, priv = false); end

  # Unmarshall a marshalled
  # [`DRbObject`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbObject.html).
  #
  # If the referenced object is located within the local server, then the object
  # itself is returned. Otherwise, a new
  # [`DRbObject`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbObject.html) is
  # created to act as a stub for the remote referenced object.
  sig { params(s: String).returns(T.nilable(Object)) }
  def self._load(s); end

  # Create a new remote object stub.
  #
  # `obj` is the (local) object we want to create a stub for. Normally this is
  # `nil`. `uri` is the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) of
  # the remote object that this will be a stub for.
  sig { params(obj: T.nilable(Object), uri: String).returns(DRb::DRbObject) }
  def self.new(obj, uri = _); end

  # Creates a
  # [`DRb::DRbObject`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbObject.html)
  # given the reference information to the remote host `uri` and object `ref`.
  sig { params(uri: String, ref: T.nilable(Object)).returns(DRb::DRbObject) }
  def self.new_with(uri, ref); end

  # Create a new
  # [`DRbObject`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbObject.html) from a
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) alone.
  sig { params(uri: String).returns(DRb::DRbObject) }
  def self.new_with_uri(uri); end
end

# [`Module`](https://docs.ruby-lang.org/en/2.6.0/Module.html) managing the
# underlying network protocol(s) used by drb.
#
# By default, drb uses the DRbTCPSocket protocol. Other protocols can be
# defined. A protocol must define the following class methods:
#
# ```
# [open(uri, config)] Open a client connection to the server at +uri+,
#                     using configuration +config+.  Return a protocol
#                     instance for this connection.
# [open_server(uri, config)] Open a server listening at +uri+,
#                            using configuration +config+.  Return a
#                            protocol instance for this listener.
# [uri_option(uri, config)] Take a URI, possibly containing an option
#                           component (e.g. a trailing '?param=val'),
#                           and return a [uri, option] tuple.
# ```
#
# All of these methods should raise a DRbBadScheme error if the
# [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) does not identify the
# protocol they support (e.g. "druby:" for the standard Ruby protocol). This is
# how the
# [`DRbProtocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html)
# module, given a [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html),
# determines which protocol implementation serves that protocol.
#
# The protocol instance returned by
# [`open_server`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html#method-i-open_server)
# must have the following methods:
#
# accept
# :   Accept a new connection to the server. Returns a protocol instance capable
#     of communicating with the client.
# close
# :   Close the server connection.
# uri
# :   Get the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) for this
#     server.
#
#
# The protocol instance returned by
# [`open`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html#method-i-open)
# must have the following methods:
#
# send\_request (ref, msg\_id, arg, b)
# :   Send a request to `ref` with the given message id and arguments. This is
#     most easily implemented by calling DRbMessage.send\_request, providing a
#     stream that sits on top of the current protocol.
# recv\_reply
# :   Receive a reply from the server and return it as a [success-boolean,
#     reply-value] pair. This is most easily implemented by calling
#     DRb.recv\_reply, providing a stream that sits on top of the current
#     protocol.
# alive?
# :   Is this connection still alive?
# close
# :   Close this connection.
#
#
# The protocol instance returned by
# [`open_server()`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html#method-i-open_server).accept()
# must have the following methods:
#
# recv\_request
# :   Receive a request from the client and return a [object, message, args,
#     block] tuple. This is most easily implemented by calling
#     DRbMessage.recv\_request, providing a stream that sits on top of the
#     current protocol.
# send\_reply(succ, result)
# :   Send a reply to the client. This is most easily implemented by calling
#     DRbMessage.send\_reply, providing a stream that sits on top of the current
#     protocol.
# close
# :   Close this connection.
#
#
# A new protocol is registered with the
# [`DRbProtocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html)
# module using the
# [`add_protocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html#method-c-add_protocol)
# method.
#
# For examples of other protocols, see DRbUNIXSocket in drb/unix.rb, and HTTP0
# in sample/http0.rb and sample/http0serv.rb in the full drb distribution.
module DRb::DRbProtocol
  # Add a new protocol to the
  # [`DRbProtocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html)
  # module.
  sig { params(prot: T.nilable(Object)).returns(T::Array[T.nilable(Object)]) }
  def self.add_protocol(prot); end

  # Open a client connection to `uri` with the configuration `config`.
  #
  # The
  # [`DRbProtocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html)
  # module asks each registered protocol in turn to try to open the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html). Each protocol signals
  # that it does not handle that
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) by raising a
  # DRbBadScheme error. If no protocol recognises the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html), then a DRbBadURI
  # error is raised. If a protocol accepts the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html), but an error occurs
  # in opening it, a DRbConnError is raised.
  sig do
    params(
      uri: String,
      config: T::Hash[Symbol, T.untyped],
      first: T::Boolean
    ).returns(DRb::DRbTCPSocket)
  end
  def self.open(uri, config, first = true); end

  # Open a server listening for connections at `uri` with configuration
  # `config`.
  #
  # The
  # [`DRbProtocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html)
  # module asks each registered protocol in turn to try to open a server at the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html). Each protocol signals
  # that it does not handle that
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) by raising a
  # DRbBadScheme error. If no protocol recognises the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html), then a DRbBadURI
  # error is raised. If a protocol accepts the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html), but an error occurs
  # in opening it, the underlying error is passed on to the caller.
  sig do
    params(
      uri: String,
      config: T::Hash[Symbol, T.untyped],
      first: T::Boolean
    ).returns(DRb::DRbTCPSocket)
  end
  def self.open_server(uri, config, first = true); end

  # Parse `uri` into a [uri, option] pair.
  #
  # The
  # [`DRbProtocol`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbProtocol.html)
  # module asks each registered protocol in turn to try to parse the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html). Each protocol signals
  # that it does not handle that
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) by raising a
  # DRbBadScheme error. If no protocol recognises the
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html), then a DRbBadURI
  # error is raised.
  sig do
    params(
      uri: String,
      config: T::Hash[Symbol, T.untyped],
      first: T::Boolean
    ).returns([String, T.nilable(Object)])
  end
  def self.uri_option(uri, config, first = _); end
end

# An exception wrapping an error object
class DRb::DRbRemoteError < ::DRb::DRbError
  # the class of the error, as a string.
  sig { returns(String) }
  def reason; end

  # Creates a new remote error that wraps the
  # [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) `error`
  sig { params(error: Exception).returns(DRb::DRbRemoteError) }
  def self.new(error); end
end

# [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) representing a drb
# server instance.
#
# A [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html) must
# be running in the local process before any incoming dRuby calls can be
# accepted, or any local objects can be passed as dRuby references to remote
# processes, even if those local objects are never actually called remotely. You
# do not need to start a
# [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html) in the
# local process if you are only making outgoing dRuby calls passing marshalled
# parameters.
#
# Unless multiple servers are being used, the local
# [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html) is
# normally started by calling
# [`DRb.start_service`](https://docs.ruby-lang.org/en/2.6.0/DRb.html#method-c-start_service).
class DRb::DRbServer
  # Is this server alive?
  sig { returns(T::Boolean) }
  def alive?; end

  # Check that a method is callable via dRuby.
  #
  # `obj` is the object we want to invoke the method on. `msg_id` is the method
  # name, as a [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html).
  #
  # If the method is an insecure method (see
  # [`insecure_method?`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html#method-i-insecure_method-3F))
  # a [`SecurityError`](https://docs.ruby-lang.org/en/2.6.0/SecurityError.html)
  # is thrown. If the method is private or undefined, a
  # [`NameError`](https://docs.ruby-lang.org/en/2.6.0/NameError.html) is thrown.
  sig do
    params(
      obj: T.nilable(Object),
      msg_id: T.any(String, Symbol)
    ).returns(T::Boolean)
  end
  def check_insecure_method(obj, msg_id); end

  # The configuration of this
  # [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html)
  sig { returns(T::Hash[Symbol, T.untyped]) }
  def config; end

  # The front object of the
  # [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html).
  #
  # This object receives remote method calls made on the server's
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) alone, with an object
  # id.
  sig { returns(T.nilable(Object)) }
  def front; end

  # Is `uri` the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) for this
  # server?
  sig { params(uri: String).returns(T::Boolean) }
  def here?(uri); end

  # Stop this server.
  def stop_service; end

  # The main thread of this
  # [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html).
  #
  # This is the thread that listens for and accepts connections from clients,
  # not that handles each client's request-response session.
  sig { returns(Thread) }
  def thread; end

  # Convert a local object to a dRuby reference.
  sig { params(obj: T.nilable(Object)).returns(T.nilable(Integer)) }
  def to_id(obj); end

  # Convert a dRuby reference to the local object it refers to.
  sig { params(ref: T.nilable(Integer)).returns(T.nilable(Object)) }
  def to_obj(ref); end

  # The [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) of this
  # [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html).
  sig { returns(String) }
  def uri; end

  # Get whether the server is in verbose mode.
  #
  # In verbose mode, failed calls are logged to stdout.
  sig { returns(T::Boolean) }
  def verbose; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) whether to operate in
  # verbose mode.
  #
  # In verbose mode, failed calls are logged to stdout.
  sig { params(v: T::Boolean).returns(T::Boolean) }
  def verbose=(v); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default access
  # control list to `acl`. The default
  # [`ACL`](https://docs.ruby-lang.org/en/2.6.0/ACL.html) is `nil`.
  #
  # See also DRb::ACL and new()
  sig { params(acl: T.nilable(Object)).returns(T.nilable(Object)) }
  def self.default_acl(acl); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default value for
  # the :argc\_limit option.
  #
  # See new(). The initial default value is 256.
  sig { params(argc: T.nilable(Object)).returns(T.nilable(Object)) }
  def self.default_argc_limit(argc); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default value for
  # the :id\_conv option.
  #
  # See new(). The initial default value is a DRbIdConv instance.
  sig { params(idconv: T.nilable(Object)).returns(T.nilable(Object)) }
  def self.default_id_conv(idconv); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default value for
  # the :load\_limit option.
  #
  # See new(). The initial default value is 25 MB.
  sig { params(sz: T.nilable(Object)).returns(T.nilable(Object)) }
  def self.default_load_limit(sz); end

  # Create a new
  # [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html)
  # instance.
  #
  # `uri` is the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) to bind
  # to. This is normally of the form 'druby://<hostname>:<port>' where
  # <hostname> is a hostname of the local machine. If nil, then the system's
  # default hostname will be bound to, on a port selected by the system; these
  # value can be retrieved from the `uri` attribute. 'druby:' specifies the
  # default dRuby transport protocol: another protocol, such as 'drbunix:', can
  # be specified instead.
  #
  # `front` is the front object for the server, that is, the object to which
  # remote method calls on the server will be passed. If nil, then the server
  # will not accept remote method calls.
  #
  # If `config_or_acl` is a hash, it is the configuration to use for this
  # server. The following options are recognised:
  #
  # :idconv
  # :   an id-to-object conversion object. This defaults to an instance of the
  #     class
  #     [`DRb::DRbIdConv`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbIdConv.html).
  # :verbose
  # :   if true, all unsuccessful remote calls on objects in the server will be
  #     logged to $stdout. false by default.
  # :tcp\_acl
  # :   the access control list for this server. See the
  #     [`ACL`](https://docs.ruby-lang.org/en/2.6.0/ACL.html) class from the
  #     main dRuby distribution.
  # :load\_limit
  # :   the maximum message size in bytes accepted by the server. Defaults to 25
  #     MB (26214400).
  # :argc\_limit
  # :   the maximum number of arguments to a remote method accepted by the
  #     server. Defaults to 256.
  # :safe\_level
  # :   The safe level of the
  #     [`DRbServer`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html).
  #     The attribute sets $SAFE for methods performed in the main\_loop.
  #     Defaults to 0.
  #
  #
  # The default values of these options can be modified on a class-wide basis by
  # the class methods default\_argc\_limit, default\_load\_limit, default\_acl,
  # default\_id\_conv, and
  # [`verbose=`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbServer.html#method-i-verbose-3D)
  #
  # If `config_or_acl` is not a hash, but is not nil, it is assumed to be the
  # access control list for this server. See the :tcp\_acl option for more
  # details.
  #
  # If no other server is currently set as the primary server, this will become
  # the primary server.
  #
  # The server will immediately start running in its own thread.
  sig do
    params(
      uri: String,
      front: T.nilable(Object),
      config_or_acl: T.nilable(Object)
    ).returns(DRb::DRbServer)
  end
  def self.new(uri, front, config_or_acl); end

  # Get the default value of the :verbose option.
  sig { returns(T::Boolean) }
  def self.verbose; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default value of
  # the :verbose option.
  #
  # See new(). The initial default value is false.
  sig { params(on: T::Boolean).returns(T::Boolean) }
  def self.verbose=(on); end
end

# List of insecure methods.
#
# These methods are not callable via dRuby.
DRb::DRbServer::INSECURE_METHOD = T.let(T.unsafe(nil), Array)

# Error raised by the [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html)
# module when an attempt is made to refer to the context's current drb server
# but the context does not have one. See current\_server.
class DRb::DRbServerNotFound < ::DRb::DRbError; end

# The default drb protocol which communicates over a TCP socket.
#
# The [`DRb`](https://docs.ruby-lang.org/en/2.6.0/DRb.html) TCP protocol
# [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) looks like:
# `druby://<host>:<port>?<option>`. The option is optional.
class DRb::DRbTCPSocket; end

# Mixin module making an object undumpable or unmarshallable.
#
# If an object which includes this module is returned by method called over drb,
# then the object remains in the server space and a reference to the object is
# returned, rather than the object being marshalled and moved into the client
# space.
module DRb::DRbUndumped; end

# [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) wrapping a
# marshalled object whose type is unknown locally.
#
# If an object is returned by a method invoked over drb, but the class of the
# object is unknown in the client namespace, or the object is a constant unknown
# in the client namespace, then the still-marshalled object is returned wrapped
# in a [`DRbUnknown`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUnknown.html)
# instance.
#
# If this object is passed as an argument to a method invoked over drb, then the
# wrapped object is passed instead.
#
# The class or constant name of the object can be read from the `name`
# attribute. The marshalled object is held in the `buf` attribute.
class DRb::DRbUnknown
  # Create a new
  # [`DRb::DRbUnknown`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUnknown.html)
  # object.
  #
  # `buf` is a string containing a marshalled object that could not be unmarshalled.
  # `err` is the error message that was raised when the unmarshalling failed.
  # It is used to determine the name of the unmarshalled object.
  sig { params(err: String, buf: IO).void }
  def initialize(err, buf); end

  # Buffer contained the marshalled, unknown object.
  sig { returns(IO) }
  def buf; end

  # Create a DRbUnknownError exception containing this object.
  sig { returns(DRb::DRbUnknownError) }
  def exception; end

  # The name of the unknown thing.
  #
  # [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) name for unknown
  # objects; variable name for unknown constants.
  sig { returns(T.nilable(T.any(String, Symbol))) }
  def name; end

  # Attempt to load the wrapped marshalled object again.
  #
  # If the class of the object is now known locally, the object will be
  # unmarshalled and returned. Otherwise, a new but identical
  # [`DRbUnknown`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUnknown.html)
  # object will be returned.
  sig { returns(T.nilable(Object)) }
  def reload; end
end

# An exception wrapping a
# [`DRb::DRbUnknown`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUnknown.html)
# object
class DRb::DRbUnknownError < ::DRb::DRbError
  # Get the wrapped
  # [`DRb::DRbUnknown`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUnknown.html)
  # object.
  sig { returns(DRb::DRbUnknown) }
  def unknown; end

  # Create a new
  # [`DRbUnknownError`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUnknownError.html)
  # for the
  # [`DRb::DRbUnknown`](https://docs.ruby-lang.org/en/2.6.0/DRb/DRbUnknown.html)
  # object `unknown`
  sig { params(unknown: DRb::DRbUnknown).returns(DRb::DRbUnknownError) }
  def self.new(unknown); end
end
