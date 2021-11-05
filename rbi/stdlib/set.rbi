# typed: __STDLIB_INTERNAL

# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) implements a collection
# of unordered values with no duplicates. This is a hybrid of Array's intuitive
# inter-operation facilities and Hash's fast lookup.
#
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) is easy to use with
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) objects
# (implementing `each`). Most of the initializer methods and binary operators
# accept generic
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) objects
# besides sets and arrays. An
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) object can
# be converted to [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) using
# the `to_set` method.
#
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) uses
# [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) as storage, so you
# must note the following points:
#
# *   Equality of elements is determined according to
#     [`Object#eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F)
#     and
#     [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash).
#     Use
#     [`Set#compare_by_identity`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-compare_by_identity)
#     to make a set compare its elements by their identity.
# *   [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) assumes that the
#     identity of each element does not change while it is stored. Modifying an
#     element of a set will render the set to an unreliable state.
# *   When a string is to be stored, a frozen copy of the string is stored
#     instead unless the original string is already frozen.
#
#
# ## Comparison
#
# The comparison operators <, >, <=, and >= are implemented as shorthand for the
# {proper\_,}{subset?,superset?} methods. However, the <=> operator is
# intentionally left out because not every pair of sets is comparable ({x, y}
# vs. {x, z} for example).
#
# ## Example
#
# ```ruby
# require 'set'
# s1 = Set[1, 2]                        #=> #<Set: {1, 2}>
# s2 = [1, 2].to_set                    #=> #<Set: {1, 2}>
# s1 == s2                              #=> true
# s1.add("foo")                         #=> #<Set: {1, 2, "foo"}>
# s1.merge([2, 6])                      #=> #<Set: {1, 2, "foo", 6}>
# s1.subset?(s2)                        #=> false
# s2.subset?(s1)                        #=> true
# ```
#
# ## Contact
#
# ```
# - Akinori MUSHA <knu@iDaemons.org> (current maintainer)
# ```
class Set < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  # Creates a new set containing the given objects.
  #
  # ```ruby
  # Set[1, 2]                   # => #<Set: {1, 2}>
  # Set[1, 2, 1]                # => #<Set: {1, 2}>
  # Set[1, 'c', :s]             # => #<Set: {1, "c", :s}>
  # ```
  sig do
    type_parameters(:U).params(
        ary: T.type_parameter(:U),
    )
    .returns(T::Set[T.type_parameter(:U)])
  end
  def self.[](*ary); end

  # Alias for: [`|`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-7C)
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def +(enum); end

  # Returns a new set containing elements exclusive between the set and the
  # given enumerable object. (set ^ enum) is equivalent to ((set | enum) - (set
  # & enum)).
  #
  # ```ruby
  # Set[1, 2] ^ Set[2, 3]                   #=> #<Set: {3, 1}>
  # Set[1, 'b', 'c'] ^ ['b', 'd']           #=> #<Set: {"d", 1, "c"}>
  # ```
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def ^(enum); end

  # Adds the given object to the set and returns self. Use `merge` to add many
  # elements at once.
  #
  # ```ruby
  # Set[1, 2].add(3)                    #=> #<Set: {1, 2, 3}>
  # Set[1, 2].add([3, 4])               #=> #<Set: {1, 2, [3, 4]}>
  # Set[1, 2].add(2)                    #=> #<Set: {1, 2}>
  # ```
  #
  #
  # Also aliased as:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3C-3C)
  sig do
    params(
        o: Elem,
    )
    .returns(T.self_type)
  end
  def add(o); end

  # Adds the given object to the set and returns self. If the object is already
  # in the set, returns nil.
  #
  # ```ruby
  # Set[1, 2].add?(3)                    #=> #<Set: {1, 2, 3}>
  # Set[1, 2].add?([3, 4])               #=> #<Set: {1, 2, [3, 4]}>
  # Set[1, 2].add?(2)                    #=> nil
  # ```
  sig do
    params(
        o: Elem,
    )
    .returns(T.nilable(T.self_type))
  end
  def add?(o); end

  # Classifies the set by the return value of the given block and returns a hash
  # of {value => set of elements} pairs. The block is called once for each
  # element of the set, passing the element as parameter.
  #
  # ```ruby
  # require 'set'
  # files = Set.new(Dir.glob("*.rb"))
  # hash = files.classify { |f| File.mtime(f).year }
  # hash       #=> {2000=>#<Set: {"a.rb", "b.rb"}>,
  #            #    2001=>#<Set: {"c.rb", "d.rb", "e.rb"}>,
  #            #    2002=>#<Set: {"f.rb"}>}
  # ```
  #
  # Returns an enumerator if no block is given.
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: T.type_parameter(:U)).returns(Elem),
    )
    .returns(T::Hash[T.type_parameter(:U), T::Set[Elem]])
  end
  def classify(&blk); end

  # Removes all elements and returns self.
  #
  # ```ruby
  # set = Set[1, 'c', :s]             #=> #<Set: {1, "c", :s}>
  # set.clear                         #=> #<Set: {}>
  # set                               #=> #<Set: {}>
  # ```
  sig {returns(T.self_type)}
  def clear(); end

  # Deletes the given object from the set and returns self. Use `subtract` to
  # delete many items at once.
  sig do
    params(
        o: Elem,
    )
    .returns(T.self_type)
  end
  def delete(o); end

  # Deletes the given object from the set and returns self. If the object is not
  # in the set, returns nil.
  sig do
    params(
        o: Elem,
    )
    .returns(T.nilable(T.self_type))
  end
  def delete?(o); end

  # Deletes every element of the set for which block evaluates to true, and
  # returns self. Returns an enumerator if no block is given.
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  def delete_if(&blk); end

  # Alias for: [`-`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-2D)
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def difference(enum); end

  # Returns true if the set and the given set have no element in common. This
  # method is the opposite of `intersect?`.
  #
  # ```ruby
  # Set[1, 2, 3].disjoint? Set[3, 4]   #=> false
  # Set[1, 2, 3].disjoint? Set[4, 5]   #=> true
  # ```
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def disjoint?(set); end

  # Calls the given block once for each element in the set, passing the element
  # as parameter. Returns an enumerator if no block is given.
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Elem])}
  def each(&blk); end

  # Returns true if the set contains no elements.
  sig {returns(T::Boolean)}
  def empty?(); end

  # Returns a new set that is a copy of the set, flattening each containing set
  # recursively.
  sig {returns(T::Set[T.untyped])}
  def flatten(); end

  # Equivalent to
  # [`Set#flatten`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-flatten),
  # but replaces the receiver with the result in place. Returns nil if no
  # modifications were made.
  sig {returns(T.nilable(T.self_type))}
  def flatten!(); end

  sig do
    params(
        enum: T.nilable(T::Enumerable[BasicObject]),
    )
    .void
  end
  def initialize(enum=nil); end

  # Returns true if the set and the given set have at least one element in
  # common.
  #
  # ```ruby
  # Set[1, 2, 3].intersect? Set[4, 5]   #=> false
  # Set[1, 2, 3].intersect? Set[3, 4]   #=> true
  # ```
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def intersect?(set); end

  # Alias for: [`&`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-26)
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def intersection(enum); end

  # Returns a string created by converting each element of the array to a
  # string, separated by the given `separator`. If the `separator` is `nil`, it
  # uses current `$,`. If both the `separator` and `$,` are `nil`, it uses an
  # empty string.
  #
  # ```ruby
  # [ "a", "b", "c" ].join        #=> "abc"
  # [ "a", "b", "c" ].join("-")   #=> "a-b-c"
  # ```
  #
  # For nested arrays, join is applied recursively:
  #
  # ```ruby
  # [ "a", [1, 2, [:x, :y]], "b" ].join("-")   #=> "a-1-2-x-y-b"
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def join(arg0=T.unsafe(nil)); end

  # Deletes every element of the set for which block evaluates to false, and
  # returns self. Returns an enumerator if no block is given.
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  def keep_if(&blk); end

  # Alias for:
  # [`collect!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-collect-21)
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Set[T.type_parameter(:U)])
  end
  def map!(&blk); end

  # Alias for:
  # [`include?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-include-3F)
  sig do
    params(
        o: Elem,
    )
    .returns(T::Boolean)
  end
  def member?(o); end

  # Merges the elements of the given enumerable object to the set and returns
  # self.
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T.self_type)
  end
  def merge(enum); end

  # Returns true if the set is a proper subset of the given set.
  #
  # Also aliased as:
  # [`<`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3C)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def proper_subset?(set); end

  # Returns true if the set is a proper superset of the given set.
  #
  # Also aliased as:
  # [`>`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3E)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def proper_superset?(set); end

  # Equivalent to
  # [`Set#delete_if`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-delete_if),
  # but returns nil if no changes were made. Returns an enumerator if no block
  # is given.
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(T.self_type))
  end
  def reject!(&blk); end

  # Replaces the contents of the set with the contents of the given enumerable
  # object and returns self.
  #
  # ```ruby
  # set = Set[1, 'c', :s]             #=> #<Set: {1, "c", :s}>
  # set.replace([1, 2])               #=> #<Set: {1, 2}>
  # set                               #=> #<Set: {1, 2}>
  # ```
  sig do
    type_parameters(:U).params(
        enum: T::Enumerable[T.type_parameter(:U)],
    )
    .returns(T::Set[T.type_parameter(:U)])
  end
  def replace(enum); end

  # Equivalent to
  # [`Set#keep_if`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-keep_if),
  # but returns nil if no changes were made. Returns an enumerator if no block
  # is given.
  #
  # Also aliased as:
  # [`filter!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-filter-21)
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(T.self_type))
  end
  def select!(&blk); end

  # Equivalent to
  # [`Set#select!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-select-21)
  #
  # Alias for:
  # [`select!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-select-21)
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(T.self_type))
  end
  def filter!(&blk); end

  # Returns the number of elements.
  #
  # Also aliased as:
  # [`length`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-length)
  sig {returns(Integer)}
  def size(); end

  # Returns true if the set is a subset of the given set.
  #
  # Also aliased as:
  # [`<=`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3C-3D)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def subset?(set); end

  # Deletes every element that appears in the given enumerable object and
  # returns self.
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T.self_type)
  end
  def subtract(enum); end

  # Returns true if the set is a superset of the given set.
  #
  # Also aliased as:
  # [`>=`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3E-3D)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def superset?(set); end

  # Converts the set to an array. The order of elements is uncertain.
  #
  # ```ruby
  # Set[1, 2].to_a                    #=> [1, 2]
  # Set[1, 'c', :s].to_a              #=> [1, "c", :s]
  # ```
  sig {returns(T::Array[Elem])}
  def to_a(); end

  # Returns a new set containing elements common to the set and the given
  # enumerable object.
  #
  # ```ruby
  # Set[1, 3, 5] & Set[3, 2, 1]             #=> #<Set: {3, 1}>
  # Set['a', 'b', 'z'] & ['a', 'b', 'c']    #=> #<Set: {"a", "b"}>
  # ```
  #
  #
  # Also aliased as:
  # [`intersection`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-intersection)
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def &(enum); end

  # Returns a new set built by duplicating the set, removing every element that
  # appears in the given enumerable object.
  #
  # ```ruby
  # Set[1, 3, 5] - Set[1, 5]                #=> #<Set: {3}>
  # Set['a', 'b', 'z'] - ['a', 'c']         #=> #<Set: {"b", "z"}>
  # ```
  #
  #
  # Also aliased as:
  # [`difference`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-difference)
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def -(enum); end

  # Alias for:
  # [`proper_subset?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-proper_subset-3F)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def <(set); end

  # Alias for:
  # [`add`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-add)
  sig do
    params(
        o: Elem,
    )
    .returns(T.self_type)
  end
  def <<(o); end

  # Alias for:
  # [`subset?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-subset-3F)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def <=(set); end

  # Alias for:
  # [`proper_superset?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-proper_superset-3F)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def >(set); end

  # Alias for:
  # [`superset?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-superset-3F)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def >=(set); end

  # Replaces the elements with ones returned by collect(). Returns an enumerator
  # if no block is given.
  #
  # Also aliased as:
  # [`map!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-map-21)
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Set[T.type_parameter(:U)])
  end
  def collect!(&blk); end

  # Returns true if the set contains the given object.
  #
  # Note that `include?` and `member?` do not test member equality using `==` as
  # do other Enumerables.
  #
  # See also
  # [`Enumerable#include?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-include-3F)
  #
  # Also aliased as:
  # [`member?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-member-3F),
  # [`===`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3D-3D-3D)
  sig do
    params(
        o: Elem,
    )
    .returns(T::Boolean)
  end
  def include?(o); end

  # Alias for:
  # [`size`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-size)
  sig {returns(Integer)}
  def length(); end

  # Returns a new set built by merging the set and the elements of the given
  # enumerable object.
  #
  # ```ruby
  # Set[1, 2, 3] | Set[2, 4, 5]         #=> #<Set: {1, 2, 3, 4, 5}>
  # Set[1, 5, 'z'] | (1..6)             #=> #<Set: {1, 5, "z", 2, 3, 4, 6}>
  # ```
  #
  #
  # Also aliased as:
  # [`+`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-2B),
  # [`union`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-union)
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def |(enum); end

  # Alias for: [`|`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-7C)
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def union(enum); end
end

# [`SortedSet`](https://docs.ruby-lang.org/en/2.7.0/SortedSet.html) implements a
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) that guarantees that its
# elements are yielded in sorted order (according to the return values of their
# #<=> methods) when iterating over them.
#
# All elements that are added to a
# [`SortedSet`](https://docs.ruby-lang.org/en/2.7.0/SortedSet.html) must respond
# to the <=> method for comparison.
#
# Also, all elements must be *mutually comparable*: `el1 <=> el2` must not
# return `nil` for any elements `el1` and `el2`, else an
# [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html) will
# be raised when iterating over the
# [`SortedSet`](https://docs.ruby-lang.org/en/2.7.0/SortedSet.html).
#
# ## Example
#
# ```ruby
# require "set"
#
# set = SortedSet.new([2, 1, 5, 6, 4, 5, 3, 3, 3])
# ary = []
#
# set.each do |obj|
#   ary << obj
# end
#
# p ary # => [1, 2, 3, 4, 5, 6]
#
# set2 = SortedSet.new([1, 2, "3"])
# set2.each { |obj| } # => raises ArgumentError: comparison of Fixnum with String failed
# ```
class SortedSet < Set
  extend T::Generic
  Elem = type_member(:out)
end

# Arrays are ordered, integer-indexed collections of any object.
#
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) indexing starts at
# 0, as in C or Java. A negative index is assumed to be relative to the end of
# the array---that is, an index of -1 indicates the last element of the array,
# -2 is the next to last element in the array, and so on.
#
# ## Creating Arrays
#
# A new array can be created by using the literal constructor `[]`. Arrays can
# contain different types of objects. For example, the array below contains an
# [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) and a Float:
#
# ```ruby
# ary = [1, "two", 3.0] #=> [1, "two", 3.0]
# ```
#
# An array can also be created by explicitly calling
# [`Array.new`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-c-new)
# with zero, one (the initial size of the
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)) or two arguments
# (the initial size and a default object).
#
# ```ruby
# ary = Array.new    #=> []
# Array.new(3)       #=> [nil, nil, nil]
# Array.new(3, true) #=> [true, true, true]
# ```
#
# Note that the second argument populates the array with references to the same
# object. Therefore, it is only recommended in cases when you need to
# instantiate arrays with natively immutable objects such as Symbols, numbers,
# true or false.
#
# To create an array with separate objects a block can be passed instead. This
# method is safe to use with mutable objects such as hashes, strings or other
# arrays:
#
# ```ruby
# Array.new(4) {Hash.new}    #=> [{}, {}, {}, {}]
# Array.new(4) {|i| i.to_s } #=> ["0", "1", "2", "3"]
# ```
#
# This is also a quick way to build up multi-dimensional arrays:
#
# ```ruby
# empty_table = Array.new(3) {Array.new(3)}
# #=> [[nil, nil, nil], [nil, nil, nil], [nil, nil, nil]]
# ```
#
# An array can also be created by using the Array() method, provided by
# [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html), which tries to
# call
# [`to_ary`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-to_ary),
# then [`to_a`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-to_a) on
# its argument.
#
# ```ruby
# Array({:a => "a", :b => "b"}) #=> [[:a, "a"], [:b, "b"]]
# ```
#
# ## Example Usage
#
# In addition to the methods it mixes in through the
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) module,
# the [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) class has
# proprietary methods for accessing, searching and otherwise manipulating
# arrays.
#
# Some of the more common ones are illustrated below.
#
# ## Accessing Elements
#
# Elements in an array can be retrieved using the
# [`Array#[]`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-5B-5D)
# method. It can take a single integer argument (a numeric index), a pair of
# arguments (start and length) or a range. Negative indices start counting from
# the end, with -1 being the last element.
#
# ```ruby
# arr = [1, 2, 3, 4, 5, 6]
# arr[2]    #=> 3
# arr[100]  #=> nil
# arr[-3]   #=> 4
# arr[2, 3] #=> [3, 4, 5]
# arr[1..4] #=> [2, 3, 4, 5]
# arr[1..-3] #=> [2, 3, 4]
# ```
#
# Another way to access a particular array element is by using the
# [`at`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-at) method
#
# ```ruby
# arr.at(0) #=> 1
# ```
#
# The [`slice`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-slice)
# method works in an identical manner to
# [`Array#[]`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-5B-5D).
#
# To raise an error for indices outside of the array bounds or else to provide a
# default value when that happens, you can use
# [`fetch`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-fetch).
#
# ```ruby
# arr = ['a', 'b', 'c', 'd', 'e', 'f']
# arr.fetch(100) #=> IndexError: index 100 outside of array bounds: -6...6
# arr.fetch(100, "oops") #=> "oops"
# ```
#
# The special methods
# [`first`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-first) and
# [`last`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-last) will
# return the first and last elements of an array, respectively.
#
# ```ruby
# arr.first #=> 1
# arr.last  #=> 6
# ```
#
# To return the first `n` elements of an array, use
# [`take`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-take)
#
# ```ruby
# arr.take(3) #=> [1, 2, 3]
# ```
#
# [`drop`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-drop) does
# the opposite of
# [`take`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-take), by
# returning the elements after `n` elements have been dropped:
#
# ```ruby
# arr.drop(3) #=> [4, 5, 6]
# ```
#
# ## Obtaining Information about an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
#
# Arrays keep track of their own length at all times. To query an array about
# the number of elements it contains, use
# [`length`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-length),
# [`count`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-count) or
# [`size`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-size).
#
# ```ruby
# browsers = ['Chrome', 'Firefox', 'Safari', 'Opera', 'IE']
# browsers.length #=> 5
# browsers.count #=> 5
# ```
#
# To check whether an array contains any elements at all
#
# ```ruby
# browsers.empty? #=> false
# ```
#
# To check whether a particular item is included in the array
#
# ```ruby
# browsers.include?('Konqueror') #=> false
# ```
#
# ## Adding Items to Arrays
#
# Items can be added to the end of an array by using either
# [`push`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-push) or
# [`<<`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-3C-3C)
#
# ```ruby
# arr = [1, 2, 3, 4]
# arr.push(5) #=> [1, 2, 3, 4, 5]
# arr << 6    #=> [1, 2, 3, 4, 5, 6]
# ```
#
# [`unshift`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-unshift)
# will add a new item to the beginning of an array.
#
# ```ruby
# arr.unshift(0) #=> [0, 1, 2, 3, 4, 5, 6]
# ```
#
# With
# [`insert`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-insert) you
# can add a new element to an array at any position.
#
# ```ruby
# arr.insert(3, 'apple')  #=> [0, 1, 2, 'apple', 3, 4, 5, 6]
# ```
#
# Using the
# [`insert`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-insert)
# method, you can also insert multiple values at once:
#
# ```ruby
# arr.insert(3, 'orange', 'pear', 'grapefruit')
# #=> [0, 1, 2, "orange", "pear", "grapefruit", "apple", 3, 4, 5, 6]
# ```
#
# ## Removing Items from an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
#
# The method
# [`pop`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-pop) removes
# the last element in an array and returns it:
#
# ```ruby
# arr =  [1, 2, 3, 4, 5, 6]
# arr.pop #=> 6
# arr #=> [1, 2, 3, 4, 5]
# ```
#
# To retrieve and at the same time remove the first item, use
# [`shift`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-shift):
#
# ```ruby
# arr.shift #=> 1
# arr #=> [2, 3, 4, 5]
# ```
#
# To delete an element at a particular index:
#
# ```ruby
# arr.delete_at(2) #=> 4
# arr #=> [2, 3, 5]
# ```
#
# To delete a particular element anywhere in an array, use
# [`delete`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-delete):
#
# ```ruby
# arr = [1, 2, 2, 3]
# arr.delete(2) #=> 2
# arr #=> [1,3]
# ```
#
# A useful method if you need to remove `nil` values from an array is
# [`compact`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-compact):
#
# ```ruby
# arr = ['foo', 0, nil, 'bar', 7, 'baz', nil]
# arr.compact  #=> ['foo', 0, 'bar', 7, 'baz']
# arr          #=> ['foo', 0, nil, 'bar', 7, 'baz', nil]
# arr.compact! #=> ['foo', 0, 'bar', 7, 'baz']
# arr          #=> ['foo', 0, 'bar', 7, 'baz']
# ```
#
# Another common need is to remove duplicate elements from an array.
#
# It has the non-destructive
# [`uniq`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-uniq), and
# destructive method
# [`uniq!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-uniq-21)
#
# ```ruby
# arr = [2, 5, 6, 556, 6, 6, 8, 9, 0, 123, 556]
# arr.uniq #=> [2, 5, 6, 556, 8, 9, 0, 123]
# ```
#
# ## Iterating over Arrays
#
# Like all classes that include the
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) module,
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) has an each method,
# which defines what elements should be iterated over and how. In case of
# Array's
# [`each`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-each), all
# elements in the [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
# instance are yielded to the supplied block in sequence.
#
# Note that this operation leaves the array unchanged.
#
# ```ruby
# arr = [1, 2, 3, 4, 5]
# arr.each {|a| print a -= 10, " "}
# # prints: -9 -8 -7 -6 -5
# #=> [1, 2, 3, 4, 5]
# ```
#
# Another sometimes useful iterator is
# [`reverse_each`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-reverse_each)
# which will iterate over the elements in the array in reverse order.
#
# ```ruby
# words = %w[first second third fourth fifth sixth]
# str = ""
# words.reverse_each {|word| str += "#{word} "}
# p str #=> "sixth fifth fourth third second first "
# ```
#
# The [`map`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-map)
# method can be used to create a new array based on the original array, but with
# the values modified by the supplied block:
#
# ```ruby
# arr.map {|a| 2*a}     #=> [2, 4, 6, 8, 10]
# arr                   #=> [1, 2, 3, 4, 5]
# arr.map! {|a| a**2}   #=> [1, 4, 9, 16, 25]
# arr                   #=> [1, 4, 9, 16, 25]
# ```
#
# ## Selecting Items from an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
#
# Elements can be selected from an array according to criteria defined in a
# block. The selection can happen in a destructive or a non-destructive manner.
# While the destructive operations will modify the array they were called on,
# the non-destructive methods usually return a new array with the selected
# elements, but leave the original array unchanged.
#
# ### Non-destructive Selection
#
# ```ruby
# arr = [1, 2, 3, 4, 5, 6]
# arr.select {|a| a > 3}       #=> [4, 5, 6]
# arr.reject {|a| a < 3}       #=> [3, 4, 5, 6]
# arr.drop_while {|a| a < 4}   #=> [4, 5, 6]
# arr                          #=> [1, 2, 3, 4, 5, 6]
# ```
#
# ### Destructive Selection
#
# [`select!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-select-21)
# and
# [`reject!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-reject-21)
# are the corresponding destructive methods to
# [`select`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-select) and
# [`reject`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-reject)
#
# Similar to
# [`select`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-select) vs.
# [`reject`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-reject),
# [`delete_if`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-delete_if)
# and
# [`keep_if`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-keep_if)
# have the exact opposite result when supplied with the same block:
#
# ```ruby
# arr.delete_if {|a| a < 4}   #=> [4, 5, 6]
# arr                         #=> [4, 5, 6]
#
# arr = [1, 2, 3, 4, 5, 6]
# arr.keep_if {|a| a < 4}   #=> [1, 2, 3]
# arr                       #=> [1, 2, 3]
# ```
#
# for pack.c
class Array
  sig { params(blk: T.nilable(T.proc.params(arg0: T.untyped).void)).returns(T::Set[T.untyped]) }
  def to_set(&blk); end
end

# The [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) mixin
# provides collection classes with several traversal and searching methods, and
# with the ability to sort. The class must provide a method each, which yields
# successive members of the collection. If
# [`Enumerable#max`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-max),
# [`min`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-min), or
# [`sort`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort) is
# used, the objects in the collection must also implement a meaningful `<=>`
# operator, as these methods rely on an ordering between members of the
# collection.
module Enumerable
  # Makes a set from the enumerable object with given arguments. Needs to
  # +require "set"+ to use this method.
  def to_set(klass = _, *args, &block); end
end
