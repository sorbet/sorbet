# typed: __STDLIB_INTERNAL

# Class Data provides a convenient way to define simple classes for value-alike
# objects.
#
# The simplest example of usage:
#
# ```ruby
# Measure = Data.define(:amount, :unit)
#
# # Positional arguments constructor is provided
# distance = Measure.new(100, 'km')
# #=> #<data Measure amount=100, unit="km">
#
# # Keyword arguments constructor is provided
# weight = Measure.new(amount: 50, unit: 'kg')
# #=> #<data Measure amount=50, unit="kg">
#
# # Alternative form to construct an object:
# speed = Measure[10, 'mPh']
# #=> #<data Measure amount=10, unit="mPh">
#
# # Works with keyword arguments, too:
# area = Measure[amount: 1.5, unit: 'm^2']
# #=> #<data Measure amount=1.5, unit="m^2">
#
# # Argument accessors are provided:
# distance.amount #=> 100
# distance.unit #=> "km"
# ```
#
# Constructed object also has a reasonable definitions of
# [`#==`](https://docs.ruby-lang.org/en/3.2/Data.html#method-i-3D-3D)
# operator, [`#to_h`](https://docs.ruby-lang.org/en/3.2/Data.html#method-i-to_h)
# hash conversion, and
# [`#deconstruct`](https://docs.ruby-lang.org/en/3.2/Data.html#method-i-deconstruct)/#deconstruct_keys
# to be used in pattern matching.
#
# [`::define`](https://docs.ruby-lang.org/en/3.2/Data.html#method-c-define)
# method accepts an optional block and evaluates it in the context of the
# newly defined class. That allows to define additional methods:
#
# ```ruby
# Measure = Data.define(:amount, :unit) do
#   def <=>(other)
#     return unless other.is_a?(self.class) && other.unit == unit
#     amount <=> other.amount
#   end
#
#   include Comparable
# end
#
# Measure[3, 'm'] < Measure[5, 'm'] #=> true
# Measure[3, 'm'] < Measure[5, 'kg']
# # comparison of Measure with Measure failed (ArgumentError)
# ```
#
# Data provides no member writers, or enumerators: it is meant to be a storage
# for immutable atomic values. But note that if some of data members is of a
# mutable class, Data does no additional immutability enforcement:
#
# ```ruby
# Event = Data.define(:time, :weekdays)
# event = Event.new('18:00', %w[Tue Wed Fri])
# #=> #<data Event time="18:00", weekdays=["Tue", "Wed", "Fri"]>
#
# # There is no #time= or #weekdays= accessors, but changes are
# # still possible:
# event.weekdays << 'Sat'
# event
# #=> #<data Event time="18:00", weekdays=["Tue", "Wed", "Fri", "Sat"]>
# ```
#
# See also [`Struct`](https://docs.ruby-lang.org/en/3.2/Struct.html), which is
# a similar concept, but has more container-alike API, allowing to change
# contents of the object and enumerate it.
class Data < Object

  # Defines a new Data class.
  #
  # ```ruby
  # measure = Data.define(:amount, :unit)
  # #=> #<Class:0x00007f70c6868498>
  # measure.new(1, 'km')
  # #=> #<data amount=1, unit="km">
  #
  # # It you store the new class in the constant, it will
  # # affect #inspect and will be more natural to use:
  # Measure = Data.define(:amount, :unit)
  # #=> Measure
  # Measure.new(1, 'km')
  # #=> #<data Measure amount=1, unit="km">
  # ```
  #
  # Note that member-less Data is acceptable and might be a useful technique
  # for defining several homogenous data classes, like
  #
  # ```ruby
  # class HTTPFetcher
  # Response = Data.define(:body)
  # NotFound = Data.define
  # # ... implementation
  # end
  # ```
  #
  # Now, different kinds of responses from +HTTPFetcher+ would have consistent
  # representation:
  #
  # ```ruby
  # #<data HTTPFetcher::Response body="<html...">
  # #<data HTTPFetcher::NotFound>
  # ```
  #
  # And are convenient to use in pattern matching:
  #
  # ```ruby
  # case fetcher.get(url)
  # in HTTPFetcher::Response(body)
  #   # process body variable
  # in HTTPFetcher::NotFound
  #   # handle not found case
  # end
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Symbol, String),
        blk: T.untyped,
    )
    .returns(Data)
  end
  def self.define(arg0, *arg1, &blk); end

  # Returns an array of member names of the data class:
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  # Measure.members # => [:amount, :unit]
  # ```
  sig { returns(T::Array[Symbol]) }
  def self.members; end

  # Constructors for classes defined with
  # [::define](https://docs.ruby-lang.org/en/3.2/Data.html#method-c-define)
  # accept both positional and keyword arguments.
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  #
  # Measure.new(1, 'km')
  # #=> #<data Measure amount=1, unit="km">
  # Measure.new(amount: 1, unit: 'km')
  # #=> #<data Measure amount=1, unit="km">
  #
  # # Alternative shorter intialization with []
  # Measure[1, 'km']
  # #=> #<data Measure amount=1, unit="km">
  # Measure[amount: 1, unit: 'km']
  # #=> #<data Measure amount=1, unit="km">
  # ```
  #
  # All arguments are mandatory (unlike [Struct](https://docs.ruby-lang.org/en/3.2/Struct.html)),
  # and converted to keyword arguments:
  #
  # ```ruby
  # Measure.new(amount: 1)
  # # in `initialize': missing keyword: :unit (ArgumentError)
  #
  # Measure.new(1)
  # # in `initialize': missing keyword: :unit (ArgumentError)
  # ```
  #
  # Note that `Measure#initialize` always receives keyword arguments, and that
  # mandatory arguments are checked in `initialize`, not in `new`. This can be
  # important for redefining initialize in order to convert arguments or provide
  # defaults:
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit) do
  #   NONE = Data.define
  #
  #   def initialize(amount:, unit: NONE.new)
  #     super(amount: Float(amount), unit:)
  #   end
  # end
  #
  # Measure.new('10', 'km') # => #<data Measure amount=10.0, unit="km">
  # Measure.new(10_000)     # => #<data Measure amount=10000.0, unit=#<data NONE>>
  # ```ruby
  #
  # ```
  # static VALUE
  # rb_data_initialize_m(int argc, const VALUE *argv, VALUE self)
  # {
  #     VALUE klass = rb_obj_class(self);
  #     rb_struct_modify(self);
  #     VALUE members = struct_ivar_get(klass, id_members);
  #     size_t num_members = RARRAY_LEN(members);
  #
  #     if (argc == 0) {
  #         if (num_members > 0) {
  #             rb_exc_raise(rb_keyword_error_new("missing", members));
  #         }
  #         return Qnil;
  #     }
  #     if (argc > 1 || !RB_TYPE_P(argv[0], T_HASH)) {
  #         rb_error_arity(argc, 0, 0);
  #     }
  #
  #     if (RHASH_SIZE(argv[0]) < num_members) {
  #         VALUE missing = rb_ary_diff(members, rb_hash_keys(argv[0]));
  #         rb_exc_raise(rb_keyword_error_new("missing", missing));
  #     }
  #
  #     struct struct_hash_set_arg arg;
  #     rb_mem_clear((VALUE *)RSTRUCT_CONST_PTR(self), num_members);
  #     arg.self = self;
  #     arg.unknown_keywords = Qnil;
  #     rb_hash_foreach(argv[0], struct_hash_set_i, (VALUE)&arg);
  #     // Freeze early before potentially raising, so that we don't leave an
  #     // unfrozen copy on the heap, which could get exposed via ObjectSpace.
  #     OBJ_FREEZE_RAW(self);
  #     if (arg.unknown_keywords != Qnil) {
  #         rb_exc_raise(rb_keyword_error_new("unknown", arg.unknown_keywords));
  #     }
  #     return Qnil;
  # }
  # ```
  sig { params(args: T.untyped).returns(Data) }
  sig { params(kwargs: T.untyped).returns(Data) }
  def new(*args, **kwargs); end

  sig { params(args: T.untyped).returns(Data) }
  sig { params(kwargs: T.untyped).returns(Data) }
  def self.[](*args, **kwargs); end

  # Returns `true` if `other` is the same class as `self`, and all members are
  # equal.
  #
  # Examples:
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  #
  # Measure[1, 'km'] == Measure[1, 'km'] #=> true
  # Measure[1, 'km'] == Measure[2, 'km'] #=> false
  # Measure[1, 'km'] == Measure[1, 'm']  #=> false
  #
  # Measurement = Data.define(:amount, :unit)
  # Even though Measurement and Measure have the same "shape"
  # their instances are never equal
  # Measure[1, 'km'] == Measurement[1, 'km'] #=> false
  #```
  sig { params(other: BasicObject).returns(T::Boolean) }
  def ==(other); end

  # Returns the values in `self` as an array, to use in pattern matching:
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  #
  # distance = Measure[10, 'km']
  # distance.deconstruct #=> [10, "km"]
  #
  # # usage
  # case distance
  # in n, 'km' # calls #deconstruct underneath
  #   puts "It is #{n} kilometers away"
  # else
  #   puts "Don't know how to handle it"
  # end
  # # prints "It is 10 kilometers away"
  # ```
  #
  # Or, with checking the class, too:
  #
  # ```
  # case distance
  # in Measure(n, 'km')
  #   puts "It is #{n} kilometers away"
  # # ...
  # end
  # ```
  sig { returns(T::Array[T.untyped]) }
  def deconstruct; end

  # Returns a hash of the name/value pairs, to use in pattern matching.
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  #
  # distance = Measure[10, 'km']
  # distance.deconstruct_keys(nil) #=> {:amount=>10, :unit=>"km"}
  # distance.deconstruct_keys([:amount]) #=> {:amount=>10}
  #
  # # usage
  # case distance
  # in amount:, unit: 'km' # calls #deconstruct_keys underneath
  #   puts "It is #{amount} kilometers away"
  # else
  #   puts "Don't know how to handle it"
  # end
  # # prints "It is 10 kilometers away"
  # ```
  #
  # Or, with checking the class, too:
  #
  # ```ruby
  # case distance
  # in Measure(amount:, unit: 'km')
  #   puts "It is #{amount} kilometers away"
  # # ...
  # end
  # ```
  sig do
    params(
      array_of_names_or_nil: T.nilable(T::Array[T.any(Symbol, String)])
    )
    .returns(T::Hash[Symbol, T.untyped])
  end
  def deconstruct_keys(array_of_names_or_nil); end

  # Equality check that is used when two items of data are keys of a
  # [`Hash`](https://docs.ruby-lang.org/en/3.2/Hash.html).
  #
  # The subtle difference with
  # [`==`](https://docs.ruby-lang.org/en/3.2/Data.html#method-i-3D-3D) is that
  # members are also compared with their
  # [`eql?`](https://docs.ruby-lang.org/en/3.2/Data.html#method-i-eql-3F)
  # method, which might be important in some cases:
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  #
  # Measure[1, 'km'] == Measure[1.0, 'km'] #=> true, they are equal as values
  # # ...but...
  # Measure[1, 'km'].eql? Measure[1.0, 'km'] #=> false, they represent different hash keys
  # ```
  #
  # See also
  # [`Object#eql?`](https://docs.ruby-lang.org/en/3.2/Object.html#method-i-eql-3F)
  # for further explanations of the method usage.
  sig { params(other: BasicObject).returns(T::Boolean) }
  def eql?(other); end

  # Redefines
  # [`Object#hash`](https://docs.ruby-lang.org/en/3.2/Object.html#method-i-hash)
  # (used to distinguish objects as
  # [`Hash`](https://docs.ruby-lang.org/en/3.2/Hash.html) keys) so that data
  # objects of the same class with same content would have the same `hash``
  # value, and represented the same Hash key.
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  #
  # Measure[1, 'km'].hash == Measure[1, 'km'].hash #=> true
  # Measure[1, 'km'].hash == Measure[10, 'km'].hash #=> false
  # Measure[1, 'km'].hash == Measure[1, 'm'].hash #=> false
  # Measure[1, 'km'].hash == Measure[1.0, 'km'].hash #=> false
  #
  # # Structurally similar data class, but shouldn't be considered
  # # the same hash key
  # Measurement = Data.define(:amount, :unit)
  #
  # Measure[1, 'km'].hash == Measurement[1, 'km'].hash #=> false
  # ```
  sig { returns(Integer) }
  def hash; end

  # Returns a string representation of `self`:
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  #
  # distance = Measure[10, 'km']
  #
  # p distance  # uses #inspect underneath
  # #<data Measure amount=10, unit="km">
  #
  # puts distance  # uses #to_s underneath, same representation
  # #<data Measure amount=10, unit="km">
  # ```
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/3.2/Data.html#method-i-to_s)
  sig { returns(String) }
  def inspect; end

  # Returns the member names from self as an array:
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  # distance = Measure[10, 'km']
  #
  # distance.members #=> [:amount, :unit]
  # ```
  sig { returns(T::Array[Symbol]) }
  def members; end

  # Returns [`Hash`](https://docs.ruby-lang.org/en/3.2/Hash.html) representation
  # of the data object.
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  # distance = Measure[10, 'km']

  # distance.to_h
  # #=> {:amount=>10, :unit=>"km"}
  # ```
  #
  # Like
  # [`Enumerable#to_h`](https://docs.ruby-lang.org/en/3.2/Enumerable.html#method-i-to_h),
  # if the block is provided, it is expected to produce key-value pairs to
  # construct a hash:
  #
  # ```ruby
  # distance.to_h { |name, val| [name.to_s, val.to_s] }
  # #=> {"amount"=>"10", "unit"=>"km"}
  # ```
  #
  # Note that there is a useful symmetry between
  # [`to_h`](https://docs.ruby-lang.org/en/3.2/Data.html#method-i-to_h)
  # and initialize:
  #
  # ```ruby
  # distance2 = Measure.new(**distance.to_h)
  # #=> #<data Measure amount=10, unit="km">
  # distance2 == distance
  # #=> true
  # ```
  sig do
    params(
      blk: T.nilable(
        T.proc.params(name: Symbol, val: T.untyped).returns(T.untyped)
      )
    )
    .returns(T::Hash[T.untyped, T.untyped])
  end
  def to_h(&blk); end

  # Returns a string representation of `self`:
  #
  # ```ruby
  # Measure = Data.define(:amount, :unit)
  #
  # distance = Measure[10, 'km']
  #
  # p distance  # uses #inspect underneath
  # #<data Measure amount=10, unit="km">
  #
  # puts distance  # uses #to_s underneath, same representation
  # #<data Measure amount=10, unit="km">
  # ```
  #
  # Alias for:
  # [`inspect`](https://docs.ruby-lang.org/en/3.2/Data.html#method-i-inspect)
  sig { returns(String) }
  def to_s; end

  # Returns a shallow copy of `self` — the instance variables of `self` are
  # copied, but not the objects they reference.

  # If the method is supplied any keyword arguments, the copy will be created
  # with the respective field values updated to use the supplied keyword
  # argument values. Note that it is an error to supply a keyword that the
  # [`Data`](https://docs.ruby-lang.org/en/3.2/Data.html) class does not have
  # as a member.
  #
  # ```ruby
  # Point = Data.define(:x, :y)
  #
  # origin = Point.new(x: 0, y: 0)
  #
  # up = origin.with(x: 1)
  # right = origin.with(y: 1)
  # up_and_right = up.with(y: 1)
  #
  # p origin       # #<data Point x=0, y=0>
  # p up           # #<data Point x=1, y=0>
  # p right        # #<data Point x=0, y=1>
  # p up_and_right # #<data Point x=1, y=1>
  #
  # out = origin.with(z: 1) # ArgumentError: unknown keyword: :z
  # some_point = origin.with(1, 2) # ArgumentError: expected keyword arguments, got positional arguments
  # ```
  sig { params(kwargs: T.untyped).returns(T.self_type) }
  def with(**kwargs); end
end
