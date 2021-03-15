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
    type_parameters(:U).params(
        enum: T::Enumerable[T.type_parameter(:U)],
    )
    .void
  end
  def initialize(enum=T.unsafe(nil)); end

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
