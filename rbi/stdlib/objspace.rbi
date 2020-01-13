# typed: __STDLIB_INTERNAL

# The objspace library extends the
# [`ObjectSpace`](https://docs.ruby-lang.org/en/2.6.0/ObjectSpace.html) module
# and adds several methods to get internal statistic information about
# object/memory management.
#
# You need to `require 'objspace'` to use this extension module.
#
# Generally, you \*SHOULD NOT\* use this library if you do not know about the
# MRI implementation. Mainly, this library is for (memory) profiler developers
# and MRI developers who need to know about MRI memory usage.
# The [`ObjectSpace`](https://docs.ruby-lang.org/en/2.6.0/ObjectSpace.html)
# module contains a number of routines that interact with the garbage collection
# facility and allow you to traverse all living objects with an iterator.
#
# [`ObjectSpace`](https://docs.ruby-lang.org/en/2.6.0/ObjectSpace.html) also
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

  # Calls the block once for each living, nonimmediate object in this Ruby
  # process. If *module* is specified, calls the block for only those classes or
  # modules that match (or are a subclass of) *module*. Returns the number of
  # objects found. Immediate objects (`Fixnum`s, `Symbol`s `true`, `false`, and
  # `nil`) are never returned. In the example below, `each_object` returns both
  # the numbers we defined and several constants defined in the `Math` module.
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
end

# An
# [`ObjectSpace::WeakMap`](https://docs.ruby-lang.org/en/2.6.0/ObjectSpace/WeakMap.html)
# object holds references to any objects, but those objects can get garbage
# collected.
#
# This class is mostly used internally by
# [`WeakRef`](https://docs.ruby-lang.org/en/2.6.0/WeakRef.html), please use
# `lib/weakref.rb` for the public interface.
class ObjectSpace::WeakMap < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end
