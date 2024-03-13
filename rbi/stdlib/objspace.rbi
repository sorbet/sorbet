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

  # Counts objects size (in bytes) for each type.
  # Note that this information is incomplete. You need to deal with this information as only a HINT. Especially, total size of T_DATA may be wrong.
  # It returns a hash as:
  # {:TOTAL=>1461154, :T_CLASS=>158280, :T_MODULE=>20672, :T_STRING=>527249, ...}
  # If the optional argument, result_hash, is given, it is overwritten and returned. This is intended to avoid probe effect.
  # The contents of the returned hash is implementation defined. It may be changed in future.
  # This method is only expected to work with C Ruby.
  sig { params(result_hash: T.nilable(T::Hash[T.untyped, T.untyped])).returns(T::Hash[T.untyped, T.untyped]) }
  sig { returns(T::Hash[Symbol, Integer])}
  def self.count_objects_size(result_hash=nil); end

  # Counts objects for each T_IMEMO type.
  # This method is only for MRI developers interested in performance and memory usage of Ruby programs.
  # It returns a hash as:
  # {:imemo_ifunc=>8,
  # :imemo_svar=>7,
  # :imemo_cref=>509,
  # :imemo_memo=>1,
  # :imemo_throw_data=>1}
  # If the optional argument, result_hash, is given, it is overwritten and returned. This is intended to avoid probe effect.
  # The contents of the returned hash is implementation specific and may change in the future.
  # In this version, keys are symbol objects.
  # This method is only expected to work with C Ruby.
  sig { params(result_hash: T.nilable(T::Hash[T.untyped, T.untyped])).returns(T::Hash[T.untyped, T.untyped]) }
  sig { returns(T::Hash[Symbol, Integer])}
  def self.count_imemo_objects(result_hash=nil); end

  # Counts objects for each T_DATA type.
  # This method is only for MRI developers interested in performance and memory usage of Ruby programs.
  # It returns a hash as:
  # {RubyVM::InstructionSequence=>504, :parser=>5, :barrier=>6,
  # :mutex=>6, Proc=>60, RubyVM::Env=>57, Mutex=>1, Encoding=>99,
  # ThreadGroup=>1, Binding=>1, Thread=>1, RubyVM=>1, :iseq=>1,
  # Random=>1, ARGF.class=>1, Data=>1, :autoload=>3, Time=>2}
  # # T_DATA objects existing at startup on r32276.
  # If the optional argument, result_hash, is given, it is overwritten and returned. This is intended to avoid probe effect.
  # The contents of the returned hash is implementation specific and may change in the future.
  # In this version, keys are Class object or Symbol object.
  # If object is kind of normal (accessible) object, the key is Class object. If object is not a kind of normal (internal) object, the key is symbol name, registered by rb_data_type_struct.
  # This method is only expected to work with C Ruby.
  sig { params(result_hash: T.nilable(T::Hash[T.untyped, T.untyped])).returns(T::Hash[T.untyped, T.untyped]) }
  sig { returns(T::Hash[T.any(T::Class[T.anything], Symbol), Integer])}
  def self.count_tdata_objects(result_hash=nil); end

  # Return consuming memory size of obj.
  # Note that the return size is incomplete. You need to deal with this information as only a HINT. Especially, the size of T_DATA may not be correct.
  # This method is only expected to work with C Ruby.
  # From Ruby 2.2, ::memsize_of(obj) returns a memory size includes sizeof(RVALUE).
  sig { params(klass: Object).returns(Integer) }
  def self.memsize_of(klass); end

  # Return consuming memory size of all living objects.
  # If klass (should be Class object) is given, return the total memory size of instances of the given class.
  # Note that the returned size is incomplete. You need to deal with this information as only a HINT. Especially, the size of T_DATA may not be correct.
  # Note that this method does NOT return total malloc’ed memory size.
  # This method can be defined by the following Ruby code:
  # def memsize_of_all klass = false
  #   total = 0
  #   ObjectSpace.each_object{|e|
  #     total += ObjectSpace.memsize_of(e) if klass == false || e.kind_of?(klass)
  #   }
  #   total
  # end
  # This method is only expected to work with C Ruby.
  sig { params(klass: T::Class[T.anything]).returns(Integer) }
  def self.memsize_of_all(klass=nil); end
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

# An
# [`ObjectSpace::WeakKeyMap`](https://docs.ruby-lang.org/en/3.3/ObjectSpace/WeakKeyMap.html)
# is a key-value map that holds weak references to its keys, so they can be garbage collected 
# when there is no more references.
# 
# Unlike ObjectSpace::WeakMap:
#  - references to values are strong, so they aren’t garbage collected while they are in the map;
#  - keys are compared by value (using Object#eql?), not by identity;
#  - only garbage-collectable objects can be used as keys.
#  
#    map = ObjectSpace::WeakKeyMap.new
#    val = Time.new(2023, 12, 7)
#    key = "name"
#    map[key] = val
#
#    # Value is fetched by equality: the instance of string "name" is
#    # different here, but it is equal to the key
#    map["name"] #=> 2023-12-07 00:00:00 +0200
#
#    val = nil
#    GC.start
#    # There is no more references to `val`, yet the pair isn't
#    # garbage-collected.
#    map["name"] #=> 2023-12-07 00:00:00 +0200
#
#    key = nil
#    GC.start
#    # There is no more references to `key`, key and value are
#    # garbage-collected.
#   
#    map["name"] #=> nil
#
# (Note that GC.start is used here only for demonstrational purposes and might not always lead to demonstrated results.)
#
# The collection is especially useful for implementing caches of lightweight value objects, so that only one copy of 
# each value representation would be stored in memory, but the copies that aren’t used would be garbage-collected.
#
#   CACHE = ObjectSpace::WeakKeyMap
# 
#   def make_value(**)
#     val = ValueObject.new(**)
#     if (existing = @cache.getkey(val))
#        # if the object with this value exists, we return it
#        existing
#     else
#        # otherwise, put it in the cache
#        @cache[val] = true
#        val
#     end
#  end
#
# This will result in make_value returning the same object for same set of attributes always, but the values that
# aren’t needed anymore woudn’t be sitting in the cache forever.
class ObjectSpace::WeakKeyMap < Object
  
  # Returns the value associated with the given key if found
  # If key is not found, returns nil
  def [](_); end
  
  # Associates the given value with the given key
  # The reference to key is weak, so when there is no other reference to key it may be garbage collected
  # If the given key exists, replaces its value with the given value; the ordering is not affected
  def []=(_,_); end
  
  # Removes all map entries; returns self.
  sig {returns(T.self_type)}
  def clear; end
  
  # Deletes the entry for the given key and returns its associated value
  # If no block is given and key is found, deletes the entry and returns the associated value
  def delete(_); end
  
  # Returns the existing equal key if it exists, otherwise returns nil.
  # This might be useful for implementing caches, so that only one copy of some object would be used everywhere in the program
  def getkey(_); end
  
  # Returns a new String containing informations about the map
  sig {returns(String)}
  def inspect; end
  
  # Returns true if key is a key in self, otherwise false.
  sig {params(key: T.any).returns(T::Boolean)}
  def key?(key); end
end
