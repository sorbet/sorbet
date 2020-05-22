# typed: __STDLIB_INTERNAL

# A [`Struct`](https://docs.ruby-lang.org/en/2.6.0/Struct.html) is a convenient
# way to bundle a number of attributes together, using accessor methods, without
# having to write an explicit class.
#
# The [`Struct`](https://docs.ruby-lang.org/en/2.6.0/Struct.html) class
# generates new subclasses that hold a set of members and their values. For each
# member a reader and writer method is created similar to
# [`Module#attr_accessor`](https://docs.ruby-lang.org/en/2.6.0/Module.html#method-i-attr_accessor).
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
# [`Struct::new`](https://docs.ruby-lang.org/en/2.6.0/Struct.html#method-c-new)
# for further examples of creating struct subclasses and instances.
#
# In the method descriptions that follow, a "member" parameter refers to a
# struct member which is either a quoted string (`"name"`) or a
# [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html) (`:name`).
class Struct < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)

  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Symbol, String),
        keyword_init: T::Boolean,
    )
    .void
  end
  def initialize(arg0, *arg1, keyword_init: false); end

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
end
