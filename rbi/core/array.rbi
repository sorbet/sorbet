# typed: __STDLIB_INTERNAL

# Arrays are ordered, integer-indexed collections of any object.
#
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) indexing starts at
# 0, as in C or Java. A negative index is assumed to be relative to the end of
# the array---that is, an index of -1 indicates the last element of the array,
# -2 is the next to last element in the array, and so on.
#
# ## Creating Arrays
#
# A new array can be created by using the literal constructor `[]`. Arrays can
# contain different types of objects. For example, the array below contains an
# [`Integer`](https://docs.ruby-lang.org/en/2.6.0/Integer.html), a
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) and a Float:
#
# ```ruby
# ary = [1, "two", 3.0] #=> [1, "two", 3.0]
# ```
#
# An array can also be created by explicitly calling
# [`Array.new`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-c-new)
# with zero, one (the initial size of the
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)) or two arguments
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
# [`Kernel`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html), which tries to
# call
# [`to_ary`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-to_ary),
# then [`to_a`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-to_a) on
# its argument.
#
# ```ruby
# Array({:a => "a", :b => "b"}) #=> [[:a, "a"], [:b, "b"]]
# ```
#
# ## Example Usage
#
# In addition to the methods it mixes in through the
# [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) module,
# the [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) class has
# proprietary methods for accessing, searching and otherwise manipulating
# arrays.
#
# Some of the more common ones are illustrated below.
#
# ## Accessing Elements
#
# Elements in an array can be retrieved using the
# [`Array#[]`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-5B-5D)
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
# [`at`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-at) method
#
# ```ruby
# arr.at(0) #=> 1
# ```
#
# The [`slice`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-slice)
# method works in an identical manner to
# [`Array#[]`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-5B-5D).
#
# To raise an error for indices outside of the array bounds or else to provide a
# default value when that happens, you can use
# [`fetch`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-fetch).
#
# ```ruby
# arr = ['a', 'b', 'c', 'd', 'e', 'f']
# arr.fetch(100) #=> IndexError: index 100 outside of array bounds: -6...6
# arr.fetch(100, "oops") #=> "oops"
# ```
#
# The special methods
# [`first`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-first) and
# [`last`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-last) will
# return the first and last elements of an array, respectively.
#
# ```ruby
# arr.first #=> 1
# arr.last  #=> 6
# ```
#
# To return the first `n` elements of an array, use
# [`take`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-take)
#
# ```ruby
# arr.take(3) #=> [1, 2, 3]
# ```
#
# [`drop`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-drop) does
# the opposite of
# [`take`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-take), by
# returning the elements after `n` elements have been dropped:
#
# ```ruby
# arr.drop(3) #=> [4, 5, 6]
# ```
#
# ## Obtaining Information about an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
#
# Arrays keep track of their own length at all times. To query an array about
# the number of elements it contains, use
# [`length`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-length),
# [`count`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-count) or
# [`size`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-size).
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
# [`push`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-push) or
# [`<<`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-3C-3C)
#
# ```ruby
# arr = [1, 2, 3, 4]
# arr.push(5) #=> [1, 2, 3, 4, 5]
# arr << 6    #=> [1, 2, 3, 4, 5, 6]
# ```
#
# [`unshift`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-unshift)
# will add a new item to the beginning of an array.
#
# ```ruby
# arr.unshift(0) #=> [0, 1, 2, 3, 4, 5, 6]
# ```
#
# With
# [`insert`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-insert) you
# can add a new element to an array at any position.
#
# ```ruby
# arr.insert(3, 'apple')  #=> [0, 1, 2, 'apple', 3, 4, 5, 6]
# ```
#
# Using the
# [`insert`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-insert)
# method, you can also insert multiple values at once:
#
# ```ruby
# arr.insert(3, 'orange', 'pear', 'grapefruit')
# #=> [0, 1, 2, "orange", "pear", "grapefruit", "apple", 3, 4, 5, 6]
# ```
#
# ## Removing Items from an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
#
# The method
# [`pop`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-pop) removes
# the last element in an array and returns it:
#
# ```ruby
# arr =  [1, 2, 3, 4, 5, 6]
# arr.pop #=> 6
# arr #=> [1, 2, 3, 4, 5]
# ```
#
# To retrieve and at the same time remove the first item, use
# [`shift`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-shift):
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
# [`delete`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-delete):
#
# ```ruby
# arr = [1, 2, 2, 3]
# arr.delete(2) #=> 2
# arr #=> [1,3]
# ```
#
# A useful method if you need to remove `nil` values from an array is
# [`compact`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-compact):
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
# [`uniq`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-uniq), and
# destructive method
# [`uniq!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-uniq-21)
#
# ```ruby
# arr = [2, 5, 6, 556, 6, 6, 8, 9, 0, 123, 556]
# arr.uniq #=> [2, 5, 6, 556, 8, 9, 0, 123]
# ```
#
# ## Iterating over Arrays
#
# Like all classes that include the
# [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) module,
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) has an each method,
# which defines what elements should be iterated over and how. In case of
# Array's
# [`each`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-each), all
# elements in the [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
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
# [`reverse_each`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-reverse_each)
# which will iterate over the elements in the array in reverse order.
#
# ```ruby
# words = %w[first second third fourth fifth sixth]
# str = ""
# words.reverse_each {|word| str += "#{word} "}
# p str #=> "sixth fifth fourth third second first "
# ```
#
# The [`map`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-map)
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
# ## Selecting Items from an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
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
# [`select!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-select-21)
# and
# [`reject!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-reject-21)
# are the corresponding destructive methods to
# [`select`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-select) and
# [`reject`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-reject)
#
# Similar to
# [`select`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-select) vs.
# [`reject`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-reject),
# [`delete_if`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-delete_if)
# and
# [`keep_if`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-keep_if)
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
class Array < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  # Returns a new array populated with the given objects.
  #
  # ```ruby
  # Array.[]( 1, 'a', /^A/)  # => [1, "a", /^A/]
  # Array[ 1, 'a', /^A/ ]    # => [1, "a", /^A/]
  # [ 1, 'a', /^A/ ]         # => [1, "a", /^A/]
  # ```
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  def self.[](*arg0); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) Intersection ---
  # Returns a new array containing unique elements common to the two arrays. The
  # order is preserved from the original array.
  #
  # It compares elements using their
  # [`hash`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-hash) and
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-eql-3F)
  # methods for efficiency.
  #
  # ```ruby
  # [ 1, 1, 3, 5 ] & [ 3, 2, 1 ]                 #=> [ 1, 3 ]
  # [ 'a', 'b', 'b', 'z' ] & [ 'a', 'b', 'c' ]   #=> [ 'a', 'b' ]
  # ```
  #
  # See also
  # [`Array#uniq`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-uniq).
  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def &(arg0); end

  # Repetition --- With a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) argument,
  # equivalent to `ary.join(str)`.
  #
  # Otherwise, returns a new array built by concatenating the `int` copies of
  # `self`.
  #
  # ```ruby
  # [ 1, 2, 3 ] * 3    #=> [ 1, 2, 3, 1, 2, 3, 1, 2, 3 ]
  # [ 1, 2, 3 ] * ","  #=> "1,2,3"
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def *(arg0); end

  # Concatenation --- Returns a new array built by concatenating the two arrays
  # together to produce a third array.
  #
  # ```ruby
  # [ 1, 2, 3 ] + [ 4, 5 ]    #=> [ 1, 2, 3, 4, 5 ]
  # a = [ "a", "b", "c" ]
  # c = a + [ "d", "e", "f" ]
  # c                         #=> [ "a", "b", "c", "d", "e", "f" ]
  # a                         #=> [ "a", "b", "c" ]
  # ```
  #
  # Note that
  #
  # ```ruby
  # x += y
  # ```
  #
  # is the same as
  #
  # ```ruby
  # x = x + y
  # ```
  #
  # This means that it produces a new array. As a consequence, repeated use of
  # `+=` on arrays can be quite inefficient.
  #
  # See also
  # [`Array#concat`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-concat).
  sig do
    params(
        arg0: T::Enumerable[Elem],
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def +(arg0); end

  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) Difference
  #
  # Returns a new array that is a copy of the original array, removing any items
  # that also appear in `other_ary`. The order is preserved from the original
  # array.
  #
  # It compares elements using their
  # [`hash`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-hash) and
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-eql-3F)
  # methods for efficiency.
  #
  # ```ruby
  # [ 1, 1, 2, 2, 3, 3, 4, 5 ] - [ 1, 2, 4 ]  #=>  [ 3, 3, 5 ]
  # ```
  #
  # If you need set-like behavior, see the library class
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html).
  #
  # See also
  # [`Array#difference`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-difference).
  sig do
    params(
        arg0: T::Array[T.untyped],
    )
    .returns(T::Array[Elem])
  end
  def -(arg0); end

  # Append---Pushes the given object on to the end of this array. This
  # expression returns the array itself, so several appends may be chained
  # together.
  #
  # ```ruby
  # a = [ 1, 2 ]
  # a << "c" << "d" << [ 3, 4 ]
  #         #=>  [ 1, 2, "c", "d", [ 3, 4 ] ]
  # a
  #         #=>  [ 1, 2, "c", "d", [ 3, 4 ] ]
  # ```
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def <<(arg0); end

  # Element Reference --- Returns the element at `index`, or returns a subarray
  # starting at the `start` index and continuing for `length` elements, or
  # returns a subarray specified by `range` of indices.
  #
  # Negative indices count backward from the end of the array (-1 is the last
  # element). For `start` and `range` cases the starting index is just before an
  # element. Additionally, an empty array is returned when the starting index
  # for an element range is at the end of the array.
  #
  # Returns `nil` if the index (or starting index) are out of range.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d", "e" ]
  # a[2] +  a[0] + a[1]    #=> "cab"
  # a[6]                   #=> nil
  # a[1, 2]                #=> [ "b", "c" ]
  # a[1..3]                #=> [ "b", "c", "d" ]
  # a[4..7]                #=> [ "e" ]
  # a[6..10]               #=> nil
  # a[-3, 3]               #=> [ "c", "d", "e" ]
  # # special cases
  # a[5]                   #=> nil
  # a[6, 1]                #=> nil
  # a[5, 1]                #=> []
  # a[5..10]               #=> []
  # ```
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def [](arg0, arg1=T.unsafe(nil)); end

  # Element Assignment --- Sets the element at `index`, or replaces a subarray
  # from the `start` index for `length` elements, or replaces a subarray
  # specified by the `range` of indices.
  #
  # If indices are greater than the current capacity of the array, the array
  # grows automatically. Elements are inserted into the array at `start` if
  # `length` is zero.
  #
  # Negative indices will count backward from the end of the array. For `start`
  # and `range` cases the starting index is just before an element.
  #
  # An [`IndexError`](https://docs.ruby-lang.org/en/2.6.0/IndexError.html) is
  # raised if a negative index points past the beginning of the array.
  #
  # See also
  # [`Array#push`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-push),
  # and
  # [`Array#unshift`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-unshift).
  #
  # ```ruby
  # a = Array.new
  # a[4] = "4";                 #=> [nil, nil, nil, nil, "4"]
  # a[0, 3] = [ 'a', 'b', 'c' ] #=> ["a", "b", "c", nil, "4"]
  # a[1..2] = [ 1, 2 ]          #=> ["a", 1, 2, nil, "4"]
  # a[0, 2] = "?"               #=> ["?", 2, nil, "4"]
  # a[0..2] = "A"               #=> ["A", "4"]
  # a[-1]   = "Z"               #=> ["A", "Z"]
  # a[1..-1] = nil              #=> ["A", nil]
  # a[1..-1] = []               #=> ["A"]
  # a[0, 0] = [ 1, 2 ]          #=> [1, 2, "A"]
  # a[3, 0] = "B"               #=> [1, 2, "A", "B"]
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
        arg2: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: T::Range[Integer],
        arg1: Elem,
    )
    .returns(Elem)
  end
  def []=(arg0, arg1, arg2=T.unsafe(nil)); end

  # Alias for:
  # [`push`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-push)
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def append(*arg0); end

  # Searches through an array whose elements are also arrays comparing `obj`
  # with the first element of each contained array using `obj.==`.
  #
  # Returns the first contained array that matches (that is, the first
  # associated array), or `nil` if no match is found.
  #
  # See also
  # [`Array#rassoc`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-rassoc)
  #
  # ```ruby
  # s1 = [ "colors", "red", "blue", "green" ]
  # s2 = [ "letters", "a", "b", "c" ]
  # s3 = "foo"
  # a  = [ s1, s2, s3 ]
  # a.assoc("letters")  #=> [ "letters", "a", "b", "c" ]
  # a.assoc("foo")      #=> nil
  # ```
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def assoc(arg0); end

  # Returns the element at `index`. A negative index counts from the end of
  # `self`. Returns `nil` if the index is out of range. See also
  # [`Array#[]`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-5B-5D).
  #
  # ```ruby
  # a = [ "a", "b", "c", "d", "e" ]
  # a.at(0)     #=> "a"
  # a.at(-1)    #=> "e"
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Elem)
  end
  def at(arg0); end

  # Removes all elements from `self`.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d", "e" ]
  # a.clear    #=> [ ]
  # ```
  sig {returns(T::Array[Elem])}
  def clear(); end

  # Invokes the given block once for each element of `self`.
  #
  # Creates a new array containing the values returned by the block.
  #
  # See also
  # [`Enumerable#collect`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-collect).
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d" ]
  # a.collect {|x| x + "!"}           #=> ["a!", "b!", "c!", "d!"]
  # a.map.with_index {|x, i| x * i}   #=> ["", "b", "cc", "ddd"]
  # a                                 #=> ["a", "b", "c", "d"]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(T::Enumerator[Elem])}
  def collect(&blk); end

  # When invoked with a block, yields all combinations of length `n` of elements
  # from the array and then returns the array itself.
  #
  # The implementation makes no guarantees about the order in which the
  # combinations are yielded.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # Examples:
  #
  # ```ruby
  # a = [1, 2, 3, 4]
  # a.combination(1).to_a  #=> [[1],[2],[3],[4]]
  # a.combination(2).to_a  #=> [[1,2],[1,3],[1,4],[2,3],[2,4],[3,4]]
  # a.combination(3).to_a  #=> [[1,2,3],[1,2,4],[1,3,4],[2,3,4]]
  # a.combination(4).to_a  #=> [[1,2,3,4]]
  # a.combination(0).to_a  #=> [[]] # one combination of length 0
  # a.combination(5).to_a  #=> []   # no combinations of length 5
  # ```
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Enumerator[T::Array[Elem]])
  end
  def combination(arg0, &blk); end

  ### This is implemented in C++ to fix the return type

  # Returns a copy of `self` with all `nil` elements removed.
  #
  # ```ruby
  # [ "a", nil, "b", nil, "c", nil ].compact
  #                   #=> [ "a", "b", "c" ]
  # ```
  sig {returns(T::Array[T.untyped])}
  def compact(); end

  # Removes `nil` elements from the array.
  #
  # Returns `nil` if no changes were made, otherwise returns the array.
  #
  # ```ruby
  # [ "a", nil, "b", nil, "c" ].compact! #=> [ "a", "b", "c" ]
  # [ "a", "b", "c" ].compact!           #=> nil
  # ```
  sig {returns(T::Array[Elem])}
  def compact!(); end

  # Appends the elements of `other_ary`s to `self`.
  #
  # ```ruby
  # [ "a", "b" ].concat( ["c", "d"])   #=> [ "a", "b", "c", "d" ]
  # [ "a" ].concat( ["b"], ["c", "d"]) #=> [ "a", "b", "c", "d" ]
  # [ "a" ].concat #=> [ "a" ]
  #
  # a = [ 1, 2, 3 ]
  # a.concat( [ 4, 5 ])
  # a                                 #=> [ 1, 2, 3, 4, 5 ]
  #
  # a = [ 1, 2 ]
  # a.concat(a, a)                    #=> [1, 2, 1, 2, 1, 2]
  # ```
  #
  # See also
  # [`Array#+`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-2B).
  sig do
    type_parameters(:T).params(
        arrays: T::Array[T.type_parameter(:T)],
    )
    .returns(T::Array[T.any(Elem, T.type_parameter(:T))])
  end
  def concat(*arrays); end

  # Returns the number of elements.
  #
  # If an argument is given, counts the number of elements which equal `obj`
  # using `==`.
  #
  # If a block is given, counts the number of elements for which the block
  # returns a true value.
  #
  # ```ruby
  # ary = [1, 2, 4, 2]
  # ary.count                  #=> 4
  # ary.count(2)               #=> 2
  # ary.count {|x| x%2 == 0}   #=> 3
  # ```
  sig {returns(Integer)}
  sig do
    params(
        arg0: Elem,
    )
    .returns(Integer)
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Integer)
  end
  def count(arg0=T.unsafe(nil), &blk); end

  # Calls the given block for each element `n` times or forever if `nil` is
  # given.
  #
  # Does nothing if a non-positive number is given or the array is empty.
  #
  # Returns `nil` if the loop has finished without getting interrupted.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # a = ["a", "b", "c"]
  # a.cycle {|x| puts x}       # print, a, b, c, a, b, c,.. forever.
  # a.cycle(2) {|x| puts x}    # print, a, b, c, a, b, c.
  # ```
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Enumerator[Elem])
  end
  def cycle(arg0=T.unsafe(nil), &blk); end

  # Deletes all items from `self` that are equal to `obj`.
  #
  # Returns the last deleted item, or `nil` if no matching item is found.
  #
  # If the optional code block is given, the result of the block is returned if
  # the item is not found. (To remove `nil` elements and get an informative
  # return value, use
  # [`Array#compact!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-compact-21))
  #
  # ```ruby
  # a = [ "a", "b", "b", "b", "c" ]
  # a.delete("b")                   #=> "b"
  # a                               #=> ["a", "c"]
  # a.delete("z")                   #=> nil
  # a.delete("z") {"not found"}     #=> "not found"
  # ```
  sig do
    params(
        arg0: Elem,
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Elem,
        blk: T.proc.params().returns(Elem),
    )
    .returns(Elem)
  end
  def delete(arg0, &blk); end

  # Deletes the element at the specified `index`, returning that element, or
  # `nil` if the `index` is out of range.
  #
  # See also
  # [`Array#slice!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-slice-21)
  #
  # ```ruby
  # a = ["ant", "bat", "cat", "dog"]
  # a.delete_at(2)    #=> "cat"
  # a                 #=> ["ant", "bat", "dog"]
  # a.delete_at(99)   #=> nil
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T.nilable(Elem))
  end
  def delete_at(arg0); end

  # Deletes every element of `self` for which block evaluates to `true`.
  #
  # The array is changed instantly every time the block is called, not after the
  # iteration is over.
  #
  # See also
  # [`Array#reject!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-reject-21)
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # scores = [ 97, 42, 75 ]
  # scores.delete_if {|score| score < 80 }   #=> [97]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def delete_if(&blk); end

  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) Difference
  #
  # Returns a new array that is a copy of the receiver, removing any items that
  # also appear in any of the arrays given as arguments. The order is preserved
  # from the original array.
  #
  # It compares elements using their
  # [`hash`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-hash) and
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-eql-3F)
  # methods for efficiency.
  #
  # ```ruby
  # [ 1, 1, 2, 2, 3, 3, 4, 5 ].difference([ 1, 2, 4 ])     #=> [ 3, 3, 5 ]
  # [ 1, 'c', :s, 'yep' ].difference([ 1 ], [ 'a', 'c' ])  #=> [ :s, "yep" ]
  # ```
  #
  # If you need set-like behavior, see the library class
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html).
  #
  # See also
  # [`Array#-`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-2D).
  sig do
    params(
        arrays: T::Array[T.untyped]
    )
    .returns(T::Array[Elem])
  end
  def difference(*arrays); end

  # Drops first `n` elements from `ary` and returns the rest of the elements in
  # an array.
  #
  # If a negative number is given, raises an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html).
  #
  # See also
  # [`Array#take`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-take)
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.drop(3)             #=> [4, 5, 0]
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def drop(arg0); end

  # Drops elements up to, but not including, the first element for which the
  # block returns `nil` or `false` and returns an array containing the remaining
  # elements.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # See also
  # [`Array#take_while`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-take_while)
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.drop_while {|i| i < 3 }   #=> [3, 4, 5, 0]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def drop_while(&blk); end

  # Calls the given block once for each element in `self`, passing that element
  # as a parameter. Returns the array itself.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.each {|x| print x, " -- " }
  # ```
  #
  # produces:
  #
  # ```
  # a -- b -- c --
  # ```
  sig {returns(T::Enumerator[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  def each(&blk); end

  # Same as
  # [`Array#each`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-each),
  # but passes the `index` of the element instead of the element itself.
  #
  # An [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned if no block is given.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.each_index {|x| print x, " -- " }
  # ```
  #
  # produces:
  #
  # ```
  # 0 -- 1 -- 2 --
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def each_index(&blk); end

  # Returns `true` if `self` contains no elements.
  #
  # ```ruby
  # [].empty?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def empty?(); end

  # Tries to return the element at position `index`, but throws an
  # [`IndexError`](https://docs.ruby-lang.org/en/2.6.0/IndexError.html)
  # exception if the referenced `index` lies outside of the array bounds. This
  # error can be prevented by supplying a second argument, which will act as a
  # `default` value.
  #
  # Alternatively, if a block is given it will only be executed when an invalid
  # `index` is referenced.
  #
  # Negative values of `index` count from the end of the array.
  #
  # ```ruby
  # a = [ 11, 22, 33, 44 ]
  # a.fetch(1)               #=> 22
  # a.fetch(-1)              #=> 44
  # a.fetch(4, 'cat')        #=> "cat"
  # a.fetch(100) {|i| puts "#{i} is out of bounds"}
  #                          #=> "100 is out of bounds"
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(Elem)
  end
  def fetch(arg0, arg1=T.unsafe(nil), &blk); end

  # The first three forms set the selected elements of `self` (which may be the
  # entire array) to `obj`.
  #
  # A `start` of `nil` is equivalent to zero.
  #
  # A `length` of `nil` is equivalent to the length of the array.
  #
  # The last three forms fill the array with the value of the given block, which
  # is passed the absolute index of each element to be filled.
  #
  # Negative values of `start` count from the end of the array, where `-1` is
  # the last element.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d" ]
  # a.fill("x")              #=> ["x", "x", "x", "x"]
  # a.fill("z", 2, 2)        #=> ["x", "x", "z", "z"]
  # a.fill("y", 0..1)        #=> ["y", "y", "z", "z"]
  # a.fill {|i| i*i}         #=> [0, 1, 4, 9]
  # a.fill(-2) {|i| i*i*i}   #=> [0, 1, 8, 27]
  # ```
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Elem,
        arg1: Integer,
        arg2: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Elem,
        arg1: T::Range[Integer],
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: T::Range[Integer],
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(T::Array[Elem])
  end
  def fill(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil), &blk); end

  # Returns the first element, or the first `n` elements, of the array. If the
  # array is empty, the first form returns `nil`, and the second form returns an
  # empty array. See also
  # [`Array#last`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-last)
  # for the opposite effect.
  #
  # ```ruby
  # a = [ "q", "r", "s", "t" ]
  # a.first     #=> "q"
  # a.first(2)  #=> ["q", "r"]
  # ```
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def first(arg0=T.unsafe(nil)); end

  ### This is implemented in C++ to fix the return type

  # Returns a new array that is a one-dimensional flattening of `self`
  # (recursively).
  #
  # That is, for every element that is an array, extract its elements into the
  # new array.
  #
  # The optional `level` argument determines the level of recursion to flatten.
  #
  # ```ruby
  # s = [ 1, 2, 3 ]           #=> [1, 2, 3]
  # t = [ 4, 5, 6, [7, 8] ]   #=> [4, 5, 6, [7, 8]]
  # a = [ s, t, 9, 10 ]       #=> [[1, 2, 3], [4, 5, 6, [7, 8]], 9, 10]
  # a.flatten                 #=> [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
  # a = [ 1, 2, [3, [4, 5] ] ]
  # a.flatten(1)              #=> [1, 2, 3, [4, 5]]
  # ```
  sig {params(depth: Integer).returns(T::Array[T.untyped])}
  def flatten(depth = -1); end

  # Returns `true` if the given `object` is present in `self` (that is, if any
  # element `==` `object`), otherwise returns `false`.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.include?("b")   #=> true
  # a.include?("z")   #=> false
  # ```
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end

  # Returns the *index* of the first object in `ary` such that the object is
  # `==` to `obj`.
  #
  # If a block is given instead of an argument, returns the *index* of the first
  # object for which the block returns `true`. Returns `nil` if no match is
  # found.
  #
  # See also
  # [`Array#rindex`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-rindex).
  #
  # An [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned if neither a block nor argument is given.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.index("b")              #=> 1
  # a.index("z")              #=> nil
  # a.index {|x| x == "b"}    #=> 1
  # ```
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T.nilable(Integer))
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Integer))
  end
  sig {returns(T::Enumerator[Elem])}
  def index(arg0=T.unsafe(nil), &blk); end

  sig {returns(Object)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Object)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  # Inserts the given values before the element with the given `index`.
  #
  # Negative indices count backwards from the end of the array, where `-1` is
  # the last element. If a negative index is used, the given values will be
  # inserted after that element, so using an index of `-1` will insert the
  # values at the end of the array.
  #
  # ```ruby
  # a = %w{ a b c d }
  # a.insert(2, 99)         #=> ["a", "b", 99, "c", "d"]
  # a.insert(-2, 1, 2, 3)   #=> ["a", "b", 99, "c", 1, 2, 3, "d"]
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .returns(T::Array[Elem])
  end
  def insert(arg0, *arg1); end

  # Creates a string representation of `self`.
  #
  # ```ruby
  # [ "a", "b", "c" ].to_s     #=> "[\"a\", \"b\", \"c\"]"
  # ```
  #
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-to_s)
  sig {returns(String)}
  def inspect(); end

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

  # Deletes every element of `self` for which the given block evaluates to
  # `false`, and returns `self`.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # a = %w[ a b c d e f ]
  # a.keep_if {|v| v =~ /[aeiou]/ }    #=> ["a", "e"]
  # a                                  #=> ["a", "e"]
  # ```
  #
  # See also
  # [`Array#select!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-select-21).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  def keep_if(&blk); end

  # Returns the last element(s) of `self`. If the array is empty, the first form
  # returns `nil`.
  #
  # See also
  # [`Array#first`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-first)
  # for the opposite effect.
  #
  # ```ruby
  # a = [ "w", "x", "y", "z" ]
  # a.last     #=> "z"
  # a.last(2)  #=> ["y", "z"]
  # ```
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def last(arg0=T.unsafe(nil)); end

  # Returns the number of elements in `self`. May be zero.
  #
  # ```ruby
  # [ 1, 2, 3, 4, 5 ].length   #=> 5
  # [].length                  #=> 0
  # ```
  #
  #
  # Also aliased as:
  # [`size`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-size)
  sig {returns(Integer)}
  def length(); end

  # Invokes the given block once for each element of `self`.
  #
  # Creates a new array containing the values returned by the block.
  #
  # See also
  # [`Enumerable#collect`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-collect).
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d" ]
  # a.collect {|x| x + "!"}           #=> ["a!", "b!", "c!", "d!"]
  # a.map.with_index {|x, i| x * i}   #=> ["", "b", "cc", "ddd"]
  # a                                 #=> ["a", "b", "c", "d"]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(T::Enumerator[Elem])}
  def map(&blk); end

  # Invokes the given block once for each element of `self`, replacing the
  # element with the value returned by the block.
  #
  # See also
  # [`Enumerable#collect`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-collect).
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d" ]
  # a.map! {|x| x + "!" }
  # a #=>  [ "a!", "b!", "c!", "d!" ]
  # a.collect!.with_index {|x, i| x[0...i] }
  # a #=>  ["", "b", "c!", "d!"]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(T::Enumerator[Elem])}
  def map!(&blk); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Boolean)
  end
  def member?(arg0); end

  # When invoked with a block, yield all permutations of length `n` of the
  # elements of the array, then return the array itself.
  #
  # If `n` is not specified, yield all permutations of all elements.
  #
  # The implementation makes no guarantees about the order in which the
  # permutations are yielded.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # Examples:
  #
  # ```ruby
  # a = [1, 2, 3]
  # a.permutation.to_a    #=> [[1,2,3],[1,3,2],[2,1,3],[2,3,1],[3,1,2],[3,2,1]]
  # a.permutation(1).to_a #=> [[1],[2],[3]]
  # a.permutation(2).to_a #=> [[1,2],[1,3],[2,1],[2,3],[3,1],[3,2]]
  # a.permutation(3).to_a #=> [[1,2,3],[1,3,2],[2,1,3],[2,3,1],[3,1,2],[3,2,1]]
  # a.permutation(0).to_a #=> [[]] # one permutation of length 0
  # a.permutation(4).to_a #=> []   # no permutations of length 4
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Enumerator[T::Array[Elem]])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  def permutation(arg0=T.unsafe(nil), &blk); end

  # Removes the last element from `self` and returns it, or `nil` if the array
  # is empty.
  #
  # If a number `n` is given, returns an array of the last `n` elements (or
  # less) just like `array.slice!(-n, n)` does. See also
  # [`Array#push`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-push)
  # for the opposite effect.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d" ]
  # a.pop     #=> "d"
  # a.pop(2)  #=> ["b", "c"]
  # a         #=> ["a"]
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T.nilable(Elem))}
  def pop(arg0=T.unsafe(nil)); end

  # Alias for:
  # [`unshift`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-unshift)
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def prepend(*arg0); end

  # Returns an array of all combinations of elements from all arrays.
  #
  # The length of the returned array is the product of the length of `self` and
  # the argument arrays.
  #
  # If given a block,
  # [`product`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-product)
  # will yield all combinations and return `self` instead.
  #
  # ```ruby
  # [1,2,3].product([4,5])     #=> [[1,4],[1,5],[2,4],[2,5],[3,4],[3,5]]
  # [1,2].product([1,2])       #=> [[1,1],[1,2],[2,1],[2,2]]
  # [1,2].product([3,4],[5,6]) #=> [[1,3,5],[1,3,6],[1,4,5],[1,4,6],
  #                            #     [2,3,5],[2,3,6],[2,4,5],[2,4,6]]
  # [1,2].product()            #=> [[1],[2]]
  # [1,2].product([])          #=> []
  # ```
  #
  # (Note: This sig is approximate. Sorbet has special handling for this method.
  sig {params(arg: T::Array[T.untyped]).returns(T::Array[T.untyped])}
  def product(*arg); end

  # Append --- Pushes the given object(s) on to the end of this array. This
  # expression returns the array itself, so several appends may be chained
  # together. See also
  # [`Array#pop`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-pop)
  # for the opposite effect.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.push("d", "e", "f")
  #         #=> ["a", "b", "c", "d", "e", "f"]
  # [1, 2, 3].push(4).push(5)
  #         #=> [1, 2, 3, 4, 5]
  # ```
  #
  #
  # Also aliased as:
  # [`append`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-append)
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def push(*arg0); end

  # Searches through the array whose elements are also arrays.
  #
  # Compares `obj` with the second element of each contained array using
  # `obj.==`.
  #
  # Returns the first contained array that matches `obj`.
  #
  # See also
  # [`Array#assoc`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-assoc).
  #
  # ```ruby
  # a = [ [ 1, "one"], [2, "two"], [3, "three"], ["ii", "two"] ]
  # a.rassoc("two")    #=> [2, "two"]
  # a.rassoc("four")   #=> nil
  # ```
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T.nilable(Elem))
  end
  def rassoc(arg0); end

  # Returns a new array containing the items in `self` for which the given block
  # is not `true`. The ordering of non-rejected elements is maintained.
  #
  # See also
  # [`Array#delete_if`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-delete_if)
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def reject(&blk); end

  # Deletes every element of `self` for which the block evaluates to `true`, if
  # no changes were made returns `nil`.
  #
  # The array may not be changed instantly every time the block is called.
  #
  # See also
  # [`Enumerable#reject`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-reject)
  # and
  # [`Array#delete_if`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-delete_if).
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def reject!(&blk); end

  # When invoked with a block, yields all repeated combinations of length `n` of
  # elements from the array and then returns the array itself.
  #
  # The implementation makes no guarantees about the order in which the repeated
  # combinations are yielded.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # Examples:
  #
  # ```ruby
  # a = [1, 2, 3]
  # a.repeated_combination(1).to_a  #=> [[1], [2], [3]]
  # a.repeated_combination(2).to_a  #=> [[1,1],[1,2],[1,3],[2,2],[2,3],[3,3]]
  # a.repeated_combination(3).to_a  #=> [[1,1,1],[1,1,2],[1,1,3],[1,2,2],[1,2,3],
  #                                 #    [1,3,3],[2,2,2],[2,2,3],[2,3,3],[3,3,3]]
  # a.repeated_combination(4).to_a  #=> [[1,1,1,1],[1,1,1,2],[1,1,1,3],[1,1,2,2],[1,1,2,3],
  #                                 #    [1,1,3,3],[1,2,2,2],[1,2,2,3],[1,2,3,3],[1,3,3,3],
  #                                 #    [2,2,2,2],[2,2,2,3],[2,2,3,3],[2,3,3,3],[3,3,3,3]]
  # a.repeated_combination(0).to_a  #=> [[]] # one combination of length 0
  # ```
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Enumerator[T::Array[Elem]])
  end
  def repeated_combination(arg0, &blk); end

  # When invoked with a block, yield all repeated permutations of length `n` of
  # the elements of the array, then return the array itself.
  #
  # The implementation makes no guarantees about the order in which the repeated
  # permutations are yielded.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # Examples:
  #
  # ```ruby
  # a = [1, 2]
  # a.repeated_permutation(1).to_a  #=> [[1], [2]]
  # a.repeated_permutation(2).to_a  #=> [[1,1],[1,2],[2,1],[2,2]]
  # a.repeated_permutation(3).to_a  #=> [[1,1,1],[1,1,2],[1,2,1],[1,2,2],
  #                                 #    [2,1,1],[2,1,2],[2,2,1],[2,2,2]]
  # a.repeated_permutation(0).to_a  #=> [[]] # one permutation of length 0
  # ```
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Enumerator[T::Array[Elem]])
  end
  def repeated_permutation(arg0, &blk); end

  # Returns a new array containing `self`'s elements in reverse order.
  #
  # ```ruby
  # [ "a", "b", "c" ].reverse   #=> ["c", "b", "a"]
  # [ 1 ].reverse               #=> [1]
  # ```
  sig {returns(T::Array[Elem])}
  def reverse(); end

  # Reverses `self` in place.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.reverse!       #=> ["c", "b", "a"]
  # a                #=> ["c", "b", "a"]
  # ```
  sig {returns(T::Array[Elem])}
  def reverse!(); end

  # Same as
  # [`Array#each`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-each),
  # but traverses `self` in reverse order.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.reverse_each {|x| print x, " " }
  # ```
  #
  # produces:
  #
  # ```ruby
  # c b a
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def reverse_each(&blk); end

  # Returns the *index* of the last object in `self` `==` to `obj`.
  #
  # If a block is given instead of an argument, returns the *index* of the first
  # object for which the block returns `true`, starting from the last object.
  #
  # Returns `nil` if no match is found.
  #
  # See also
  # [`Array#index`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-index).
  #
  # If neither block nor argument is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # a = [ "a", "b", "b", "b", "c" ]
  # a.rindex("b")             #=> 3
  # a.rindex("z")             #=> nil
  # a.rindex {|x| x == "b"}   #=> 3
  # ```
  sig do
    params(
        arg0: Elem,
    )
    .returns(T.nilable(Integer))
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Integer))
  end
  sig {returns(T::Enumerator[Elem])}
  def rindex(arg0=T.unsafe(nil), &blk); end

  # Returns a new array by rotating `self` so that the element at `count` is the
  # first element of the new array.
  #
  # If `count` is negative then it rotates in the opposite direction, starting
  # from the end of `self` where `-1` is the last element.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d" ]
  # a.rotate         #=> ["b", "c", "d", "a"]
  # a                #=> ["a", "b", "c", "d"]
  # a.rotate(2)      #=> ["c", "d", "a", "b"]
  # a.rotate(-3)     #=> ["b", "c", "d", "a"]
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def rotate(arg0=T.unsafe(nil)); end

  # Rotates `self` in place so that the element at `count` comes first, and
  # returns `self`.
  #
  # If `count` is negative then it rotates in the opposite direction, starting
  # from the end of the array where `-1` is the last element.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d" ]
  # a.rotate!        #=> ["b", "c", "d", "a"]
  # a                #=> ["b", "c", "d", "a"]
  # a.rotate!(2)     #=> ["d", "a", "b", "c"]
  # a.rotate!(-3)    #=> ["a", "b", "c", "d"]
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def rotate!(arg0=T.unsafe(nil)); end

  # Choose a random element or `n` random elements from the array.
  #
  # The elements are chosen by using random and unique indices into the array in
  # order to ensure that an element doesn't repeat itself unless the array
  # already contained duplicate elements.
  #
  # If the array is empty the first form returns `nil` and the second form
  # returns an empty array.
  #
  # ```ruby
  # a = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]
  # a.sample         #=> 7
  # a.sample(4)      #=> [6, 4, 2, 5]
  # ```
  #
  # The optional `rng` argument will be used as the random number generator.
  #
  # ```ruby
  # a.sample(random: Random.new(1))     #=> 6
  # a.sample(4, random: Random.new(1))  #=> [6, 10, 9, 2]
  # ```
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def sample(arg0=T.unsafe(nil)); end

  # Returns a new array containing all elements of `ary` for which the given
  # `block` returns a true value.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # [1,2,3,4,5].select {|num| num.even? }     #=> [2, 4]
  #
  # a = %w[ a b c d e f ]
  # a.select {|v| v =~ /[aeiou]/ }    #=> ["a", "e"]
  # ```
  #
  # See also
  # [`Enumerable#select`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-select).
  #
  # [`Array#filter`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-filter)
  # is an alias for
  # [`Array#select`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-select).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def select(&blk); end

  # Invokes the given block passing in successive elements from `self`, deleting
  # elements for which the block returns a `false` value.
  #
  # The array may not be changed instantly every time the block is called.
  #
  # If changes were made, it will return `self`, otherwise it returns `nil`.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # See also
  # [`Array#keep_if`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-keep_if).
  #
  # [`Array#filter!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-filter-21)
  # is an alias for
  # [`Array#select!`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-select-21).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def select!(&blk); end

  # Removes the first element of `self` and returns it (shifting all other
  # elements down by one). Returns `nil` if the array is empty.
  #
  # If a number `n` is given, returns an array of the first `n` elements (or
  # less) just like `array.slice!(0, n)` does. With `ary` containing only the
  # remainder elements, not including what was shifted to `new_ary`. See also
  # [`Array#unshift`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-unshift)
  # for the opposite effect.
  #
  # ```ruby
  # args = [ "-m", "-q", "filename" ]
  # args.shift     #=> "-m"
  # args           #=> ["-q", "filename"]
  #
  # args = [ "-m", "-q", "filename" ]
  # args.shift(2)  #=> ["-m", "-q"]
  # args           #=> ["filename"]
  # ```
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def shift(arg0=T.unsafe(nil)); end

  # Returns a new array with elements of `self` shuffled.
  #
  # ```ruby
  # a = [ 1, 2, 3 ]           #=> [1, 2, 3]
  # a.shuffle                 #=> [2, 3, 1]
  # a                         #=> [1, 2, 3]
  # ```
  #
  # The optional `rng` argument will be used as the random number generator.
  #
  # ```ruby
  # a.shuffle(random: Random.new(1))  #=> [1, 3, 2]
  # ```
  sig {returns(T::Array[Elem])}
  def shuffle(); end

  # Shuffles elements in `self` in place.
  #
  # ```ruby
  # a = [ 1, 2, 3 ]           #=> [1, 2, 3]
  # a.shuffle!                #=> [2, 3, 1]
  # a                         #=> [2, 3, 1]
  # ```
  #
  # The optional `rng` argument will be used as the random number generator.
  #
  # ```ruby
  # a.shuffle!(random: Random.new(1))  #=> [1, 3, 2]
  # ```
  sig {returns(T::Array[Elem])}
  def shuffle!(); end

  # Deletes the element(s) given by an `index` (optionally up to `length`
  # elements) or by a `range`.
  #
  # Returns the deleted object (or objects), or `nil` if the `index` is out of
  # range.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.slice!(1)     #=> "b"
  # a               #=> ["a", "c"]
  # a.slice!(-1)    #=> "c"
  # a               #=> ["a"]
  # a.slice!(100)   #=> nil
  # a               #=> ["a"]
  # ```
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.nilable(Elem))
  end
  def slice!(arg0, arg1=T.unsafe(nil)); end

  # Returns a new array created by sorting `self`.
  #
  # Comparisons for the sort will be done using the `<=>` operator or using an
  # optional code block.
  #
  # The block must implement a comparison between `a` and `b` and return an
  # integer less than 0 when `b` follows `a`, `0` when `a` and `b` are
  # equivalent, or an integer greater than 0 when `a` follows `b`.
  #
  # The result is not guaranteed to be stable. When the comparison of two
  # elements returns `0`, the order of the elements is unpredictable.
  #
  # ```ruby
  # ary = [ "d", "a", "e", "c", "b" ]
  # ary.sort                     #=> ["a", "b", "c", "d", "e"]
  # ary.sort {|a, b| b <=> a}    #=> ["e", "d", "c", "b", "a"]
  # ```
  #
  # See also
  # [`Enumerable#sort_by`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-sort_by).
  sig {returns(T::Array[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def sort(&blk); end

  # Sorts `self` in place.
  #
  # Comparisons for the sort will be done using the `<=>` operator or using an
  # optional code block.
  #
  # The block must implement a comparison between `a` and `b` and return an
  # integer less than 0 when `b` follows `a`, `0` when `a` and `b` are
  # equivalent, or an integer greater than 0 when `a` follows `b`.
  #
  # The result is not guaranteed to be stable. When the comparison of two
  # elements returns `0`, the order of the elements is unpredictable.
  #
  # ```ruby
  # ary = [ "d", "a", "e", "c", "b" ]
  # ary.sort!                     #=> ["a", "b", "c", "d", "e"]
  # ary.sort! {|a, b| b <=> a}    #=> ["e", "d", "c", "b", "a"]
  # ```
  #
  # See also
  # [`Enumerable#sort_by`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-sort_by).
  sig {returns(T::Array[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def sort!(&blk); end

  # Sorts `self` in place using a set of keys generated by mapping the values in
  # `self` through the given block.
  #
  # The result is not guaranteed to be stable. When two keys are equal, the
  # order of the corresponding elements is unpredictable.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # See also
  # [`Enumerable#sort_by`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-sort_by).
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def sort_by!(&blk); end

  # Returns first `n` elements from the array.
  #
  # If a negative number is given, raises an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html).
  #
  # See also
  # [`Array#drop`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-drop)
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.take(3)             #=> [1, 2, 3]
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def take(arg0); end

  # Passes elements to the block until the block returns `nil` or `false`, then
  # stops iterating and returns an array of all prior elements.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # See also
  # [`Array#drop_while`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-drop_while)
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.take_while {|i| i < 3}    #=> [1, 2]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def take_while(&blk); end

  # Returns `self`.
  #
  # If called on a subclass of
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html), converts the
  # receiver to an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
  # object.
  sig {returns(T::Array[Elem])}
  def to_a(); end

  # Returns `self`.
  sig {returns(T::Array[Elem])}
  def to_ary(); end

  # Assumes that `self` is an array of arrays and transposes the rows and
  # columns.
  #
  # ```ruby
  # a = [[1,2], [3,4], [5,6]]
  # a.transpose   #=> [[1, 3, 5], [2, 4, 6]]
  # ```
  #
  # If the length of the subarrays don't match, an
  # [`IndexError`](https://docs.ruby-lang.org/en/2.6.0/IndexError.html) is
  # raised.
  sig {returns(T::Array[Elem])}
  def transpose(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) Union --- Returns a
  # new array by joining `other_ary`s with `self`, excluding any duplicates and
  # preserving the order from the given arrays.
  #
  # It compares elements using their
  # [`hash`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-hash) and
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-eql-3F)
  # methods for efficiency.
  #
  # ```ruby
  # [ "a", "b", "c" ].union( [ "c", "d", "a" ] )    #=> [ "a", "b", "c", "d" ]
  # [ "a" ].union( ["e", "b"], ["a", "c", "b"] )    #=> [ "a", "e", "b", "c" ]
  # [ "a" ].union #=> [ "a" ]
  # ```
  #
  # See also Array#|.
  sig do
    params(
        arrays: T::Array[T.untyped]
    )
    .returns(T::Array[T.untyped])
  end
  def union(*arrays); end

  # Returns a new array by removing duplicate values in `self`.
  #
  # If a block is given, it will use the return value of the block for
  # comparison.
  #
  # It compares values using their
  # [`hash`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-hash) and
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-eql-3F)
  # methods for efficiency.
  #
  # `self` is traversed in order, and the first occurrence is kept.
  #
  # ```ruby
  # a = [ "a", "a", "b", "b", "c" ]
  # a.uniq   # => ["a", "b", "c"]
  #
  # b = [["student","sam"], ["student","george"], ["teacher","matz"]]
  # b.uniq {|s| s.first}   # => [["student", "sam"], ["teacher", "matz"]]
  # ```
  sig {returns(T::Array[Elem])}
  def uniq(); end

  # Removes duplicate elements from `self`.
  #
  # If a block is given, it will use the return value of the block for
  # comparison.
  #
  # It compares values using their
  # [`hash`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-hash) and
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-eql-3F)
  # methods for efficiency.
  #
  # `self` is traversed in order, and the first occurrence is kept.
  #
  # Returns `nil` if no changes are made (that is, no duplicates are found).
  #
  # ```ruby
  # a = [ "a", "a", "b", "b", "c" ]
  # a.uniq!   # => ["a", "b", "c"]
  #
  # b = [ "a", "b", "c" ]
  # b.uniq!   # => nil
  #
  # c = [["student","sam"], ["student","george"], ["teacher","matz"]]
  # c.uniq! {|s| s.first}   # => [["student", "sam"], ["teacher", "matz"]]
  # ```
  sig {returns(T::Array[Elem])}
  def uniq!(); end

  # Prepends objects to the front of `self`, moving other elements upwards. See
  # also
  # [`Array#shift`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-shift)
  # for the opposite effect.
  #
  # ```ruby
  # a = [ "b", "c", "d" ]
  # a.unshift("a")   #=> ["a", "b", "c", "d"]
  # a.unshift(1, 2)  #=> [ 1, 2, "a", "b", "c", "d"]
  # ```
  #
  #
  # Also aliased as:
  # [`prepend`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-prepend)
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def unshift(*arg0); end

  # Returns an array containing the elements in `self` corresponding to the
  # given `selector`(s).
  #
  # The selectors may be either integer indices or ranges.
  #
  # See also
  # [`Array#select`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-select).
  #
  # ```ruby
  # a = %w{ a b c d e f }
  # a.values_at(1, 3, 5)          # => ["b", "d", "f"]
  # a.values_at(1, 3, 5, 7)       # => ["b", "d", "f", nil]
  # a.values_at(-1, -2, -2, -7)   # => ["f", "e", "e", nil]
  # a.values_at(4..6, 3...6)      # => ["e", "f", nil, "d", "e", "f"]
  # ```
  sig do
    params(
        arg0: T.any(T::Range[Integer], Integer),
    )
    .returns(T::Array[Elem])
  end
  def values_at(*arg0); end

  # Converts any arguments to arrays, then merges elements of `self` with
  # corresponding elements from each argument.
  #
  # This generates a sequence of `ary.size` *n*-element arrays, where *n* is one
  # more than the count of arguments.
  #
  # If the size of any argument is less than the size of the initial array,
  # `nil` values are supplied.
  #
  # If a block is given, it is invoked for each output `array`, otherwise an
  # array of arrays is returned.
  #
  # ```ruby
  # a = [ 4, 5, 6 ]
  # b = [ 7, 8, 9 ]
  # [1, 2, 3].zip(a, b)   #=> [[1, 4, 7], [2, 5, 8], [3, 6, 9]]
  # [1, 2].zip(a, b)      #=> [[1, 4, 7], [2, 5, 8]]
  # a.zip([1, 2], [8])    #=> [[4, 1, 8], [5, 2, nil], [6, nil, nil]]
  # ```
  sig do
    type_parameters(:U).params(
        arg0: T::Array[T.type_parameter(:U)],
    )
    .returns(T::Array[[Elem, T.nilable(T.type_parameter(:U))]])
  end
  def zip(*arg0); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) Union --- Returns a
  # new array by joining `ary` with `other_ary`, excluding any duplicates and
  # preserving the order from the given arrays.
  #
  # It compares elements using their
  # [`hash`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-hash) and
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-eql-3F)
  # methods for efficiency.
  #
  # ```ruby
  # [ "a", "b", "c" ] | [ "c", "d", "a" ]    #=> [ "a", "b", "c", "d" ]
  # [ "c", "d", "a" ] | [ "a", "b", "c" ]    #=> [ "c", "d", "a", "b" ]
  # ```
  #
  # See also
  # [`Array#union`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-union).
  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def |(arg0); end

  # Alias for:
  # [`length`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-length)
  sig {returns(Integer)}
  def size(); end

  # Element Reference --- Returns the element at `index`, or returns a subarray
  # starting at the `start` index and continuing for `length` elements, or
  # returns a subarray specified by `range` of indices.
  #
  # Negative indices count backward from the end of the array (-1 is the last
  # element). For `start` and `range` cases the starting index is just before an
  # element. Additionally, an empty array is returned when the starting index
  # for an element range is at the end of the array.
  #
  # Returns `nil` if the index (or starting index) are out of range.
  #
  # ```ruby
  # a = [ "a", "b", "c", "d", "e" ]
  # a[2] +  a[0] + a[1]    #=> "cab"
  # a[6]                   #=> nil
  # a[1, 2]                #=> [ "b", "c" ]
  # a[1..3]                #=> [ "b", "c", "d" ]
  # a[4..7]                #=> [ "e" ]
  # a[6..10]               #=> nil
  # a[-3, 3]               #=> [ "c", "d", "e" ]
  # # special cases
  # a[5]                   #=> nil
  # a[6, 1]                #=> nil
  # a[5, 1]                #=> []
  # a[5..10]               #=> []
  # ```
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def slice(arg0, arg1=T.unsafe(nil)); end

  # Returns the sum of elements. For example, [e1, e2, e3].sum returns init + e1
  # + e2 + e3.
  #
  # If a block is given, the block is applied to each element before addition.
  #
  # If *ary* is empty, it returns *init*.
  #
  # ```ruby
  # [].sum                             #=> 0
  # [].sum(0.0)                        #=> 0.0
  # [1, 2, 3].sum                      #=> 6
  # [3, 5.5].sum                       #=> 8.5
  # [2.5, 3.0].sum(0.0) {|e| e * e }   #=> 15.25
  # [Object.new].sum                   #=> TypeError
  # ```
  #
  # The (arithmetic) mean value of an array can be obtained as follows.
  #
  # ```ruby
  # mean = ary.sum(0.0) / ary.length
  # ```
  #
  # This method can be used for non-numeric objects by explicit *init* argument.
  #
  # ```ruby
  # ["a", "b", "c"].sum("")            #=> "abc"
  # [[1], [[2]], [3]].sum([])          #=> [1, [2], 3]
  # ```
  #
  # However,
  # [`Array#join`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-join)
  # and
  # [`Array#flatten`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-flatten)
  # is faster than
  # [`Array#sum`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-sum)
  # for array of strings and array of arrays.
  #
  # ```ruby
  # ["a", "b", "c"].join               #=> "abc"
  # [[1], [[2]], [3]].flatten(1)       #=> [1, [2], 3]
  # ```
  #
  # [`Array#sum`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-sum)
  # method may not respect method redefinition of "+" methods such as
  # [`Integer#+`](https://docs.ruby-lang.org/en/2.6.0/Integer.html#method-i-2B).
  ### {#sum} will combine non-{Numeric} {Elem} types using `:+`, the sigs here
  ### assume any custom implementation of `:+` is sane and returns the same type
  ### as the receiver.
  ###
  ### @note Since `[].sum` is `0`, {Integer} is a potential return type even if it
  ###   is incompatible to sum with the {Elem} type.
  ###
  ### @example returning {Integer}
  ###   T::Array[Float].new.sum #=> 0
  ### @example returning {Elem}
  ###   [1.0].sum #=> 1.0
  sig {returns(T.any(Elem, Integer))}
  ### {#sum} can optionally take a block to perform a function on each element of
  ### the receiver before summation. (It will still return `0` for an empty
  ### array.)
  ###
  ### @example returning {Integer}
  ###   T::Array[Float].new.sum(&:to_f) #=> 0
  ### @example returing generic type
  ###   ['a', 'b'].sum{|t| t.ord.to_f} #=> 195.0
  sig do
    type_parameters(:T).params(
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:T))
    ).returns(T.any(Integer, T.type_parameter(:T)))
  end
  ### The generic is probably overkill here, but `arg0` is returned when the
  ### receiver is empty, even if `arg0` and the {Elem} type cannot be combined
  ### with `:+`.
  ###
  ### @example returning {Elem} type
  ###   [1.0].sum(1) #=> 2.0
  ### @example returning generic type
  ###   T::Array[Float].new.sum(1) #=> 1
  sig do
    type_parameters(:T)
      .params(arg0: T.type_parameter(:T))
      .returns(T.any(Elem, T.type_parameter(:T)))
  end
  ### In the most general case, {#sum} can take both an initial value and a block
  ### to process each element. We require `arg0` and the `blk` return type to
  ### match, but ruby does not require this.
  ###
  ### @example
  ###   [1.0].sum(1) {|t| t.to_i}
  ### @example this is valid ruby via coercion but does not typecheck
  ###   T::Array[Float].new.sum(1) {|t| Complex(t, 1)} #=> 1
  ### @example this is valid ruby via coercion but does not typecheck
  ###   [Complex(1, 2)].sum(1) {|t| 1.0} #=> 2.0
  sig do
    type_parameters(:U).params(
      arg0: T.type_parameter(:U),
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  def sum(arg0=T.unsafe(0), &blk); end

  # Alias for:
  # [`inspect`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-inspect)
  sig {returns(String)}
  def to_s(); end
end
