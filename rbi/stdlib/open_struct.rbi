# typed: __STDLIB_INTERNAL

# An [`OpenStruct`](https://docs.ruby-lang.org/en/2.6.0/OpenStruct.html) is a
# data structure, similar to a
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html), that allows the
# definition of arbitrary attributes with their accompanying values. This is
# accomplished by using Ruby's metaprogramming to define methods on the class
# itself.
#
# ## Examples
#
# ```ruby
# require "ostruct"
#
# person = OpenStruct.new
# person.name = "John Smith"
# person.age  = 70
#
# person.name      # => "John Smith"
# person.age       # => 70
# person.address   # => nil
# ```
#
# An [`OpenStruct`](https://docs.ruby-lang.org/en/2.6.0/OpenStruct.html) employs
# a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) internally to store
# the attributes and values and can even be initialized with one:
#
# ```ruby
# australia = OpenStruct.new(:country => "Australia", :capital => "Canberra")
#   # => #<OpenStruct country="Australia", capital="Canberra">
# ```
#
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) keys with spaces or
# characters that could normally not be used for method calls (e.g. `()[]*`)
# will not be immediately available on the
# [`OpenStruct`](https://docs.ruby-lang.org/en/2.6.0/OpenStruct.html) object as
# a method for retrieval or assignment, but can still be reached through the
# [`Object#send`](https://docs.ruby-lang.org/en/2.6.0/Object.html#method-i-send)
# method.
#
# ```ruby
# measurements = OpenStruct.new("length (in inches)" => 24)
# measurements.send("length (in inches)")   # => 24
#
# message = OpenStruct.new(:queued? => true)
# message.queued?                           # => true
# message.send("queued?=", false)
# message.queued?                           # => false
# ```
#
# Removing the presence of an attribute requires the execution of the
# [`delete_field`](https://docs.ruby-lang.org/en/2.6.0/OpenStruct.html#method-i-delete_field)
# method as setting the property value to `nil` will not remove the attribute.
#
# ```ruby
# first_pet  = OpenStruct.new(:name => "Rowdy", :owner => "John Smith")
# second_pet = OpenStruct.new(:name => "Rowdy")
#
# first_pet.owner = nil
# first_pet                 # => #<OpenStruct name="Rowdy", owner=nil>
# first_pet == second_pet   # => false
#
# first_pet.delete_field(:owner)
# first_pet                 # => #<OpenStruct name="Rowdy">
# first_pet == second_pet   # => true
# ```
#
# ## Implementation
#
# An [`OpenStruct`](https://docs.ruby-lang.org/en/2.6.0/OpenStruct.html)
# utilizes Ruby's method lookup structure to find and define the necessary
# methods for properties. This is accomplished through the methods
# method\_missing and define\_singleton\_method.
#
# This should be a consideration if there is a concern about the performance of
# the objects that are created, as there is much more overhead in the setting of
# these properties compared to using a
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) or a
# [`Struct`](https://docs.ruby-lang.org/en/2.6.0/Struct.html).
class OpenStruct
  InspectKey = ::T.let(nil, ::T.untyped)

  # Compares this object and `other` for equality. An
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html) is equal
  # to `other` when `other` is an
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html) and the
  # two objects' [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) tables
  # are equal.
  #
  # ```ruby
  # require "ostruct"
  # first_pet  = OpenStruct.new("name" => "Rowdy")
  # second_pet = OpenStruct.new(:name  => "Rowdy")
  # third_pet  = OpenStruct.new("name" => "Rowdy", :age => nil)
  #
  # first_pet == second_pet   # => true
  # first_pet == third_pet    # => false
  # ```
  sig do
    params(
      other: ::T.untyped,
    )
    .returns(::T::Boolean)
  end
  def ==(other); end

  # Returns the value of an attribute.
  #
  # ```ruby
  # require "ostruct"
  # person = OpenStruct.new("name" => "John Smith", "age" => 70)
  # person[:age]   # => 70, same as person.age
  # ```
  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def [](name); end

  # Sets the value of an attribute.
  #
  # ```ruby
  # require "ostruct"
  # person = OpenStruct.new("name" => "John Smith", "age" => 70)
  # person[:age] = 42   # equivalent to person.age = 42
  # person.age          # => 42
  # ```
  sig do
    params(
      name: ::T.untyped,
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def []=(name, value); end

  # Removes the named field from the object. Returns the value that the field
  # contained if it was defined.
  #
  # ```ruby
  # require "ostruct"
  #
  # person = OpenStruct.new(name: "John", age: 70, pension: 300)
  #
  # person.delete_field("age")   # => 70
  # person                       # => #<OpenStruct name="John", pension=300>
  # ```
  #
  # Setting the value to `nil` will not remove the attribute:
  #
  # ```ruby
  # person.pension = nil
  # person                 # => #<OpenStruct name="John", pension=nil>
  # ```
  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def delete_field(name); end

  # Extracts the nested value specified by the sequence of `name` objects by
  # calling `dig` at each step, returning `nil` if any intermediate step is
  # `nil`.
  #
  # ```ruby
  # require "ostruct"
  # address = OpenStruct.new("city" => "Anytown NC", "zip" => 12345)
  # person  = OpenStruct.new("name" => "John Smith", "address" => address)
  #
  # person.dig(:address, "zip")            # => 12345
  # person.dig(:business_address, "zip")   # => nil
  #
  # data = OpenStruct.new(:array => [1, [2, 3]])
  #
  # data.dig(:array, 1, 0)   # => 2
  # data.dig(:array, 0, 0)   # TypeError: Integer does not have #dig method
  # ```
  sig do
    params(
      name: ::T.untyped,
      names: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dig(name, *names); end

  # Yields all attributes (as symbols) along with the corresponding values or
  # returns an enumerator if no block is given.
  #
  # ```ruby
  # require "ostruct"
  # data = OpenStruct.new("country" => "Australia", :capital => "Canberra")
  # data.each_pair.to_a   # => [[:country, "Australia"], [:capital, "Canberra"]]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Symbol, arg1: ::T.untyped).returns(BasicObject),
    )
    .returns(T::Array[[Symbol, ::T.untyped]])
  end
  sig {returns(T::Enumerator[[Symbol,::T.untyped]])}
  def each_pair(&blk); end

  # Compares this object and `other` for equality. An
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html) is eql?
  # to `other` when `other` is an
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html) and the
  # two objects' [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) tables
  # are eql?.
  sig do
    params(
      other: ::T.untyped,
    )
    .returns(::T::Boolean)
  end
  def eql?(other); end

  sig {returns(::T.untyped)}
  def freeze(); end

  # Computes a hash code for this
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html). Two
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html) objects
  # with the same content will have the same hash code (and will compare using
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html#method-i-eql-3F)).
  #
  # See also
  # [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash).
  sig {returns(Integer)}
  def hash(); end

  sig do
    params(
      hash: ::T.untyped,
    )
    .void
  end
  def initialize(hash=T.unsafe(nil)); end

  # Returns a string containing a detailed summary of the keys and values.
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html#method-i-to_s)
  sig {returns(String)}
  def inspect(); end

  # Provides marshalling support for use by the
  # [`Marshal`](https://docs.ruby-lang.org/en/2.7.0/Marshal.html) library.
  sig {returns(::T.untyped)}
  def marshal_dump(); end

  # Provides marshalling support for use by the
  # [`Marshal`](https://docs.ruby-lang.org/en/2.7.0/Marshal.html) library.
  sig do
    params(
      x: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def marshal_load(x); end

  sig do
    params(
      mid: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def method_missing(mid, *args); end

  sig {returns(::T.untyped)}
  def modifiable(); end

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def new_ostruct_member(name); end

  sig {returns(::T.untyped)}
  def table(); end

  sig {returns(::T.untyped)}
  def table!(); end

  # Converts the
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html) to a
  # hash with keys representing each attribute (as symbols) and their
  # corresponding values.
  #
  # If a block is given, the results of the block on each pair of the receiver
  # will be used as pairs.
  #
  # ```ruby
  # require "ostruct"
  # data = OpenStruct.new("country" => "Australia", :capital => "Canberra")
  # data.to_h   # => {:country => "Australia", :capital => "Canberra" }
  # data.to_h {|name, value| [name.to_s, value.upcase] }
  #             # => {"country" => "AUSTRALIA", "capital" => "CANBERRA" }
  # ```
  sig {returns(::T::Hash[Symbol, ::T.untyped])}
  def to_h(); end

  # Alias for:
  # [`inspect`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html#method-i-inspect)
  sig {returns(String)}
  def to_s(); end
end
