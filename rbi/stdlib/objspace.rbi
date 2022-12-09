# typed: __STDLIB_INTERNAL

# The objspace library extends the
# [`ObjectSpace`](https://docs.ruby-lang.org/en/2.7.0/ObjectSpace.html) module
# and adds several methods to get internal statistic information about
# object/memory management.
#
# You need to `require 'objspace'` to use this extension module.
#
# Generally, you \*SHOULD NOT\* use this library if you do not know about the
# MRI implementation. Mainly, this library is for (memory) profiler developers
# and MRI developers who need to know about MRI memory usage.
# The [`ObjectSpace`](https://docs.ruby-lang.org/en/2.7.0/ObjectSpace.html)
# module contains a number of routines that interact with the garbage collection
# facility and allow you to traverse all living objects with an iterator.
#
# [`ObjectSpace`](https://docs.ruby-lang.org/en/2.7.0/ObjectSpace.html) also
# provides support for object finalizers, procs that will be called when a
# specific object is about to be destroyed by garbage collection.
#
# ```ruby
# require 'objspace'
#
# a = "A"
# b = "B"
#
# ObjectSpace.define_finalizer(a, proc {|id| puts "Finalizer one on #{id}" })
# ObjectSpace.define_finalizer(b, proc {|id| puts "Finalizer two on #{id}" })
# ```
#
# *produces:*
#
# ```ruby
# Finalizer two on 537763470
# Finalizer one on 537763480
# ```
module ObjectSpace
  # Converts an object id to a reference to the object. May not be called on an
  # object id passed as a parameter to a finalizer.
  #
  # ```ruby
  # s = "I am a string"                    #=> "I am a string"
  # r = ObjectSpace._id2ref(s.object_id)   #=> "I am a string"
  # r == s                                 #=> true
  # ```
  sig {params(object_id: Integer).returns(T.untyped)}
  def self._id2ref(object_id); end

  # Counts all objects grouped by type.
  #
  # It returns a hash, such as:
  #
  # ```ruby
  # {
  #   :TOTAL=>10000,
  #   :FREE=>3011,
  #   :T_OBJECT=>6,
  #   :T_CLASS=>404,
  #   # ...
  # }
  # ```
  #
  # The contents of the returned hash are implementation specific. It may be
  # changed in future.
  #
  # The keys starting with `:T_` means live objects. For example, `:T_ARRAY` is
  # the number of arrays. `:FREE` means object slots which is not used now.
  # `:TOTAL` means sum of above.
  #
  # If the optional argument `result_hash` is given, it is overwritten and
  # returned. This is intended to avoid probe effect.
  #
  # ```ruby
  # h = {}
  # ObjectSpace.count_objects(h)
  # puts h
  # # => { :TOTAL=>10000, :T_CLASS=>158280, :T_MODULE=>20672, :T_STRING=>527249 }
  # ```
  #
  # This method is only expected to work on C Ruby.
  def self.count_objects(*_); end

  # Adds *aProc* as a finalizer, to be called after *obj* was destroyed. The
  # object ID of the *obj* will be passed as an argument to *aProc*. If *aProc*
  # is a lambda or method, make sure it can be called with a single argument.
  def self.define_finalizer(*_); end

  # Calls the block once for each living, nonimmediate object in this Ruby
  # process. If *module* is specified, calls the block for only those classes or
  # modules that match (or are a subclass of) *module*. Returns the number of
  # objects found. Immediate objects (`Fixnum`s, `Symbol`s `true`, `false`, and
  # `nil`) are never returned. In the example below, each\_object returns both
  # the numbers we defined and several constants defined in the
  # [`Math`](https://docs.ruby-lang.org/en/2.7.0/Math.html) module.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = 102.7
  # b = 95       # Won't be returned
  # c = 12345678987654321
  # count = ObjectSpace.each_object(Numeric) {|x| p x }
  # puts "Total count: #{count}"
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # 12345678987654321
  # 102.7
  # 2.71828182845905
  # 3.14159265358979
  # 2.22044604925031e-16
  # 1.7976931348623157e+308
  # 2.2250738585072e-308
  # Total count: 7
  # ```
  sig {params(mod: Module).returns(T::Enumerator[BasicObject])}
  sig {params(mod: Module, blk: T.proc.params(obj: BasicObject).void).returns(Integer)}
  def self.each_object(mod=BasicObject, &blk)
  end

  # Initiates garbage collection, unless manually disabled.
  #
  # This method is defined with keyword arguments that default to true:
  #
  # ```ruby
  # def GC.start(full_mark: true, immediate_sweep: true); end
  # ```
  #
  # Use full\_mark: false to perform a minor
  # [`GC`](https://docs.ruby-lang.org/en/2.6.0/GC.html). Use immediate\_sweep:
  # false to defer sweeping (use lazy sweep).
  #
  # Note: These keyword arguments are implementation and version dependent. They
  # are not guaranteed to be future-compatible, and may be ignored if the
  # underlying implementation does not support them.
  def self.garbage_collect(*_); end

  # Removes all finalizers for *obj*.
  def self.undefine_finalizer(_); end

  # Starts tracing object allocations.
  def self.trace_object_allocations_start; end

  # Stop tracing object allocations.
  #
  # Note that if
  # [`::trace_object_allocations_start`](https://docs.ruby-lang.org/en/2.7.0/ObjectSpace.html#method-c-trace_object_allocations_start)
  # is called n-times, then tracing will stop after calling
  # [`::trace_object_allocations_stop`](https://docs.ruby-lang.org/en/2.7.0/ObjectSpace.html#method-c-trace_object_allocations_stop)
  # n-times.
  def self.trace_object_allocations_stop; end

  # Clear recorded tracing information.
  def self.trace_object_allocations_clear; end
end

# An
# [`ObjectSpace::WeakMap`](https://docs.ruby-lang.org/en/2.7.0/ObjectSpace/WeakMap.html)
# object holds references to any objects, but those objects can get garbage
# collected.
#
# This class is mostly used internally by
# [`WeakRef`](https://docs.ruby-lang.org/en/2.7.0/WeakRef.html), please use
# `lib/weakref.rb` for the public interface.
class ObjectSpace::WeakMap < Object
  include Enumerable

  extend T::Generic
  Elem = type_member {{ fixed: T.untyped }}

  # Retrieves a weakly referenced object with the given key
  def [](_); end

  # Creates a weak reference from the given key to the given value
  def []=(_, _); end

  # Iterates over keys and objects in a weakly referenced object
  def each; end

  # Iterates over keys and objects in a weakly referenced object
  def each_key; end

  # Iterates over keys and objects in a weakly referenced object
  def each_pair; end

  # Iterates over keys and objects in a weakly referenced object
  def each_value; end

  # Returns `true` if `key` is registered
  def include?(_); end

  def inspect; end

  # Returns `true` if `key` is registered
  def key?(_); end

  # Iterates over keys and objects in a weakly referenced object
  def keys; end

  # Returns the number of referenced objects
  def length; end

  # Returns `true` if `key` is registered
  def member?(_); end

  # Returns the number of referenced objects
  def size; end

  # Iterates over values and objects in a weakly referenced object
  def values; end

end
