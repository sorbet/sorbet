# typed: __STDLIB_INTERNAL

# A [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) is a dictionary-like
# collection of unique keys and their values. Also called associative arrays,
# they are similar to Arrays, but where an
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) uses integers as its
# index, a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) allows you to
# use any object type.
#
# Hashes enumerate their values in the order that the corresponding keys were
# inserted.
#
# A [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) can be easily
# created by using its implicit form:
#
# ```ruby
# grades = { "Jane Doe" => 10, "Jim Doe" => 6 }
# ```
#
# Hashes allow an alternate syntax for keys that are symbols. Instead of
#
# ```ruby
# options = { :font_size => 10, :font_family => "Arial" }
# ```
#
# You could write it as:
#
# ```ruby
# options = { font_size: 10, font_family: "Arial" }
# ```
#
# Each named key is a symbol you can access in hash:
#
# ```ruby
# options[:font_size]  # => 10
# ```
#
# A [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) can also be created
# through its
# [`::new`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-c-new) method:
#
# ```ruby
# grades = Hash.new
# grades["Dorothy Doe"] = 9
# ```
#
# Hashes have a *default value* that is returned when accessing keys that do not
# exist in the hash. If no default is set `nil` is used. You can set the default
# value by sending it as an argument to
# [`Hash.new`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-c-new):
#
# ```ruby
# grades = Hash.new(0)
# ```
#
# Or by using the
# [`default=`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-default-3D)
# method:
#
# ```ruby
# grades = {"Timmy Doe" => 8}
# grades.default = 0
# ```
#
# Accessing a value in a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
# requires using its key:
#
# ```ruby
# puts grades["Jane Doe"] # => 0
# ```
#
# ### Common Uses
#
# Hashes are an easy way to represent data structures, such as
#
# ```ruby
# books         = {}
# books[:matz]  = "The Ruby Programming Language"
# books[:black] = "The Well-Grounded Rubyist"
# ```
#
# Hashes are also commonly used as a way to have named parameters in functions.
# Note that no brackets are used below. If a hash is the last argument on a
# method call, no braces are needed, thus creating a really clean interface:
#
# ```ruby
# Person.create(name: "John Doe", age: 27)
#
# def self.create(params)
#   @name = params[:name]
#   @age  = params[:age]
# end
# ```
#
# ### [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) Keys
#
# Two objects refer to the same hash key when their `hash` value is identical
# and the two objects are `eql?` to each other.
#
# A user-defined class may be used as a hash key if the `hash` and `eql?`
# methods are overridden to provide meaningful behavior. By default, separate
# instances refer to separate hash keys.
#
# A typical implementation of `hash` is based on the object's data while `eql?`
# is usually aliased to the overridden `==` method:
#
# ```ruby
# class Book
#   attr_reader :author, :title
#
#   def initialize(author, title)
#     @author = author
#     @title = title
#   end
#
#   def ==(other)
#     self.class === other and
#       other.author == @author and
#       other.title == @title
#   end
#
#   alias eql? ==
#
#   def hash
#     @author.hash ^ @title.hash # XOR
#   end
# end
#
# book1 = Book.new 'matz', 'Ruby in a Nutshell'
# book2 = Book.new 'matz', 'Ruby in a Nutshell'
#
# reviews = {}
#
# reviews[book1] = 'Great reference!'
# reviews[book2] = 'Nice and compact!'
#
# reviews.length #=> 1
# ```
#
# See also
# [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash)
# and
# [`Object#eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F)
class Hash < Object
  include Enumerable

  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)

  # Returns `true` if *hash* is subset of *other*.
  #
  # ```ruby
  # h1 = {a:1, b:2}
  # h2 = {a:1, b:2, c:3}
  # h1 < h2    #=> true
  # h2 < h1    #=> false
  # h1 < h1    #=> false
  # ```
  def <(_); end

  # Returns `true` if *hash* is subset of *other* or equals to *other*.
  #
  # ```ruby
  # h1 = {a:1, b:2}
  # h2 = {a:1, b:2, c:3}
  # h1 <= h2   #=> true
  # h2 <= h1   #=> false
  # h1 <= h1   #=> true
  # ```
  def <=(_); end

  # Equality---Two hashes are equal if they each contain the same number of keys
  # and if each key-value pair is equal to (according to Object#==) the
  # corresponding elements in the other hash.
  #
  # ```ruby
  # h1 = { "a" => 1, "c" => 2 }
  # h2 = { 7 => 35, "c" => 2, "a" => 1 }
  # h3 = { "a" => 1, "c" => 2, 7 => 35 }
  # h4 = { "a" => 1, "d" => 2, "f" => 35 }
  # h1 == h2   #=> false
  # h2 == h3   #=> true
  # h3 == h4   #=> false
  # ```
  #
  # The orders of each hashes are not compared.
  #
  # ```ruby
  # h1 = { "a" => 1, "c" => 2 }
  # h2 = { "c" => 2, "a" => 1 }
  # h1 == h2   #=> true
  # ```
  sig {params(_: T.untyped).returns(T::Boolean)}
  def ==(_); end

  # Returns `true` if *other* is subset of *hash*.
  #
  # ```ruby
  # h1 = {a:1, b:2}
  # h2 = {a:1, b:2, c:3}
  # h1 > h2    #=> false
  # h2 > h1    #=> true
  # h1 > h1    #=> false
  # ```
  def >(_); end

  # Returns `true` if *other* is subset of *hash* or equals to *hash*.
  #
  # ```ruby
  # h1 = {a:1, b:2}
  # h2 = {a:1, b:2, c:3}
  # h1 >= h2   #=> false
  # h2 >= h1   #=> true
  # h1 >= h1   #=> true
  # ```
  def >=(_); end

  # Creates a new hash populated with the given objects.
  #
  # Similar to the literal `{ key => value, ... }`. In the first form, keys and
  # values occur in pairs, so there must be an even number of arguments.
  #
  # The second and third form take a single argument which is either an array of
  # key-value pairs or an object convertible to a hash.
  #
  # ```ruby
  # Hash["a", 100, "b", 200]             #=> {"a"=>100, "b"=>200}
  # Hash[ [ ["a", 100], ["b", 200] ] ]   #=> {"a"=>100, "b"=>200}
  # Hash["a" => 100, "b" => 200]         #=> {"a"=>100, "b"=>200}
  # ```
  sig do
    type_parameters(:U, :V).params(
      arg0: T.any(T::Array[[T.type_parameter(:U), T.type_parameter(:V)]], T::Hash[T.type_parameter(:U), T.type_parameter(:V)]),
    )
    .returns(T::Hash[T.type_parameter(:U), T.type_parameter(:V)])
  end
  def self.[](*arg0); end

  # Element Reference---Retrieves the *value* object corresponding to the *key*
  # object. If not found, returns the default value (see
  # [`Hash::new`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-c-new)
  # for details).
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h["a"]   #=> 100
  # h["c"]   #=> nil
  # ```
  sig do
    params(
        arg0: K,
    )
    .returns(T.nilable(V))
  end
  def [](arg0); end

  # ## Element Assignment
  #
  # Associates the value given by `value` with the key given by `key`.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h["a"] = 9
  # h["c"] = 4
  # h   #=> {"a"=>9, "b"=>200, "c"=>4}
  # h.store("d", 42) #=> 42
  # h   #=> {"a"=>9, "b"=>200, "c"=>4, "d"=>42}
  # ```
  #
  # `key` should not have its value changed while it is in use as a key (an
  # `unfrozen String` passed as a key will be duplicated and frozen).
  #
  # ```ruby
  # a = "a"
  # b = "b".freeze
  # h = { a => 100, b => 200 }
  # h.key(100).equal? a #=> false
  # h.key(200).equal? b #=> true
  # ```
  sig do
    params(
        arg0: K,
        arg1: V,
    )
    .returns(V)
  end
  def []=(arg0, arg1); end

  # Searches through the hash comparing *obj* with the key using `==`. Returns
  # the key-value pair (two elements array) or `nil` if no match is found. See
  # [`Array#assoc`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-assoc).
  #
  # ```ruby
  # h = {"colors"  => ["red", "blue", "green"],
  #      "letters" => ["a", "b", "c" ]}
  # h.assoc("letters")  #=> ["letters", ["a", "b", "c"]]
  # h.assoc("foo")      #=> nil
  # ```
  sig do
    params(
        arg0: K,
    )
    .returns(T.nilable(T::Array[T.any(K, V)]))
  end
  def assoc(arg0); end

  # See also
  # [`Enumerable#any?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-any-3F)
  sig {returns(T::Boolean)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Boolean)
  end
  sig { params(pattern: T.untyped).returns(T::Boolean) }
  def any?(pattern = nil, &blk); end

  # Removes all key-value pairs from *hsh*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }   #=> {"a"=>100, "b"=>200}
  # h.clear                          #=> {}
  # ```
  sig {returns(T::Hash[K, V])}
  def clear(); end

  # Returns a new hash with the nil values/key pairs removed
  #
  # ```ruby
  # h = { a: 1, b: false, c: nil }
  # h.compact     #=> { a: 1, b: false }
  # h             #=> { a: 1, b: false, c: nil }
  # ```
  def compact; end

  # Removes all nil values from the hash. Returns nil if no changes were made,
  # otherwise returns the hash.
  #
  # ```ruby
  # h = { a: 1, b: false, c: nil }
  # h.compact!     #=> { a: 1, b: false }
  # ```
  def compact!; end

  # Makes *hsh* compare its keys by their identity, i.e. it will consider exact
  # same objects as same keys.
  #
  # ```ruby
  # h1 = { "a" => 100, "b" => 200, :c => "c" }
  # h1["a"]        #=> 100
  # h1.compare_by_identity
  # h1.compare_by_identity? #=> true
  # h1["a".dup]    #=> nil  # different objects.
  # h1[:c]         #=> "c"  # same symbols are all same.
  # ```
  sig {returns(T::Hash[K, V])}
  def compare_by_identity(); end

  # Returns `true` if *hsh* will compare its keys by their identity. Also see
  # [`Hash#compare_by_identity`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-compare_by_identity).
  sig {returns(T::Boolean)}
  def compare_by_identity?(); end

  # Returns the default value, the value that would be returned by *[hsh](key)*
  # if *key* did not exist in *hsh*. See also
  # [`Hash::new`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-c-new)
  # and
  # [`Hash#default=`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-default-3D).
  #
  # ```ruby
  # h = Hash.new                            #=> {}
  # h.default                               #=> nil
  # h.default(2)                            #=> nil
  #
  # h = Hash.new("cat")                     #=> {}
  # h.default                               #=> "cat"
  # h.default(2)                            #=> "cat"
  #
  # h = Hash.new {|h,k| h[k] = k.to_i*10}   #=> {}
  # h.default                               #=> nil
  # h.default(2)                            #=> 20
  # ```
  sig do
    params(
        arg0: K,
    )
    .returns(T.nilable(V))
  end
  sig do
    params(
        arg0: K,
        blk: T.proc.params(arg0: K).returns(V),
    )
    .returns(T.nilable(V))
  end
  def default(arg0=T.unsafe(nil), &blk); end

  # Sets the default value, the value returned for a key that does not exist in
  # the hash. It is not possible to set the default to a
  # [`Proc`](https://docs.ruby-lang.org/en/2.7.0/Proc.html) that will be
  # executed on each key lookup.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.default = "Go fish"
  # h["a"]     #=> 100
  # h["z"]     #=> "Go fish"
  # # This doesn't do what you might hope...
  # h.default = proc do |hash, key|
  #   hash[key] = key + key
  # end
  # h[2]       #=> #<Proc:0x401b3948@-:6>
  # h["cat"]   #=> #<Proc:0x401b3948@-:6>
  # ```
  sig do
    params(
        arg0: V,
    )
    .returns(V)
  end
  def default=(arg0); end

  # If [`Hash::new`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-c-new)
  # was invoked with a block, return that block, otherwise return `nil`.
  #
  # ```ruby
  # h = Hash.new {|h,k| h[k] = k*k }   #=> {}
  # p = h.default_proc                 #=> #<Proc:0x401b3d08@-:1>
  # a = []                             #=> []
  # p.call(a, 2)
  # a                                  #=> [nil, nil, 4]
  # ```
  def default_proc; end

  # Sets the default proc to be executed on each failed key lookup.
  #
  # ```ruby
  # h.default_proc = proc do |hash, key|
  #   hash[key] = key + key
  # end
  # h[2]       #=> 4
  # h["cat"]   #=> "catcat"
  # ```
  def default_proc=(_); end

  # Deletes the key-value pair and returns the value from *hsh* whose key is
  # equal to *key*. If the key is not found, it returns *nil*. If the optional
  # code block is given and the key is not found, pass in the key and return the
  # result of *block*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.delete("a")                              #=> 100
  # h.delete("z")                              #=> nil
  # h.delete("z") { |el| "#{el} not found" }   #=> "z not found"
  # ```
  sig do
    params(
        arg0: K,
    )
    .returns(T.nilable(V))
  end
  sig do
    type_parameters(:U).params(
        arg0: K,
        blk: T.proc.params(arg0: K).returns(T.type_parameter(:U)),
    )
    .returns(T.any(T.type_parameter(:U), V))
  end
  def delete(arg0, &blk); end

  # Deletes every key-value pair from *hsh* for which *block* evaluates to
  # `true`.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200, "c" => 300 }
  # h.delete_if {|key, value| key >= "b" }   #=> {"a"=>100}
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(T::Enumerator[[K, V]])}
  def delete_if(&blk); end

  # Extracts the nested value specified by the sequence of *key* objects by
  # calling `dig` at each step, returning `nil` if any intermediate step is
  # `nil`.
  #
  # ```ruby
  # h = { foo: {bar: {baz: 1}}}
  #
  # h.dig(:foo, :bar, :baz)     #=> 1
  # h.dig(:foo, :zot, :xyz)     #=> nil
  #
  # g = { foo: [10, 11, 12] }
  # g.dig(:foo, 1)              #=> 11
  # g.dig(:foo, 1, 0)           #=> TypeError: Integer does not have #dig method
  # g.dig(:foo, :bar)           #=> TypeError: no implicit conversion of Symbol into Integer
  # ```
  sig do
    params(
      key: K,
      rest: T.untyped
    )
    .returns(T.untyped)
  end
  def dig(key, *rest); end

  # Calls *block* once for each key in *hsh*, passing the key-value pair as
  # parameters.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.each {|key, value| puts "#{key} is #{value}" }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # a is 100
  # b is 200
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: [K, V]).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(T::Enumerator[[K, V]])}
  def each(&blk); end

  # Calls *block* once for each key in *hsh*, passing the key as a parameter.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.each_key {|key| puts key }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # a
  # b
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: K).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(T::Enumerator[K])}
  def each_key(&blk); end

  # Calls *block* once for each key in *hsh*, passing the key-value pair as
  # parameters.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.each {|key, value| puts "#{key} is #{value}" }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # a is 100
  # b is 200
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(T::Enumerator[[K, V]])}
  def each_pair(&blk); end

  # Calls *block* once for each key in *hsh*, passing the value as a parameter.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.each_value {|value| puts value }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # 100
  # 200
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(T::Enumerator[V])}
  def each_value(&blk); end

  # Returns `true` if *hsh* contains no key-value pairs.
  #
  # ```ruby
  # {}.empty?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def empty?(); end

  # Returns `true` if *hash* and *other* are both hashes with the same content.
  # The orders of each hashes are not compared.
  def eql?(_); end

  # Returns a value from the hash for the given key. If the key can't be found,
  # there are several options: With no other arguments, it will raise a
  # [`KeyError`](https://docs.ruby-lang.org/en/2.7.0/KeyError.html) exception;
  # if *default* is given, then that will be returned; if the optional code
  # block is specified, then that will be run and its result returned.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.fetch("a")                            #=> 100
  # h.fetch("z", "go fish")                 #=> "go fish"
  # h.fetch("z") { |el| "go fish, #{el}"}   #=> "go fish, z"
  # ```
  #
  # The following example shows that an exception is raised if the key is not
  # found and a default value is not supplied.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.fetch("z")
  # ```
  #
  # *produces:*
  #
  # ```
  # prog.rb:2:in `fetch': key not found (KeyError)
  #  from prog.rb:2
  # ```
  sig do
    params(
        arg0: K,
    )
    .returns(V)
  end
  sig do
   type_parameters(:X).params(
      arg0: K,
      arg1: T.type_parameter(:X),
    )
    .returns(T.any(V, T.type_parameter(:X)))
  end
  sig do
   type_parameters(:X).params(
        arg0: K,
        blk: T.proc.params(arg0: K).returns(T.type_parameter(:X)),
    )
    .returns(T.any(V, T.type_parameter(:X)))
  end
  def fetch(arg0, arg1=T.unsafe(nil), &blk); end

  # Returns an array containing the values associated with the given keys but
  # also raises [`KeyError`](https://docs.ruby-lang.org/en/2.7.0/KeyError.html)
  # when one of keys can't be found. Also see
  # [`Hash#values_at`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-values_at)
  # and
  # [`Hash#fetch`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-fetch).
  #
  # ```ruby
  # h = { "cat" => "feline", "dog" => "canine", "cow" => "bovine" }
  #
  # h.fetch_values("cow", "cat")                   #=> ["bovine", "feline"]
  # h.fetch_values("cow", "bird")                  # raises KeyError
  # h.fetch_values("cow", "bird") { |k| k.upcase } #=> ["bovine", "BIRD"]
  # ```
  def fetch_values(*_); end

  # Returns a new Hash excluding entries for the given keys.
  # [`Hash#except`](https://ruby-doc.org/core-3.0.0/Hash.html#method-i-except)
  #
  # ```ruby
  # h = { a: 100, b: 200, c: 300 }
  # h.except(:a)          #=> {:b=>200, :c=>300}
  # ```
  sig do
    params(
        args: K,
    )
    .returns(T::Hash[K, V])
  end
  def except(*args); end

  # Returns a new array that is a one-dimensional flattening of this hash. That
  # is, for every key or value that is an array, extract its elements into the
  # new array. Unlike
  # [`Array#flatten`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-flatten),
  # this method does not flatten recursively by default. The optional *level*
  # argument determines the level of recursion to flatten.
  #
  # ```ruby
  # a =  {1=> "one", 2 => [2,"two"], 3 => "three"}
  # a.flatten    # => [1, "one", 2, [2, "two"], 3, "three"]
  # a.flatten(2) # => [1, "one", 2, 2, "two", 3, "three"]
  # ```
  def flatten(*_); end

  # Returns `true` if the given key is present in *hsh*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.has_key?("a")   #=> true
  # h.has_key?("z")   #=> false
  # ```
  #
  # Note that
  # [`include?`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-include-3F)
  # and
  # [`member?`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-member-3F)
  # do not test member equality using `==` as do other Enumerables.
  #
  # See also
  # [`Enumerable#include?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-include-3F)
  sig do
    params(
        arg0: K,
    )
    .returns(T::Boolean)
  end
  def has_key?(arg0); end

  # Returns `true` if the given value is present for some key in *hsh*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.value?(100)   #=> true
  # h.value?(999)   #=> false
  # ```
  sig do
    params(
        arg0: V,
    )
    .returns(T::Boolean)
  end
  def has_value?(arg0); end

  # Returns `true` if the given key is present in *hsh*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.has_key?("a")   #=> true
  # h.has_key?("z")   #=> false
  # ```
  #
  # Note that
  # [`include?`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-include-3F)
  # and
  # [`member?`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-member-3F)
  # do not test member equality using `==` as do other Enumerables.
  #
  # See also
  # [`Enumerable#include?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-include-3F)
  sig do
    params(
        arg0: K,
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end

  # Compute a hash-code for this hash. Two hashes with the same content will
  # have the same hash code (and will compare using `eql?`).
  #
  # See also
  # [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash).
  def hash; end

  sig {void}
  sig {params(default: V).void}
  sig do
    params(
        blk: T.proc.params(hash: T::Hash[K, V], key: K).void
    )
    .void
  end
  def initialize(default=T.unsafe(nil), &blk); end

  # Return the contents of this hash as a string.
  #
  # ```ruby
  # h = { "c" => 300, "a" => 100, "d" => 400, "c" => 300  }
  # h.to_s   #=> "{\"c\"=>300, \"a\"=>100, \"d\"=>400}"
  # ```
  #
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-to_s)
  sig {returns(String)}
  def inspect(); end

  # Returns a new hash created by using *hsh*'s values as keys, and the keys as
  # values. If a key with the same value already exists in the *hsh*, then the
  # last one defined will be used, the earlier value(s) will be discarded.
  #
  # ```ruby
  # h = { "n" => 100, "m" => 100, "y" => 300, "d" => 200, "a" => 0 }
  # h.invert   #=> {0=>"a", 100=>"m", 200=>"d", 300=>"y"}
  # ```
  #
  # If there is no key with the same value,
  # [`Hash#invert`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-invert)
  # is involutive.
  #
  # ```ruby
  # h = { a: 1, b: 3, c: 4 }
  # h.invert.invert == h #=> true
  # ```
  #
  # The condition, no key with the same value, can be tested by comparing the
  # size of inverted hash.
  #
  # ```ruby
  # # no key with the same value
  # h = { a: 1, b: 3, c: 4 }
  # h.size == h.invert.size #=> true
  #
  # # two (or more) keys has the same value
  # h = { a: 1, b: 3, c: 1 }
  # h.size == h.invert.size #=> false
  # ```
  sig {returns(T::Hash[V, K])}
  def invert(); end

  # Deletes every key-value pair from *hsh* for which *block* evaluates to
  # `false`.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # See also
  # [`Hash#select!`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-select-21).
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(T::Enumerator[[K, V]])}
  def keep_if(&blk); end

  # Returns the key of an occurrence of a given value. If the value is not
  # found, returns `nil`.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200, "c" => 300, "d" => 300 }
  # h.key(200)   #=> "b"
  # h.key(300)   #=> "c"
  # h.key(999)   #=> nil
  # ```
  sig do
    params(
        arg0: V,
    )
    .returns(T.nilable(K))
  end
  def key(arg0); end

  # Returns `true` if the given key is present in *hsh*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.has_key?("a")   #=> true
  # h.has_key?("z")   #=> false
  # ```
  #
  # Note that
  # [`include?`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-include-3F)
  # and
  # [`member?`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-member-3F)
  # do not test member equality using `==` as do other Enumerables.
  #
  # See also
  # [`Enumerable#include?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-include-3F)
  sig do
    params(
        arg0: K,
    )
    .returns(T::Boolean)
  end
  def key?(arg0); end

  # Returns a new array populated with the keys from this hash. See also
  # [`Hash#values`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-values).
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200, "c" => 300, "d" => 400 }
  # h.keys   #=> ["a", "b", "c", "d"]
  # ```
  sig {returns(T::Array[K])}
  def keys(); end

  # Returns the number of key-value pairs in the hash.
  #
  # ```ruby
  # h = { "d" => 100, "a" => 200, "v" => 300, "e" => 400 }
  # h.size          #=> 4
  # h.delete("a")   #=> 200
  # h.size          #=> 3
  # h.length        #=> 3
  # ```
  #
  # [`Hash#length`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-length)
  # is an alias for
  # [`Hash#size`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-size).
  sig {returns(Integer)}
  def length(); end

  # Returns `true` if the given key is present in *hsh*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.has_key?("a")   #=> true
  # h.has_key?("z")   #=> false
  # ```
  #
  # Note that
  # [`include?`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-include-3F)
  # and
  # [`member?`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-member-3F)
  # do not test member equality using `==` as do other Enumerables.
  #
  # See also
  # [`Enumerable#include?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-include-3F)
  sig do
    params(
        arg0: K,
    )
    .returns(T::Boolean)
  end
  def member?(arg0); end

  # Returns a new hash that combines the contents of the receiver and the
  # contents of the given hashes.
  #
  # If no block is given, entries with duplicate keys are overwritten with the
  # values from each `other_hash` successively, otherwise the value for each
  # duplicate key is determined by calling the block with the key, its value in
  # the receiver and its value in each `other_hash`.
  #
  # When called without any argument, returns a copy of the receiver.
  #
  # ```ruby
  # h1 = { "a" => 100, "b" => 200 }
  # h2 = { "b" => 246, "c" => 300 }
  # h3 = { "b" => 357, "d" => 400 }
  # h1.merge          #=> {"a"=>100, "b"=>200}
  # h1.merge(h2)      #=> {"a"=>100, "b"=>246, "c"=>300}
  # h1.merge(h2, h3)  #=> {"a"=>100, "b"=>357, "c"=>300, "d"=>400}
  # h1.merge(h2) {|key, oldval, newval| newval - oldval}
  #                   #=> {"a"=>100, "b"=>46,  "c"=>300}
  # h1.merge(h2, h3) {|key, oldval, newval| newval - oldval}
  #                   #=> {"a"=>100, "b"=>311, "c"=>300, "d"=>400}
  # h1                #=> {"a"=>100, "b"=>200}
  # ```
  sig do
    type_parameters(:A ,:B).params(
        arg0: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
    )
    .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  end
  sig do
    type_parameters(:A ,:B).params(
        arg0: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
        blk: T.proc.params(arg0: K, arg1: V, arg2: T.type_parameter(:B)).returns(T.any(V, T.type_parameter(:B))),
    )
    .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  end
  def merge(*arg0, &blk); end

  # Adds the contents of the given hashes to the receiver.
  #
  # If no block is given, entries with duplicate keys are overwritten with the
  # values from each `other_hash` successively, otherwise the value for each
  # duplicate key is determined by calling the block with the key, its value in
  # the receiver and its value in each `other_hash`.
  #
  # ```ruby
  # h1 = { "a" => 100, "b" => 200 }
  # h1.merge!          #=> {"a"=>100, "b"=>200}
  # h1                 #=> {"a"=>100, "b"=>200}
  #
  # h1 = { "a" => 100, "b" => 200 }
  # h2 = { "b" => 246, "c" => 300 }
  # h1.merge!(h2)      #=> {"a"=>100, "b"=>246, "c"=>300}
  # h1                 #=> {"a"=>100, "b"=>246, "c"=>300}
  #
  # h1 = { "a" => 100, "b" => 200 }
  # h2 = { "b" => 246, "c" => 300 }
  # h3 = { "b" => 357, "d" => 400 }
  # h1.merge!(h2, h3)
  #                    #=> {"a"=>100, "b"=>357, "c"=>300, "d"=>400}
  # h1                 #=> {"a"=>100, "b"=>357, "c"=>300, "d"=>400}
  #
  # h1 = { "a" => 100, "b" => 200 }
  # h2 = { "b" => 246, "c" => 300 }
  # h3 = { "b" => 357, "d" => 400 }
  # h1.merge!(h2, h3) {|key, v1, v2| v1 }
  #                    #=> {"a"=>100, "b"=>200, "c"=>300, "d"=>400}
  # h1                 #=> {"a"=>100, "b"=>200, "c"=>300, "d"=>400}
  # ```
  #
  # [`Hash#update`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-update)
  # is an alias for
  # [`Hash#merge!`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-merge-21).
  sig do
    type_parameters(:A ,:B).params(
        other_hash: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
    )
    .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  end
  sig do
    type_parameters(:A ,:B).params(
        other_hash: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
        blk: T.proc.params(key: K, oldval: V, newval: T.type_parameter(:B)).returns(T.any(V, T.type_parameter(:B))),
    )
    .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  end
  def merge!(*other_hash, &blk); end

  # Searches through the hash comparing *obj* with the value using `==`. Returns
  # the first key-value pair (two-element array) that matches. See also
  # [`Array#rassoc`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-rassoc).
  #
  # ```ruby
  # a = {1=> "one", 2 => "two", 3 => "three", "ii" => "two"}
  # a.rassoc("two")    #=> [2, "two"]
  # a.rassoc("four")   #=> nil
  # ```
  sig do
    params(
        arg0: V,
    )
    .returns(T.nilable([K, V]))
  end
  def rassoc(arg0); end

  # Rebuilds the hash based on the current hash values for each key. If values
  # of key objects have changed since they were inserted, this method will
  # reindex *hsh*. If
  # [`Hash#rehash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-rehash)
  # is called while an iterator is traversing the hash, a
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html) will
  # be raised in the iterator.
  #
  # ```ruby
  # a = [ "a", "b" ]
  # c = [ "c", "d" ]
  # h = { a => 100, c => 300 }
  # h[a]       #=> 100
  # a[0] = "z"
  # h[a]       #=> nil
  # h.rehash   #=> {["z", "b"]=>100, ["c", "d"]=>300}
  # h[a]       #=> 100
  # ```
  sig {returns(T::Hash[K, V])}
  def rehash(); end

  # Returns a new hash consisting of entries for which the block returns false.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200, "c" => 300 }
  # h.reject {|k,v| k < "b"}  #=> {"b" => 200, "c" => 300}
  # h.reject {|k,v| v > 100}  #=> {"a" => 100}
  # ```
  sig {returns(T::Enumerator[[K, V]])}
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def reject(&blk); end

  # Equivalent to
  # [`Hash#delete_if`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-delete_if),
  # but returns `nil` if no changes were made.
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def reject!(&blk); end

  # Replaces the contents of *hsh* with the contents of *other\_hash*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.replace({ "c" => 300, "d" => 400 })   #=> {"c"=>300, "d"=>400}
  # ```
  def replace(_); end

  # Returns a new hash consisting of entries for which the block returns true.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200, "c" => 300 }
  # h.select {|k,v| k > "a"}  #=> {"b" => 200, "c" => 300}
  # h.select {|k,v| v < 200}  #=> {"a" => 100}
  # ```
  #
  # [`Hash#filter`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-filter)
  # is an alias for
  # [`Hash#select`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-select).
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(T::Enumerator[[K, V]])}
  def select(&blk); end

  # Returns a hash containing only the given keys and their values.
  #
  # ```ruby
  # h = { a: 100, b: 200, c: 300 }
  # h.slice(:a)           #=> {:a=>100}
  # h.slice(:b, :c, :d)   #=> {:b=>200, :c=>300}
  # ```
  def slice(*_); end

  # Returns a new hash consisting of entries for which the block returns true.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200, "c" => 300 }
  # h.select {|k,v| k > "a"}  #=> {"b" => 200, "c" => 300}
  # h.select {|k,v| v < 200}  #=> {"a" => 100}
  # ```
  #
  # [`Hash#filter`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-filter)
  # is an alias for
  # [`Hash#select`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-select).
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(T::Enumerator[[K, V]])}
  def filter(&blk); end

  # Equivalent to
  # [`Hash#keep_if`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-keep_if),
  # but returns `nil` if no changes were made.
  #
  # [`Hash#filter!`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-filter-21)
  # is an alias for
  # [`Hash#select!`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-select-21).
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def select!(&blk); end

  # Equivalent to
  # [`Hash#keep_if`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-keep_if),
  # but returns `nil` if no changes were made.
  #
  # [`Hash#filter!`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-filter-21)
  # is an alias for
  # [`Hash#select!`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-select-21).
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def filter!(&blk); end

  # Removes a key-value pair from *hsh* and returns it as the two-item array `[`
  # *key, value* `]`, or nil if the hash is empty.
  #
  # ```ruby
  # h = { 1 => "a", 2 => "b", 3 => "c" }
  # h.shift   #=> [1, "a"]
  # h         #=> {2=>"b", 3=>"c"}
  # ```
  sig {returns(T.nilable(T::Array[T.any(K, V)]))}
  def shift(); end

  # Returns the number of key-value pairs in the hash.
  #
  # ```ruby
  # h = { "d" => 100, "a" => 200, "v" => 300, "e" => 400 }
  # h.size          #=> 4
  # h.delete("a")   #=> 200
  # h.size          #=> 3
  # h.length        #=> 3
  # ```
  #
  # [`Hash#length`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-length)
  # is an alias for
  # [`Hash#size`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-size).
  sig {returns(Integer)}
  def size(); end

  # ## Element Assignment
  #
  # Associates the value given by `value` with the key given by `key`.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h["a"] = 9
  # h["c"] = 4
  # h   #=> {"a"=>9, "b"=>200, "c"=>4}
  # h.store("d", 42) #=> 42
  # h   #=> {"a"=>9, "b"=>200, "c"=>4, "d"=>42}
  # ```
  #
  # `key` should not have its value changed while it is in use as a key (an
  # `unfrozen String` passed as a key will be duplicated and frozen).
  #
  # ```ruby
  # a = "a"
  # b = "b".freeze
  # h = { a => 100, b => 200 }
  # h.key(100).equal? a #=> false
  # h.key(200).equal? b #=> true
  # ```
  sig do
    params(
        arg0: K,
        arg1: V,
    )
    .returns(V)
  end
  def store(arg0, arg1); end

  # Converts *hsh* to a nested array of `[` *key, value* `]` arrays.
  #
  # ```ruby
  # h = { "c" => 300, "a" => 100, "d" => 400, "c" => 300  }
  # h.to_a   #=> [["c", 300], ["a", 100], ["d", 400]]
  # ```
  sig {returns(T::Array[[K, V]])}
  def to_a(); end

  # Returns `self`. If called on a subclass of
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html), converts the
  # receiver to a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  # object.
  #
  # If a block is given, the results of the block on each pair of the receiver
  # will be used as pairs.
  sig do
    type_parameters(:A, :B)
      .params(
        blk: T.proc.params(arg0: K, arg1: V)
              .returns([T.type_parameter(:A), T.type_parameter(:B)])
      )
      .returns(T::Hash[T.type_parameter(:A), T.type_parameter(:B)])
  end
  sig {returns(T::Hash[K, V])}
  def to_h(&blk); end

  # Returns `self`.
  sig {returns(T::Hash[K, V])}
  def to_hash(); end

  # Returns a [`Proc`](https://docs.ruby-lang.org/en/2.7.0/Proc.html) which maps
  # keys to values.
  #
  # ```ruby
  # h = {a:1, b:2}
  # hp = h.to_proc
  # hp.call(:a)          #=> 1
  # hp.call(:b)          #=> 2
  # hp.call(:c)          #=> nil
  # [:a, :b, :c].map(&h) #=> [1, 2, nil]
  # ```
  def to_proc; end

  # Alias for:
  # [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-inspect)
  sig {returns(String)}
  def to_s(); end

  # Returns a new hash with the results of running the block once for every key.
  # This method does not change the values.
  #
  # ```ruby
  # h = { a: 1, b: 2, c: 3 }
  # h.transform_keys {|k| k.to_s }  #=> { "a" => 1, "b" => 2, "c" => 3 }
  # h.transform_keys(&:to_s)        #=> { "a" => 1, "b" => 2, "c" => 3 }
  # h.transform_keys.with_index {|k, i| "#{k}.#{i}" }
  #                                 #=> { "a.0" => 1, "b.1" => 2, "c.2" => 3 }
  # ```
  #
  # If no block is given, an enumerator is returned instead.
  sig do
    type_parameters(:A).params(
      blk: T.proc.params(arg0: K).returns(T.type_parameter(:A))
    )
                       .returns(T::Hash[T.type_parameter(:A), V])
  end
  sig do
    returns(T::Enumerator[K])
  end
  def transform_keys(&blk); end

  # Invokes the given block once for each key in *hsh*, replacing it with the
  # new key returned by the block, and then returns *hsh*. This method does not
  # change the values.
  #
  # ```ruby
  # h = { a: 1, b: 2, c: 3 }
  # h.transform_keys! {|k| k.to_s }  #=> { "a" => 1, "b" => 2, "c" => 3 }
  # h.transform_keys!(&:to_sym)      #=> { a: 1, b: 2, c: 3 }
  # h.transform_keys!.with_index {|k, i| "#{k}.#{i}" }
  #                                  #=> { "a.0" => 1, "b.1" => 2, "c.2" => 3 }
  # ```
  #
  # If no block is given, an enumerator is returned instead.
  sig do
    type_parameters(:A).params(
      blk: T.proc.params(arg0: K).returns(T.type_parameter(:A))
    )
                       .returns(T::Hash[T.type_parameter(:A), V])
  end
  sig do
    returns(T::Enumerator[K])
  end
  def transform_keys!(&blk); end

  # Returns a new hash with the results of running the block once for every
  # value. This method does not change the keys.
  #
  # ```ruby
  # h = { a: 1, b: 2, c: 3 }
  # h.transform_values {|v| v * v + 1 }  #=> { a: 2, b: 5, c: 10 }
  # h.transform_values(&:to_s)           #=> { a: "1", b: "2", c: "3" }
  # h.transform_values.with_index {|v, i| "#{v}.#{i}" }
  #                                      #=> { a: "1.0", b: "2.1", c: "3.2" }
  # ```
  #
  # If no block is given, an enumerator is returned instead.
  sig do
    type_parameters(:A).params(
      blk: T.proc.params(arg0: V).returns(T.type_parameter(:A))
    )
                       .returns(T::Hash[K, T.type_parameter(:A)])
  end
  sig do
    returns(T::Enumerator[V])
  end
  def transform_values(&blk); end

  # Invokes the given block once for each value in *hsh*, replacing it with the
  # new value returned by the block, and then returns *hsh*. This method does
  # not change the keys.
  #
  # ```ruby
  # h = { a: 1, b: 2, c: 3 }
  # h.transform_values! {|v| v * v + 1 }  #=> { a: 2, b: 5, c: 10 }
  # h.transform_values!(&:to_s)           #=> { a: "2", b: "5", c: "10" }
  # h.transform_values!.with_index {|v, i| "#{v}.#{i}" }
  #                                       #=> { a: "2.0", b: "5.1", c: "10.2" }
  # ```
  #
  # If no block is given, an enumerator is returned instead.
  sig do
    type_parameters(:A).params(
      blk: T.proc.params(arg0: V).returns(T.type_parameter(:A))
    )
                       .returns(T::Hash[K, T.type_parameter(:A)])
  end
  sig do
    returns(T::Enumerator[V])
  end
  def transform_values!(&blk); end

  # Attempts to convert a given object to a `Hash``. If *obj* is a `Hash``,
  # *obj* is returned. If *obj* reponds to `to_hash`, the return of
  # *obj.to_hash* is returned. Otherwise, `nil` is returned.
  #
  # ```ruby
  # Hash.try_convert({ a: :b }) #=> { a: :b }
  # Hash.try_convert(Object.new) #=> nil
  # ```
  #
  # An exception is raised when `obj.to_hash` doesn't return a Hash.
  sig do
    params(obj: T.untyped).returns(T.nilable(T::Hash[T.untyped, T.untyped]))
  end
  def self.try_convert(obj); end

  # Adds the contents of the given hashes to the receiver.
  #
  # If no block is given, entries with duplicate keys are overwritten with the
  # values from each `other_hash` successively, otherwise the value for each
  # duplicate key is determined by calling the block with the key, its value in
  # the receiver and its value in each `other_hash`.
  #
  # ```ruby
  # h1 = { "a" => 100, "b" => 200 }
  # h1.merge!          #=> {"a"=>100, "b"=>200}
  # h1                 #=> {"a"=>100, "b"=>200}
  #
  # h1 = { "a" => 100, "b" => 200 }
  # h2 = { "b" => 246, "c" => 300 }
  # h1.merge!(h2)      #=> {"a"=>100, "b"=>246, "c"=>300}
  # h1                 #=> {"a"=>100, "b"=>246, "c"=>300}
  #
  # h1 = { "a" => 100, "b" => 200 }
  # h2 = { "b" => 246, "c" => 300 }
  # h3 = { "b" => 357, "d" => 400 }
  # h1.merge!(h2, h3)
  #                    #=> {"a"=>100, "b"=>357, "c"=>300, "d"=>400}
  # h1                 #=> {"a"=>100, "b"=>357, "c"=>300, "d"=>400}
  #
  # h1 = { "a" => 100, "b" => 200 }
  # h2 = { "b" => 246, "c" => 300 }
  # h3 = { "b" => 357, "d" => 400 }
  # h1.merge!(h2, h3) {|key, v1, v2| v1 }
  #                    #=> {"a"=>100, "b"=>200, "c"=>300, "d"=>400}
  # h1                 #=> {"a"=>100, "b"=>200, "c"=>300, "d"=>400}
  # ```
  #
  # [`Hash#update`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-update)
  # is an alias for
  # [`Hash#merge!`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-merge-21).
  sig do
    type_parameters(:A ,:B).params(
        other_hash: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
    )
    .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  end
  sig do
    type_parameters(:A ,:B).params(
        other_hash: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
        blk: T.proc.params(key: K, oldval: V, newval: T.type_parameter(:B)).returns(T.any(V, T.type_parameter(:B))),
    )
    .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  end
  def update(*other_hash, &blk); end

  # Returns `true` if the given value is present for some key in *hsh*.
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200 }
  # h.value?(100)   #=> true
  # h.value?(999)   #=> false
  # ```
  sig do
    params(
        arg0: V,
    )
    .returns(T::Boolean)
  end
  def value?(arg0); end

  # Returns a new array populated with the values from *hsh*. See also
  # [`Hash#keys`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-keys).
  #
  # ```ruby
  # h = { "a" => 100, "b" => 200, "c" => 300 }
  # h.values   #=> [100, 200, 300]
  # ```
  sig {returns(T::Array[V])}
  def values(); end

  # Return an array containing the values associated with the given keys. Also
  # see
  # [`Hash.select`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-select).
  #
  # ```ruby
  # h = { "cat" => "feline", "dog" => "canine", "cow" => "bovine" }
  # h.values_at("cow", "cat")  #=> ["bovine", "feline"]
  # ```
  sig do
    params(
        arg0: K,
    )
    .returns(T::Array[V])
  end
  def values_at(*arg0); end
end
