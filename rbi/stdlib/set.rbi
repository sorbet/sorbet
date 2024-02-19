# typed: __STDLIB_INTERNAL

# This library provides the
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) class, which deals with
# a
# collection
# of
# unordered values with no duplicates. It is a hybrid of
# Array's
# intuitive
# inter-operation facilities and Hash's fast lookup.
# The method `to_set` is added to
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) for
# convenience.
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) implements a collection
# of unordered values with no
# duplicates.
# This
# is a hybrid of Array's intuitive inter-operation facilities
# and
# Hash's
# fast lookup.
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) is easy to use with
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) objects
# (implementing
# `each`).
# Most
# of the initializer methods and binary operators accept
# generic
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html)
# objects besides sets and arrays. An
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html)
# object
# can
# be converted to [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) using
# the `to_set` method.
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) uses
# [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) as storage, so you
# must note the following points:
# *   Equality of elements is determined according to
#     [`Object#eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F)
#     and
#
#     [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash).
#     Use
#     [`Set#compare_by_identity`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-compare_by_identity)
#     to make a set
#     compare
#
#     its elements by their identity.
# *   [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) assumes that the
#     identity of each element does not
#     change
#
#     while it is stored. Modifying an element of a set will render
#     the
#
#     set to an unreliable state.
# *   When a string is to be stored, a frozen copy of the string
#     is
#
#     stored instead unless the original string is already frozen.
#
# ## Comparison
# The comparison operators `<`, `>`, `<=`, and `>=` are implemented
# as
# shorthand
# for the {proper\_,}{subset?,superset?} methods. The
# `<=>`
# operator
# reflects this order, or return `nil` for sets that
# both
# have
# distinct elements (`{x, y}` vs. `{x, z}` for example).
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
# *   Akinori MUSHA <mailto:knu@iDaemons.org> (current maintainer)
#
# ## What's Here
# First, what's elsewhere.
# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) Set:
# *   Inherits from [class
#     Object](https://docs.ruby-lang.org/en/master/Object.html#class-Object-label-What-27s+Here).
# *   Includes [module
#     Enumerable](https://docs.ruby-lang.org/en/master/Enumerable.html#module-Enumerable-label-What-27s+Here),
#
#     which provides dozens of additional methods.
#
# In particular, class [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html)
# does not have many methods of its
# own
# for
# fetching or for
# iterating.
# Instead,
# it relies on those in
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html).
# Here, class [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) provides
# methods that are useful for:
# *   [Creating a Set](#class-Set-label-Methods+for+Creating+a+Set)
# *   [Set Operations](#class-Set-label-Methods+for+Set+Operations)
# *   [Comparing](#class-Set-label-Methods+for+Comparing)
# *   [Querying](#class-Set-label-Methods+for+Querying)
# *   [Assigning](#class-Set-label-Methods+for+Assigning)
# *   [Deleting](#class-Set-label-Methods+for+Deleting)
# *   [Converting](#class-Set-label-Methods+for+Converting)
# *   [Iterating](#class-Set-label-Methods+for+Iterating)
# *   [And more....](#class-Set-label-Other+Methods)
#
# ### Methods for Creating a [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html)
# *   [`::[]`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-c-5B-5D)
#     -
#
#     Returns a new set containing the given objects.
# *   [`::new`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-c-new)
#     -
#
#     Returns a new set containing either the given
#     objects
#
#     (if no block given) or the return values from the called
#     block
#
#     (if a block given).
#
# ### Methods for [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) Operations
# *   [|](#method-i-7C) (aliased as
#     [`union`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-union) and
#     [`+`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-2B))
#     -
#
#     Returns a new set containing all elements from
#     `self`
#
#     and all elements from a given enumerable (no duplicates).
# *   [&](#method-i-26) (aliased as
#     [`intersection`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-intersection))
#     -
#
#     Returns a new set containing all elements common to
#     `self`
#
#     and a given enumerable.
# *   [-](#method-i-2D) (aliased as
#     [`difference`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-difference))
#     -
#
#     Returns a copy of `self` with all
#     elements
#
#     in a given enumerable removed.
# *   [\^](#method-i-5E)
#     -
#
#     Returns a new set containing all elements from
#     `self`
#
#     and a given enumerable except those common to both.
#
# ### Methods for Comparing
# *   [<=>](#method-i-3C-3D-3E)
#     -
#
#     Returns -1, 0, or 1 as `self` is less than, equal
#     to,
#
#     or greater than a given object.
# *   [==](#method-i-3D-3D)
#     -
#
#     Returns whether `self` and a given enumerable are
#     equal,
#
#     as determined by
#     [`Object#eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F).
# *   [`compare_by_identity?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-compare_by_identity-3F)
#     -
#
#     Returns whether the set considers only
#     identity
#
#     when comparing elements.
#
# ### Methods for Querying
# *   [`length`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-length)
#     (aliased as
#     [`size`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-size))
#     -
#
#     Returns the count of elements.
# *   [`empty?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-empty-3F)
#     -
#
#     Returns whether the set has no elements.
# *   [`include?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-include-3F)
#     (aliased as
#     [`member?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-member-3F)
#     and
#     [`===`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3D-3D-3D))
#     -
#
#     Returns whether a given object is an element in the set.
# *   [`subset?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-subset-3F)
#     (aliased as [<=](#method-i-3C-3D))
#     -
#
#     Returns whether a given object is a subset of the set.
# *   [`proper_subset?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-proper_subset-3F)
#     (aliased as [<](#method-i-3C))
#     -
#
#     Returns whether a given enumerable is a proper subset of the set.
# *   [`superset?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-superset-3F)
#     (aliased as [<=](#method-i-3E-3D)])
#     -
#
#     Returns whether a given enumerable is a superset of the set.
# *   [`proper_superset?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-proper_superset-3F)
#     (aliased as [>](#method-i-3E))
#     -
#
#     Returns whether a given enumerable is a proper superset of the set.
# *   [`disjoint?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-disjoint-3F)
#     -
#
#     Returns `true` if the set and a given
#     enumerable
#
#     have no common elements, `false` otherwise.
# *   [`intersect?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-intersect-3F)
#     -
#
#     Returns `true` if the set and a given enumerable
#     -
#
#     have any common elements, `false` otherwise.
# *   [`compare_by_identity?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-compare_by_identity-3F)
#     -
#
#     Returns whether the set considers only
#     identity
#
#     when comparing elements.
#
# ### Methods for Assigning
# *   [`add`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-add)
#     (aliased as
#     [`&lt;&lt;`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3C-3C))
#     -
#
#     Adds a given object to the set; returns `self`.
# *   [`add?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-add-3F)
#     -
#
#     If the given object is not an element in the
#     set,
#
#     adds it and returns `self`; otherwise, returns `nil`.
# *   [`merge`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-merge)
#     -
#
#     Adds each given object to the set; returns `self`.
# *   [`replace`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-replace)
#     -
#
#     Replaces the contents of the set with the
#     contents
#
#     of a given enumerable.
#
# ### Methods for Deleting
# *   [`clear`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-clear)
#     -
#
#     Removes all elements in the set; returns `self`.
# *   [`delete`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-delete)
#     -
#
#     Removes a given object from the set; returns `self`.
# *   [`delete?`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-delete-3F)
#     -
#
#     If the given object is an element in the
#     set,
#
#     removes it and returns `self`; otherwise, returns `nil`.
# *   [`subtract`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-subtract)
#     -
#
#     Removes each given object from the set; returns `self`.
# *   [`delete_if`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-delete_if)
#     - Removes elements specified by a given block.
# *   [`select!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-select-21)
#     (aliased as
#     [`filter!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-filter-21))
#     -
#
#     Removes elements not specified by a given block.
# *   [`keep_if`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-keep_if)
#     -
#
#     Removes elements not specified by a given block.
# *   [`reject!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-reject-21)
#
#     Removes elements specified by a given block.
#
# ### Methods for Converting
# *   [`classify`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-classify)
#     -
#
#     Returns a hash that classifies the
#     elements,
#
#     as determined by the given block.
# *   [`collect!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-collect-21)
#     (aliased as
#     [`map!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-map-21))
#     -
#
#     Replaces each element with a block return-value.
# *   [`divide`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-divide)
#     -
#
#     Returns a hash that classifies the
#     elements,
#
#     as determined by the given
#     block;
#
#     differs from
#     [`classify`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-classify)
#     in that the block may
#     accept
#
#     either one or two arguments.
# *   [`flatten`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-flatten)
#     -
#
#     Returns a new set that is a recursive flattening of
#     `self`.
#
#     [`flatten!`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-flatten-21)
#     -
#
#     Replaces each nested set in `self` with the elements from that set.
# *   [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-inspect)
#     (aliased as
#     [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-to_s))
#     -
#
#     Returns a string displaying the elements.
# *   [`join`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-join)
#     -
#
#     Returns a string containing all elements, converted to
#     strings
#
#     as needed, and joined by the given record separator.
# *   [`to_a`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-to_a)
#     -
#
#     Returns an array containing all set elements.
# *   [`to_set`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-to_set)
#     -
#
#     Returns `self` if given no arguments and no
#     block;
#
#     with a block given, returns a new set consisting of
#     block
#
#     return values.
#
# ### Methods for Iterating
# *   [`each`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-each)
#     -
#
#     Calls the block with each successive element; returns `self`.
#
# ### Other Methods
# *   [`reset`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-reset)
#     -
#
#     Resets the internal state; useful if an
#     object
#
#     has been modified while an element in the set.
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

  # Returns a new set containing elements exclusive between the
  # set
  # and
  # the given enumerable object. `(set ^ enum)` is equivalent
  # to
  # `((set
  # | enum) - (set & enum))`.
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

  # Adds the given object to the set and returns self. Use `merge`
  # to
  # add
  # many elements at once.
  #
  # ```ruby
  # Set[1, 2].add(3)                    #=> #<Set: {1, 2, 3}>
  # Set[1, 2].add([3, 4])               #=> #<Set: {1, 2, [3, 4]}>
  # Set[1, 2].add(2)                    #=> #<Set: {1, 2}>
  # ```
  #
  #
  # Also aliased as:
  # [`&lt;&lt;`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3C-3C)
  sig do
    params(
        o: Elem,
    )
    .returns(T.self_type)
  end
  def add(o); end

  # Adds the given object to the set and returns self. If
  # the
  # object
  # is already in the set, returns nil.
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

  # Classifies the set by the return value of the given block
  # and
  # returns
  # a hash of {value => set of elements} pairs. The block
  # is
  # called
  # once for each element of the set, passing the element
  # as
  # parameter.
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

  # Makes the set compare its elements by their identity and
  # returns
  # self.
  # This method may not be supported by all subclasses of
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html).
  sig { void }
  def compare_by_identity; end

  # Returns true if the set will compare its elements by
  # their
  # identity.
  # Also see
  # [`Set#compare_by_identity`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-compare_by_identity).
  sig { returns(T::Boolean) }
  def compare_by_identity?; end

  # Deletes the given object from the set and returns self.
  # Use
  # `subtract`
  # to delete many items at once.
  sig do
    params(
        o: Elem,
    )
    .returns(T.self_type)
  end
  def delete(o); end

  # Deletes the given object from the set and returns self. If
  # the
  # object
  # is not in the set, returns nil.
  sig do
    params(
        o: Elem,
    )
    .returns(T.nilable(T.self_type))
  end
  def delete?(o); end

  # Deletes every element of the set for which block evaluates
  # to
  # true,
  # and returns self. Returns an enumerator if no block
  # is
  # given.
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

  # Returns true if the set and the given enumerable
  # have
  # no
  # element in common. This method is the opposite of `intersect?`.
  #
  # ```ruby
  # Set[1, 2, 3].disjoint? Set[3, 4]   #=> false
  # Set[1, 2, 3].disjoint? Set[4, 5]   #=> true
  # Set[1, 2, 3].disjoint? [3, 4]      #=> false
  # Set[1, 2, 3].disjoint? 4..5        #=> true
  # ```
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def disjoint?(set); end

  # Calls the given block once for each element in the set,
  # passing
  # the
  # element as parameter. Returns an enumerator if no block
  # is
  # given.
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

  # Returns a new set that is a copy of the set, flattening
  # each
  # containing
  # set recursively.
  sig {returns(T::Set[T.untyped])}
  def flatten(); end

  # Equivalent to
  # [`Set#flatten`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-flatten),
  # but replaces the receiver with
  # the
  # result
  # in place. Returns nil if no modifications were made.
  sig {returns(T.nilable(T.self_type))}
  def flatten!(); end

  # Creates a new set containing the elements of the given
  # enumerable object.
  #
  # If a block is given, the elements of enum are preprocessed by
  # the given block.
  #
  # ```ruby
  # Set.new([1, 2])                    #=> #<Set: {1, 2}>
  # Set.new([1, 2, 1])                 #=> #<Set: {1, 2}>
  # Set.new([1, 'c', :s])              #=> #<Set: {1, "c", :s}>
  # Set.new(1..5)                      #=> #<Set: {1, 2, 3, 4, 5}>
  # Set.new([1, 2, 3]) { |x| x * x }   #=> #<Set: {1, 4, 9}>
  # ```
  sig do
    type_parameters(:U).params(
      enum: T.nilable(T::Enumerable[T.type_parameter(:U)]),
      blk: T.nilable(T.proc.params(arg0: T.type_parameter(:U)).returns(Elem))
    )
    .void
  end
  def initialize(enum=nil, &blk); end

  # Returns true if the set and the given enumerable have at least
  # one
  # element
  # in common.
  #
  # ```ruby
  # Set[1, 2, 3].intersect? Set[4, 5]   #=> false
  # Set[1, 2, 3].intersect? Set[3, 4]   #=> true
  # Set[1, 2, 3].intersect? 4..5        #=> false
  # Set[1, 2, 3].intersect? [3, 4]      #=> true
  # ```
  sig do
    params(
        set: T::Enumerable[Elem],
    )
    .returns(T::Boolean)
  end
  def intersect?(set); end

  # Alias for:
  # [`&amp;`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-26)
  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def intersection(enum); end

  # Returns a string created by converting each element of the set to a
  # string
  # See
  # also:
  # [`Array#join`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-join)
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def join(arg0=T.unsafe(nil)); end

  # Deletes every element of the set for which block evaluates
  # to
  # false,
  # and returns self. Returns an enumerator if no block
  # is
  # given.
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

  # Merges the elements of the given enumerable object to the set
  # and
  # returns
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
  # [`&lt;`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3C)
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
  # [`&gt;`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3E)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def proper_superset?(set); end

  # Equivalent to
  # [`Set#delete_if`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-delete_if),
  # but returns nil if no changes
  # were
  # made.
  # Returns an enumerator if no block is given.
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(T.self_type))
  end
  def reject!(&blk); end

  # Replaces the contents of the set with the contents of the
  # given
  # enumerable
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
  # but returns nil if no changes
  # were
  # made.
  # Returns an enumerator if no block is given.
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
  # [`&lt;=`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3C-3D)
  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T::Boolean)
  end
  def subset?(set); end

  # Deletes every element that appears in the given enumerable
  # object
  # and
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
  # [`&gt;=`](https://docs.ruby-lang.org/en/2.7.0/Set.html#method-i-3E-3D)
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

  # Returns a new set containing elements common to the set and
  # the
  # given
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

  # Returns a new set built by duplicating the set, removing
  # every
  # element
  # that appears in the given enumerable object.
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

  # Replaces the elements with ones returned by
  # `collect()`.
  # Returns
  # an enumerator if no block is given.
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
  # Note that `include?` and `member?` do not test
  # member
  # equality
  # using `==` as do other Enumerables.
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

  # Returns a new set built by merging the set and the elements of
  # the
  # given
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

  private

  # Clone internal hash.
  sig { params(orig: T::Set[Elem], options: T.untyped).returns(T::Set[Elem]) }
  def initialize_clone(orig, **options); end
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

# An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) is an ordered,
# integer-indexed collection of objects, called *elements*. Any object may be an
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) element.
#
# ## [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) Indexes
#
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) indexing starts at
# 0, as in C or Java.
#
# A positive index is an offset from the first element:
# *   Index 0 indicates the first element.
# *   Index 1 indicates the second element.
# *   ...
#
#
# A negative index is an offset, backwards, from the end of the array:
# *   Index -1 indicates the last element.
# *   Index -2 indicates the next-to-last element.
# *   ...
#
#
# A non-negative index is *in range* if it is smaller than the size of the
# array. For a 3-element array:
# *   Indexes 0 through 2 are in range.
# *   Index 3 is out of range.
#
#
# A negative index is *in range* if its absolute value is not larger than the
# size of the array. For a 3-element array:
# *   Indexes -1 through -3 are in range.
# *   Index -4 is out of range.
#
#
# ## Creating Arrays
#
# You can create an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
# object explicitly with:
#
# *   An [array literal](doc/syntax/literals_rdoc.html#label-Array+Literals).
#
#
# You can convert certain objects to Arrays with:
#
# *   [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html)
#     [Array](Kernel.html#method-i-Array).
#
#
# An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) can contain
# different types of objects. For example, the array below contains an
# [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) and a Float:
#
# ```ruby
# ary = [1, "two", 3.0] #=> [1, "two", 3.0]
# ```
#
# An array can also be created by calling
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
# [`&lt;&lt;`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-3C-3C)
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
# ## What's Here
#
# First, what's elsewhere.
# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) Array:
#
# *   Inherits from [class
#     Object](Object.html#class-Object-label-What-27s+Here).
# *   Includes [module
#     Enumerable](Enumerable.html#module-Enumerable-label-What-27s+Here), which
#     provides dozens of additional methods.
#
#
# Here, class [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) provides
# methods that are useful for:
#
# *   [Creating an Array](#class-Array-label-Methods+for+Creating+an+Array)
# *   [Querying](#class-Array-label-Methods+for+Querying)
# *   [Comparing](#class-Array-label-Methods+for+Comparing)
# *   [Fetching](#class-Array-label-Methods+for+Fetching)
# *   [Assigning](#class-Array-label-Methods+for+Assigning)
# *   [Deleting](#class-Array-label-Methods+for+Deleting)
# *   [Combining](#class-Array-label-Methods+for+Combining)
# *   [Iterating](#class-Array-label-Methods+for+Iterating)
# *   [Converting](#class-Array-label-Methods+for+Converting)
# *   [And more....](#class-Array-label-Other+Methods)
#
#
# ### Methods for Creating an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
#
# [`::[]`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-c-5B-5D)
# :   Returns a new array populated with given objects.
# [`::new`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-c-new)
# :   Returns a new array.
# [`::try_convert`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-c-try_convert)
# :   Returns a new array created from a given object.
#
#
# ### Methods for Querying
#
# [`length`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-length), [`size`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-size)
# :   Returns the count of elements.
# [`include?`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-include-3F)
# :   Returns whether any element `==` a given object.
# [`empty?`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-empty-3F)
# :   Returns whether there are no elements.
# [`all?`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-all-3F)
# :   Returns whether all elements meet a given criterion.
# [`any?`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-any-3F)
# :   Returns whether any element meets a given criterion.
# [`none?`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-none-3F)
# :   Returns whether no element `==` a given object.
# [`one?`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-one-3F)
# :   Returns whether exactly one element `==` a given object.
# [`count`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-count)
# :   Returns the count of elements that meet a given criterion.
# [`find_index`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-find_index), [`index`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-index)
# :   Returns the index of the first element that meets a given criterion.
# [`rindex`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-rindex)
# :   Returns the index of the last element that meets a given criterion.
# [`hash`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-hash)
# :   Returns the integer hash code.
#
#
# ### Methods for Comparing
# [#<=>](#method-i-3C-3D-3E)
# :   Returns -1, 0, or 1 as `self` is less than, equal to, or greater than a
#     given object.
# [#==](#method-i-3D-3D)
# :   Returns whether each element in `self` is `==` to the corresponding
#     element in a given object.
# [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-eql-3F)
# :   Returns whether each element in `self` is `eql?` to the corresponding
#     element in a given object.
#
#
# ### Methods for Fetching
#
# These methods do not modify `self`.
#
# [`[]`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-5B-5D)
# :   Returns one or more elements.
# [`fetch`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-fetch)
# :   Returns the element at a given offset.
# [`first`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-first)
# :   Returns one or more leading elements.
# [`last`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-last)
# :   Returns one or more trailing elements.
# [`max`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-max)
# :   Returns one or more maximum-valued elements, as determined by `<=>` or a
#     given block.
# [`max`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-max)
# :   Returns one or more minimum-valued elements, as determined by `<=>` or a
#     given block.
# [`minmax`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-minmax)
# :   Returns the minimum-valued and maximum-valued elements, as determined by
#     `<=>` or a given block.
# [`assoc`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-assoc)
# :   Returns the first element that is an array whose first element `==` a
#     given object.
# [`rassoc`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-rassoc)
# :   Returns the first element that is an array whose second element `==` a
#     given object.
# [`at`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-at)
# :   Returns the element at a given offset.
# [`values_at`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-values_at)
# :   Returns the elements at given offsets.
# [`dig`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-dig)
# :   Returns the object in nested objects that is specified by a given index
#     and additional arguments.
# [`drop`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-drop)
# :   Returns trailing elements as determined by a given index.
# [`take`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-take)
# :   Returns leading elements as determined by a given index.
# [`drop_while`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-drop_while)
# :   Returns trailing elements as determined by a given block.
# [`take_while`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-take_while)
# :   Returns leading elements as determined by a given block.
# [`slice`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-slice)
# :   Returns consecutive elements as determined by a given argument.
# [`sort`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-sort)
# :   Returns all elements in an order determined by `<=>` or a given block.
# [`reverse`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-reverse)
# :   Returns all elements in reverse order.
# [`compact`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-compact)
# :   Returns an array containing all non-`nil` elements.
# [`select`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-select), [`filter`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-filter)
# :   Returns an array containing elements selected by a given block.
# [`uniq`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-uniq)
# :   Returns an array containing non-duplicate elements.
# [`rotate`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-rotate)
# :   Returns all elements with some rotated from one end to the other.
# [`bsearch`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-bsearch)
# :   Returns an element selected via a binary search as determined by a given
#     block.
# [`bsearch_index`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-bsearch_index)
# :   Returns the index of an element selected via a binary search as determined
#     by a given block.
# [`sample`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-sample)
# :   Returns one or more random elements.
# [`shuffle`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-shuffle)
# :   Returns elements in a random order.
#
#
# ### Methods for Assigning
#
# These methods add, replace, or reorder elements in `self`.
#
# [`[]=`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-5B-5D-3D)
# :   Assigns specified elements with a given object.
# [`push`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-push), [`append`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-append), [`&lt;&lt;`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-3C-3C)
# :   Appends trailing elements.
# [`unshift`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-unshift), [`prepend`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-prepend)
# :   Prepends leading elements.
# [`insert`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-insert)
# :   Inserts given objects at a given offset; does not replace elements.
# [`concat`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-concat)
# :   Appends all elements from given arrays.
# [`fill`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-fill)
# :   Replaces specified elements with specified objects.
# [`replace`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-replace)
# :   Replaces the content of `self` with the content of a given array.
# [`reverse!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-reverse-21)
# :   Replaces `self` with its elements reversed.
# [`rotate!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-rotate-21)
# :   Replaces `self` with its elements rotated.
# [`shuffle!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-shuffle-21)
# :   Replaces `self` with its elements in random order.
# [`sort!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-sort-21)
# :   Replaces `self` with its elements sorted, as determined by `<=>` or a
#     given block.
# [`sort_by!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-sort_by-21)
# :   Replaces `self` with its elements sorted, as determined by a given block.
#
#
# ### Methods for Deleting
#
# Each of these methods removes elements from `self`:
#
# [`pop`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-pop)
# :   Removes and returns the last element.
# [`shift`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-shift)
# :   Removes and returns the first element.
# [`compact!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-compact-21)
# :   Removes all non-`nil` elements.
# [`delete`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-delete)
# :   Removes elements equal to a given object.
# [`delete_at`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-delete_at)
# :   Removes the element at a given offset.
# [`delete_if`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-delete_if)
# :   Removes elements specified by a given block.
# [`keep_if`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-keep_if)
# :   Removes elements not specified by a given block.
# [`reject!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-reject-21)
# :   Removes elements specified by a given block.
# [`select!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-select-21), [`filter!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-filter-21)
# :   Removes elements not specified by a given block.
# [`slice!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-slice-21)
# :   Removes and returns a sequence of elements.
# [`uniq!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-uniq-21)
# :   Removes duplicates.
#
#
# ### Methods for Combining
#
# [#&](#method-i-26)
# :   Returns an array containing elements found both in `self` and a given
#     array.
# [`intersection`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-intersection)
# :   Returns an array containing elements found both in `self` and in each
#     given array.
# [`+`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-2B)
# :   Returns an array containing all elements of `self` followed by all
#     elements of a given array.
# [`-`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-2D)
# :   Returns an array containiing all elements of `self` that are not found in
#     a given array.
# [#|](#method-i-7C)
# :   Returns an array containing all elements of `self` and all elements of a
#     given array, duplicates removed.
# [`union`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-union)
# :   Returns an array containing all elements of `self` and all elements of
#     given arrays, duplicates removed.
# [`difference`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-difference)
# :   Returns an array containing all elements of `self` that are not found in
#     any of the given arrays..
# [`product`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-product)
# :   Returns or yields all combinations of elements from `self` and given
#     arrays.
#
#
# ### Methods for Iterating
#
# [`each`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-each)
# :   Passes each element to a given block.
# [`reverse_each`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-reverse_each)
# :   Passes each element, in reverse order, to a given block.
# [`each_index`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-each_index)
# :   Passes each element index to a given block.
# [`cycle`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-cycle)
# :   Calls a given block with each element, then does so again, for a specified
#     number of times, or forever.
# [`combination`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-combination)
# :   Calls a given block with combinations of elements of `self`; a combination
#     does not use the same element more than once.
# [`permutation`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-permutation)
# :   Calls a given block with permutations of elements of `self`; a permutation
#     does not use the same element more than once.
# [`repeated_combination`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-repeated_combination)
# :   Calls a given block with combinations of elements of `self`; a combination
#     may use the same element more than once.
# [`repeated_permutation`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-repeated_permutation)
# :   Calls a given block with permutations of elements of `self`; a permutation
#     may use the same element more than once.
#
#
# ### Methods for Converting
#
# [`map`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-map), [`collect`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-collect)
# :   Returns an array containing the block return-value for each element.
# [`map!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-map-21), [`collect!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-collect-21)
# :   Replaces each element with a block return-value.
# [`flatten`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-flatten)
# :   Returns an array that is a recursive flattening of `self`.
# [`flatten!`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-flatten-21)
# :   Replaces each nested array in `self` with the elements from that array.
# [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-inspect), [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-to_s)
# :   Returns a new [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
#     containing the elements.
# [`join`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-join)
# :   Returns a newsString containing the elements joined by the field
#     separator.
# [`to_a`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-to_a)
# :   Returns `self` or a new array containing all elements.
# [`to_ary`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-to_ary)
# :   Returns `self`.
# [`to_h`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-to_h)
# :   Returns a new hash formed from the elements.
# [`transpose`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-transpose)
# :   Transposes `self`, which must be an array of arrays.
# [`zip`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-zip)
# :   Returns a new array of arrays containing `self` and given arrays; follow
#     the link for details.
#
#
# ### Other Methods
#
# [`*`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-2A)
# :   Returns one of the following:
#     *   With integer argument `n`, a new array that is the concatenation of
#         `n` copies of `self`.
#     *   With string argument `field_separator`, a new string that is
#         equivalent to `join(field_separator)`.
#
# [`abbrev`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-abbrev)
# :   Returns a hash of unambiguous abbreviations for elements.
# [`pack`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-pack)
# :   Packs the elements into a binary sequence.
# [`sum`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-sum)
# :   Returns a sum of elements according to either `+` or a given block.
#
# for pack.c
class Array
  sig do
    returns(T::Set[Elem])
  end
  sig do
    type_parameters(:Return)
    .params(blk: T.nilable(T.proc.params(arg0: Elem).returns(T.type_parameter(:Return))))
    .returns(T::Set[T.type_parameter(:Return)])
  end
  def to_set(&blk); end
end

# ## What's Here
#
# [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html)
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) provides
# methods that are useful to a collection class for:
# *   [Querying](#module-Enumerable-label-Methods+for+Querying)
# *   [Fetching](#module-Enumerable-label-Methods+for+Fetching)
# *   [Searching](#module-Enumerable-label-Methods+for+Searching)
# *   [Sorting](#module-Enumerable-label-Methods+for+Sorting)
# *   [Iterating](#module-Enumerable-label-Methods+for+Iterating)
# *   [And more....](#module-Enumerable-label-Other+Methods)
#
#
# ### Methods for Querying
#
# These methods return information about the
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) other than
# the elements themselves:
#
# [`include?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-include-3F), [`member?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-member-3F)
# :   Returns `true` if self == object, `false` otherwise.
# [`all?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-all-3F)
# :   Returns `true` if all elements meet a specified criterion; `false`
#     otherwise.
# [`any?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-any-3F)
# :   Returns `true` if any element meets a specified criterion; `false`
#     otherwise.
# [`none?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-none-3F)
# :   Returns `true` if no element meets a specified criterion; `false`
#     otherwise.
# [`one?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-one-3F)
# :   Returns `true` if exactly one element meets a specified criterion; `false`
#     otherwise.
# [`count`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-count)
# :   Returns the count of elements, based on an argument or block criterion, if
#     given.
# [`tally`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-tally)
# :   Returns a new [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
#     containing the counts of occurrences of each element.
#
#
# ### Methods for Fetching
#
# These methods return entries from the
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html), without
# modifying it:
#
# *Leading, trailing, or all elements*:
# [`entries`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-entries), [`to_a`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-to_a)
# :   Returns all elements.
# [`first`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-first)
# :   Returns the first element or leading elements.
# [`take`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-take)
# :   Returns a specified number of leading elements.
# [`drop`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-drop)
# :   Returns a specified number of trailing elements.
# [`take_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-take_while)
# :   Returns leading elements as specified by the given block.
# [`drop_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-drop_while)
# :   Returns trailing elements as specified by the given block.
#
#
# *Minimum and maximum value elements*:
# [`min`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-min)
# :   Returns the elements whose values are smallest among the elements, as
#     determined by `<=>` or a given block.
# [`max`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-max)
# :   Returns the elements whose values are largest among the elements, as
#     determined by `<=>` or a given block.
# [`minmax`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-minmax)
# :   Returns a 2-element
#     [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) containing the
#     smallest and largest elements.
# [`min_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-min_by)
# :   Returns the smallest element, as determined by the given block.
# [`max_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-max_by)
# :   Returns the largest element, as determined by the given block.
# [`minmax_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-minmax_by)
# :   Returns the smallest and largest elements, as determined by the given
#     block.
#
#
# *Groups, slices, and partitions*:
# [`group_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-group_by)
# :   Returns a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) that
#     partitions the elements into groups.
# [`partition`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-partition)
# :   Returns elements partitioned into two new Arrays, as determined by the
#     given block.
# [`slice_after`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-slice_after)
# :   Returns a new
#     [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) whose
#     entries are a partition of `self`, based either on a given `object` or a
#     given block.
# [`slice_before`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-slice_before)
# :   Returns a new
#     [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) whose
#     entries are a partition of `self`, based either on a given `object` or a
#     given block.
# [`slice_when`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-slice_when)
# :   Returns a new
#     [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) whose
#     entries are a partition of `self` based on the given block.
# [`chunk`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-chunk)
# :   Returns elements organized into chunks as specified by the given block.
# [`chunk_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-chunk_while)
# :   Returns elements organized into chunks as specified by the given block.
#
#
# ### Methods for Searching and Filtering
#
# These methods return elements that meet a specified criterion.
#
# [`find`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-find), [`detect`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-detect)
# :   Returns an element selected by the block.
# [`find_all`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-find_all), [`filter`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-filter), [`select`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-select)
# :   Returns elements selected by the block.
# [`find_index`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-find_index)
# :   Returns the index of an element selected by a given object or block.
# [`reject`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-reject)
# :   Returns elements not rejected by the block.
# [`uniq`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-uniq)
# :   Returns elements that are not duplicates.
#
#
# ### Methods for Sorting
#
# These methods return elements in sorted order.
#
# [`sort`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort)
# :   Returns the elements, sorted by `<=>` or the given block.
# [`sort_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort_by)
# :   Returns the elements, sorted by the given block.
#
#
# ### Methods for Iterating
#
# [`each_entry`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-each_entry)
# :   Calls the block with each successive element (slightly different from
#     each).
# [`each_with_index`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-each_with_index)
# :   Calls the block with each successive element and its index.
# [`each_with_object`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-each_with_object)
# :   Calls the block with each successive element and a given object.
# [`each_slice`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-each_slice)
# :   Calls the block with successive non-overlapping slices.
# [`each_cons`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-each_cons)
# :   Calls the block with successive overlapping slices. (different from
#     [`each_slice`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-each_slice)).
# [`reverse_each`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-reverse_each)
# :   Calls the block with each successive element, in reverse order.
#
#
# ### Other Methods
#
# [`map`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-map), [`collect`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-collect)
# :   Returns objects returned by the block.
# [`filter_map`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-filter_map)
# :   Returns truthy objects returned by the block.
# [`flat_map`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-flat_map), [`collect_concat`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-collect_concat)
# :   Returns flattened objects returned by the block.
# [`grep`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-grep)
# :   Returns elements selected by a given object or objects returned by a given
#     block.
# [`grep_v`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-grep_v)
# :   Returns elements selected by a given object or objects returned by a given
#     block.
# [`reduce`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-reduce), [`inject`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-inject)
# :   Returns the object formed by combining all elements.
# [`sum`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sum)
# :   Returns the sum of the elements, using method +++.
# [`zip`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-zip)
# :   Combines each element with elements from other enumerables; returns the
#     n-tuples or calls the block with each.
# [`cycle`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-cycle)
# :   Calls the block with each element, cycling repeatedly.
#
#
# ## Usage
#
# To use module
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) in a
# collection class:
#
# *   Include it:
#
# ```ruby
# include Enumerable
# ```
#
# *   Implement method `#each` which must yield successive elements of the
#     collection. The method will be called by almost any
#     [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html)
#     method.
#
#
# Example:
#
# ```ruby
# class Foo
#   include Enumerable
#   def each
#     yield 1
#     yield 1, 2
#     yield
#   end
# end
# Foo.new.each_entry{ |element| p element }
# ```
#
# Output:
#
# ```ruby
# 1
# [1, 2]
# nil
# ```
#
# ## [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) in Ruby Core Classes
# Some Ruby classes include Enumerable:
# *   [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
# *   [`Dir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html)
# *   [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
# *   [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
# *   [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html)
# *   [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html)
# *   [`Struct`](https://docs.ruby-lang.org/en/2.7.0/Struct.html)
#
# Virtually all methods in
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) call
# method `#each` in the including class:
# *   `Hash#each` yields the next key-value pair as a 2-element
#     [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
# *   `Struct#each` yields the next name-value pair as a 2-element
#     [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
# *   For the other classes above, `#each` yields the next object from the
#     collection.
#
#
# ## About the Examples
# The example code snippets for the
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) methods:
# *   Always show the use of one or more Array-like classes (often
#     [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) itself).
# *   Sometimes show the use of a Hash-like class. For some methods, though, the
#     usage would not make sense, and so it is not shown. Example:
#     [`tally`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-tally)
#     would find exactly one of each
#     [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) entry.
module Enumerable
  # Makes a set from the enumerable object with given
  # arguments.
  # Needs
  # to `require "set"` to use this method.
  def to_set(klass = _, *args, &block); end
end
