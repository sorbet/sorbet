# typed: __STDLIB_INTERNAL

# A [`Struct`](https://docs.ruby-lang.org/en/2.7.0/Struct.html) is a convenient
# way to bundle a number of attributes together, using accessor methods, without
# having to write an explicit class.
#
# The [`Struct`](https://docs.ruby-lang.org/en/2.7.0/Struct.html) class
# generates new subclasses that hold a set of members and their values. For each
# member a reader and writer method is created similar to
# [`Module#attr_accessor`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-attr_accessor).
#
# ```ruby
# Customer = Struct.new(:name, :address) do
#   def greeting
#     "Hello #{name}!"
#   end
# end
#
# dave = Customer.new("Dave", "123 Main")
# dave.name     #=> "Dave"
# dave.greeting #=> "Hello Dave!"
# ```
#
# See
# [`Struct::new`](https://docs.ruby-lang.org/en/2.7.0/Struct.html#method-c-new)
# for further examples of creating struct subclasses and instances.
#
# In the method descriptions that follow, a "member" parameter refers to a
# struct member which is either a quoted string (`"name"`) or a
# [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) (`:name`).
class Struct < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Symbol, String),
        keyword_init: T::Boolean,
        blk: T.untyped,
    )
    .void
  end
  def initialize(arg0, *arg1, keyword_init: false, &blk); end

  # Equality---Returns `true` if `other` has the same struct subclass and has
  # equal member values (according to Object#==).
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe   = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joejr = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # jane  = Customer.new("Jane Doe", "456 Elm, Anytown NC", 12345)
  # joe == joejr   #=> true
  # joe == jane    #=> false
  # ```
  def ==(_); end

  # Attribute Reference---Returns the value of the given struct `member` or the
  # member at the given `index`.  Raises
  # [`NameError`](https://docs.ruby-lang.org/en/2.7.0/NameError.html) if the
  # `member` does not exist and
  # [`IndexError`](https://docs.ruby-lang.org/en/2.7.0/IndexError.html) if the
  # `index` is out of range.
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  #
  # joe["name"]   #=> "Joe Smith"
  # joe[:name]    #=> "Joe Smith"
  # joe[0]        #=> "Joe Smith"
  # ```
  def [](_); end

  # Attribute Assignment---Sets the value of the given struct `member` or the
  # member at the given `index`. Raises
  # [`NameError`](https://docs.ruby-lang.org/en/2.7.0/NameError.html) if the
  # `member` does not exist and
  # [`IndexError`](https://docs.ruby-lang.org/en/2.7.0/IndexError.html) if the
  # `index` is out of range.
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  #
  # joe["name"] = "Luke"
  # joe[:zip]   = "90210"
  #
  # joe.name   #=> "Luke"
  # joe.zip    #=> "90210"
  # ```
  def []=(_, _); end

  # Extracts the nested value specified by the sequence of `key` objects by
  # calling `dig` at each step, returning `nil` if any intermediate step is
  # `nil`.
  #
  # ```ruby
  # Foo = Struct.new(:a)
  # f = Foo.new(Foo.new({b: [1, 2, 3]}))
  #
  # f.dig(:a, :a, :b, 0)    # => 1
  # f.dig(:b, 0)            # => nil
  # f.dig(:a, :a, :b, :c)   # TypeError: no implicit conversion of Symbol into Integer
  # ```
  def dig(*_); end

  # Yields the value of each struct member in order. If no block is given an
  # enumerator is returned.
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.each {|x| puts(x) }
  # ```
  #
  # Produces:
  #
  # ```
  # Joe Smith
  # 123 Maple, Anytown NC
  # 12345
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end

  # Yields the name and value of each struct member in order. If no block is
  # given an enumerator is returned.
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.each_pair {|name, value| puts("#{name} => #{value}") }
  # ```
  #
  # Produces:
  #
  # ```
  # name => Joe Smith
  # address => 123 Maple, Anytown NC
  # zip => 12345
  # ```
  def each_pair; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) equality---`other`
  # and `struct` refer to the same hash key if they have the same struct
  # subclass and have equal member values (according to
  # [`Object#eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F)).
  def eql?(_); end

  # Returns a hash value based on this struct's contents.
  #
  # See also
  # [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash).
  def hash; end

  # Returns a description of this struct as a string.
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Struct.html#method-i-to_s)
  def inspect; end

  # Returns the number of struct members.
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.length   #=> 3
  # ```
  def length; end

  # Returns the struct members as an array of symbols:
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.members   #=> [:name, :address, :zip]
  # ```
  def members; end

  sig {returns(T::Array[Symbol])}
  def self.members; end

  # Create a new instance of a struct subclass. The number of value parameters
  # must be less than or equal to the number of attributes defined for the
  # structure. Unset parameters default to nil.  Passing more parameters than
  # number of attributes will raise an ArgumentError.
  #
  # ```
  # Customer = Struct.new(:name, :address)
  # Customer.new("Dave", "123 Main")
  # #=> #<struct Customer name="Dave", address="123 Main">
  # Customer["Dave"]
  # #=> #<struct Customer name="Dave", address=nil>
  # ```
  sig {params(args: T.untyped).returns(Struct)}
  def self.[](*args); end

  sig {params(args: T.untyped).returns(Struct)}
  def new(*args); end

  # Yields each member value from the struct to the block and returns an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) containing the
  # member values from the `struct` for which the given block returns a true
  # value (equivalent to
  # [`Enumerable#select`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-select)).
  #
  # ```ruby
  # Lots = Struct.new(:a, :b, :c, :d, :e, :f)
  # l = Lots.new(11, 22, 33, 44, 55, 66)
  # l.select {|v| v.even? }   #=> [22, 44, 66]
  # ```
  #
  # [`Struct#filter`](https://docs.ruby-lang.org/en/2.7.0/Struct.html#method-i-filter)
  # is an alias for
  # [`Struct#select`](https://docs.ruby-lang.org/en/2.7.0/Struct.html#method-i-select).
  def select(*_); end

  # Returns the number of struct members.
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.length   #=> 3
  # ```
  def size; end

  # Returns the values for this struct as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.to_a[1]   #=> "123 Maple, Anytown NC"
  # ```
  def to_a; end

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) containing
  # the names and values for the struct's members.
  #
  # If a block is given, the results of the block on each pair of the receiver
  # will be used as pairs.
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.to_h[:address]   #=> "123 Maple, Anytown NC"
  # joe.to_h{|name, value| [name.upcase, value.to_s.upcase]}[:ADDRESS]
  #                      #=> "123 MAPLE, ANYTOWN NC"
  # ```
  def to_h; end

  # Alias for:
  # [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Struct.html#method-i-inspect)
  def to_s; end

  # Returns the values for this struct as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.to_a[1]   #=> "123 Maple, Anytown NC"
  # ```
  def values; end

  # Returns the struct member values for each `selector` as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html). A `selector` may
  # be either an [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
  # offset or a [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) of
  # offsets (as in
  # [`Array#values_at`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-values_at)).
  #
  # ```ruby
  # Customer = Struct.new(:name, :address, :zip)
  # joe = Customer.new("Joe Smith", "123 Maple, Anytown NC", 12345)
  # joe.values_at(0, 2)   #=> ["Joe Smith", 12345]
  # ```
  def values_at(*_); end
end
