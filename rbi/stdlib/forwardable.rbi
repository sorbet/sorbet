# typed: __STDLIB_INTERNAL

# The [`Forwardable`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html)
# module provides delegation of specified methods to a designated object, using
# the methods
# [`def_delegator`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-def_delegator)
# and
# [`def_delegators`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-def_delegators).
#
# For example, say you have a class RecordCollection which contains an array
# `@records`. You could provide the lookup method record\_number(), which simply
# calls [] on the `@records` array, like this:
#
# ```ruby
# require 'forwardable'
#
# class RecordCollection
#   attr_accessor :records
#   extend Forwardable
#   def_delegator :@records, :[], :record_number
# end
# ```
#
# We can use the lookup method like so:
#
# ```ruby
# r = RecordCollection.new
# r.records = [4,5,6]
# r.record_number(0)  # => 4
# ```
#
# Further, if you wish to provide the methods size, <<, and map, all of which
# delegate to @records, this is how you can do it:
#
# ```ruby
# class RecordCollection # re-open RecordCollection class
#   def_delegators :@records, :size, :<<, :map
# end
#
# r = RecordCollection.new
# r.records = [1,2,3]
# r.record_number(0)   # => 1
# r.size               # => 3
# r << 4               # => [1, 2, 3, 4]
# r.map { |x| x * 2 }  # => [2, 4, 6, 8]
# ```
#
# You can even extend regular objects with
# [`Forwardable`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html).
#
# ```ruby
# my_hash = Hash.new
# my_hash.extend Forwardable              # prepare object for delegation
# my_hash.def_delegator "STDOUT", "puts"  # add delegation for STDOUT.puts()
# my_hash.puts "Howdy!"
# ```
#
# ## Another example
#
# We want to rely on what has come before obviously, but with delegation we can
# take just the methods we need and even rename them as appropriate. In many
# cases this is preferable to inheritance, which gives us the entire old
# interface, even if much of it isn't needed.
#
# ```ruby
# class Queue
#   extend Forwardable
#
#   def initialize
#     @q = [ ]    # prepare delegate object
#   end
#
#   # setup preferred interface, enq() and deq()...
#   def_delegator :@q, :push, :enq
#   def_delegator :@q, :shift, :deq
#
#   # support some general Array methods that fit Queues well
#   def_delegators :@q, :clear, :first, :push, :shift, :size
# end
#
# q = Queue.new
# q.enq 1, 2, 3, 4, 5
# q.push 6
#
# q.shift    # => 1
# while q.size > 0
#   puts q.deq
# end
#
# q.enq "Ruby", "Perl", "Python"
# puts q.first
# q.clear
# puts q.first
# ```
#
# This should output:
#
# ```ruby
# 2
# 3
# 4
# 5
# 6
# Ruby
# nil
# ```
#
# ## Notes
#
# Be advised, [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) will not
# detect delegated methods.
#
# `forwardable.rb` provides single-method delegation via the
# [`def_delegator`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-def_delegator)
# and
# [`def_delegators`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-def_delegators)
# methods. For full-class delegation via DelegateClass, see `delegate.rb`.
module Forwardable
  FILTER_EXCEPTION = T.let(T.unsafe(nil), String)
  FORWARDABLE_VERSION = T.let(T.unsafe(nil), String)

  # Alias for:
  # [`def_instance_delegator`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-def_instance_delegator)
  sig { params(accessor: T.any(Symbol, String), method: Symbol, ali: Symbol).returns(Symbol) }
  def def_delegator(accessor, method, ali = method); end

  # Alias for:
  # [`def_instance_delegators`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-def_instance_delegators)
  sig { params(accessor: T.any(Symbol, String), methods: Symbol).returns(T::Array[Symbol]) }
  def def_delegators(accessor, *methods); end

  # Define `method` as delegator instance method with an optional alias name
  # `ali`. [`Method`](https://docs.ruby-lang.org/en/2.6.0/Method.html) calls to
  # `ali` will be delegated to `accessor.method`.
  #
  # ```ruby
  # class MyQueue
  #   extend Forwardable
  #   attr_reader :queue
  #   def initialize
  #     @queue = []
  #   end
  #
  #   def_delegator :@queue, :push, :mypush
  # end
  #
  # q = MyQueue.new
  # q.mypush 42
  # q.queue    #=> [42]
  # q.push 23  #=> NoMethodError
  # ```
  #
  #
  # Also aliased as:
  # [`def_delegator`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-def_delegator)
  sig { params(accessor: T.any(Symbol, String), method: Symbol, ali: Symbol).returns(Symbol) }
  def def_instance_delegator(accessor, method, ali = method); end

  # Shortcut for defining multiple delegator methods, but with no provision for
  # using a different name. The following two code samples have the same effect:
  #
  # ```ruby
  # def_delegators :@records, :size, :<<, :map
  #
  # def_delegator :@records, :size
  # def_delegator :@records, :<<
  # def_delegator :@records, :map
  # ```
  #
  #
  # Also aliased as:
  # [`def_delegators`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-def_delegators)
  sig { params(accessor: T.any(Symbol, String), methods: Symbol).returns(T::Array[Symbol]) }
  def def_instance_delegators(accessor, *methods); end

  # Alias for:
  # [`instance_delegate`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-instance_delegate)
  sig do
    params(
      hash: T::Hash[T.any(Symbol, T::Array[Symbol]), Symbol]
    ).returns(T.any(Symbol, T::Array[Symbol]))
  end
  def delegate(hash); end

  # Takes a hash as its argument. The key is a symbol or an array of symbols.
  # These symbols correspond to method names. The value is the accessor to which
  # the methods will be delegated.
  #
  # Also aliased as:
  # [`delegate`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html#method-i-delegate)
  sig do
    params(
      hash: T::Hash[T.any(Symbol, T::Array[Symbol]), Symbol]
    ).returns(T.any(Symbol, T::Array[Symbol]))
  end
  def instance_delegate(hash); end
end

# [`SingleForwardable`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html)
# can be used to setup delegation at the object level as well.
#
# ```ruby
# printer = String.new
# printer.extend SingleForwardable        # prepare object for delegation
# printer.def_delegator "STDOUT", "puts"  # add delegation for STDOUT.puts()
# printer.puts "Howdy!"
# ```
#
# Also,
# [`SingleForwardable`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html)
# can be used to set up delegation for a
# [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) or
# [`Module`](https://docs.ruby-lang.org/en/2.6.0/Module.html).
#
# ```ruby
# class Implementation
#   def self.service
#     puts "serviced!"
#   end
# end
#
# module Facade
#   extend SingleForwardable
#   def_delegator :Implementation, :service
# end
#
# Facade.service #=> serviced!
# ```
#
# If you want to use both
# [`Forwardable`](https://docs.ruby-lang.org/en/2.6.0/Forwardable.html) and
# [`SingleForwardable`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html),
# you can use methods def\_instance\_delegator and
# [`def_single_delegator`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html#method-i-def_single_delegator),
# etc.
module SingleForwardable
  # Alias for:
  # [`def_single_delegator`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html#method-i-def_single_delegator)
  sig { params(accessor: T.any(Symbol, String), method: Symbol, ali: Symbol).returns(Symbol) }
  def def_delegator(accessor, method, ali = method); end

  # Alias for:
  # [`def_single_delegators`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html#method-i-def_single_delegators)
  sig { params(accessor: T.any(Symbol, String), methods: Symbol).returns(T::Array[Symbol]) }
  def def_delegators(accessor, *methods); end

  # Defines a method *method* which delegates to *accessor* (i.e. it calls the
  # method of the same name in *accessor*). If *new\_name* is provided, it is
  # used as the name for the delegate method.
  #
  # Also aliased as:
  # [`def_delegator`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html#method-i-def_delegator)
  sig { params(accessor: T.any(Symbol, String), method: Symbol, new_name: Symbol).returns(Symbol) }
  def def_single_delegator(accessor, method, new_name = method); end

  # Shortcut for defining multiple delegator methods, but with no provision for
  # using a different name. The following two code samples have the same effect:
  #
  # ```ruby
  # def_delegators :@records, :size, :<<, :map
  #
  # def_delegator :@records, :size
  # def_delegator :@records, :<<
  # def_delegator :@records, :map
  # ```
  #
  #
  # Also aliased as:
  # [`def_delegators`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html#method-i-def_delegators)
  sig { params(accessor: T.any(Symbol, String), methods: Symbol).returns(T::Array[Symbol]) }
  def def_single_delegators(accessor, *methods); end

  # Alias for:
  # [`single_delegate`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html#method-i-single_delegate)
  sig do
    params(
      hash: T::Hash[T.any(Symbol, T::Array[Symbol]), Symbol]
    ).returns(T.any(Symbol, T::Array[Symbol]))
  end
  def delegate(hash); end

  # Takes a hash as its argument. The key is a symbol or an array of symbols.
  # These symbols correspond to method names. The value is the accessor to which
  # the methods will be delegated.
  #
  # Also aliased as:
  # [`delegate`](https://docs.ruby-lang.org/en/2.6.0/SingleForwardable.html#method-i-delegate)
  sig do
    params(
      hash: T::Hash[T.any(Symbol, T::Array[Symbol]), Symbol]
    ).returns(T.any(Symbol, T::Array[Symbol]))
  end
  def single_delegate(hash); end
end
