# frozen_string_literal: true
# typed: strict
require_relative '../../extn'
Opus::AutogenLoader.init(__FILE__)

# Enumerations allow for type-safe declarations of a fixed set of values.
#
# Every value is a singleton instance of the class (i.e. `Suit::SPADE.is_a?(Suit) == true`).
#
# Each value has a corresponding serialized value. By default this is the constant's name converted
# to lowercase (e.g. `Suit::Club.serialize == 'club'`); however a custom value may be passed to the
# constructor. Enum will `freeze` the serialized value.
#
# @example Declaring an Enum:
#   class Suit < T::Enum
#     enums do
#       CLUB = T.let(new, Suit)
#       SPADE = T.let(new, Suit)
#       DIAMOND = T.let(new, Suit)
#       HEART = T.let(new, Suit)
#     end
#   end
#
# @example Custom serialization value:
#   class Status < T::Enum
#     enums do
#       READY = T.let(new('rdy'), Status)
#       ...
#     end
#   end
#
# @example Accessing values:
#   Suit::SPADE
#
# @example Converting from serialized value to enum instance:
#   Suit.from_serialized('club') == Suit::CLUB
#
# @example Using enums in type signatures:
#   sig {params(suit: Suit).returns(Boolean)}
#   def is_red?(suit); ...; end
#
# WARNING: Enum instances are singletons that are shared among all their users. Their internals
# should be kept immutable to avoid unpredictable action at a distance.
class T::Enum
  extend T::Props::CustomType

  # TODO(jez) Might want to restrict this, or make subclasses provide this type
  SerializedVal = T.type_alias {T.untyped}
  private_constant :SerializedVal

  ## Enum class methods ##
  sig {returns(T::Array[T.experimental_attached_class])}
  def self.values
    if @values.nil?
      raise "Attempting to access values of #{self.class} before it has been initialized." \
        " Enums are not initialized until the 'enums do' block they are defined in has finished running."
    end
    @values
  end

  # Convert from serialized value to enum instance.
  #
  # Note: It would have been nice to make this method final before people started overriding it.
  # Note: Failed CriticalMethodsNoRuntimeTypingTest
  #
  # @return [self]
  # @raise [KeyError] if serialized value does not match any instance.
  sig {overridable.params(serialized_val: SerializedVal).returns(T.experimental_attached_class).checked(:never)}
  def self.from_serialized(serialized_val)
    if @mapping.nil?
      raise "Attempting to access serialization map of #{self.class} before it has been initialized." \
        " Enums are not initialized until the 'enums do' block they are defined in has finished running."
    end
    res = @mapping[serialized_val]
    if res.nil?
      raise KeyError.new("Enum #{self} key not found: #{serialized_val.inspect}")
    end
    res
  end

  # Note: It would have been nice to make this method final before people started overriding it.
  # @return [Boolean] Does the given serialized value correspond with any of this enum's values.
  sig {overridable.params(serialized_val: SerializedVal).returns(T::Boolean)}
  def self.has_serialized?(serialized_val)
    if @mapping.nil?
      raise "Attempting to access serialization map of #{self.class} before it has been initialized." \
        " Enums are not initialized until the 'enums do' block they are defined in has finished running."
    end
    @mapping.include?(serialized_val)
  end


  ## T::Props::CustomType

  # Note: Failed CriticalMethodsNoRuntimeTypingTest
  sig {params(value: T.untyped).returns(T::Boolean).checked(:never)}
  def self.instance?(value)
    value.is_a?(self)
  end

  # Note: Failed CriticalMethodsNoRuntimeTypingTest
  sig {params(instance: T.nilable(T::Enum)).returns(SerializedVal).checked(:never)}
  def self.serialize(instance)
    # This is needed otherwise if a Chalk::ODM::Document with a property of the shape
    # T::Hash[T.nilable(MyEnum), Integer] and a value that looks like {nil => 0} is
    # serialized, we throw the error on L102.
    return nil if instance.nil?

    if self == T::Enum
      Opus::Error.hard("Cannot call T::Enum.serialize directly. You must call on a specific child class.")
    end
    if instance.class != self
      Opus::Error.hard("Cannot call #serialize on a value that is not an instance of #{self}.")
    end
    instance.serialize
  end

  # Note: Failed CriticalMethodsNoRuntimeTypingTest
  sig {params(mongo_value: SerializedVal).returns(T.experimental_attached_class).checked(:never)}
  def self.deserialize(mongo_value)
    if self == T::Enum
      Opus::Error.hard("Cannot call T::Enum.deserialize directly. You must call on a specific child class.")
    end
    self.from_serialized(mongo_value)
  end


  ## Enum instance methods ##


  sig {returns(T.self_type)}
  def dup
    self
  end

  sig {returns(T.self_type).checked(:tests)}
  def clone
    self
  end

  # Note: Failed CriticalMethodsNoRuntimeTypingTest
  sig {returns(SerializedVal).checked(:never)}
  def serialize
    assert_bound!
    @serialized_val
  end

  sig {params(args: T.untyped).returns(T.untyped)}
  def to_json(*args)
    serialize.to_json(*args)
  end

  sig {returns(String)}
  def to_s
    inspect
  end

  sig {returns(String)}
  def inspect
    "#<#{self.class.name}::#{@const_name || '__UNINITIALIZED__'}>"
  end

  sig {params(other: BasicObject).returns(T.nilable(Integer))}
  def <=>(other)
    case other
    when self.class
      self.serialize <=> other.serialize
    else
      nil
    end
  end


  ## Migrating from `make_accessible` / `make_accessible_DEPRECATED` ##
  #
  # TODO [nroman] Remove these once the migration is complete

  # NB: Do not call this method. This exists to allow for a safe migration path in places where enum
  # values are compared directly against string values.
  #
  # Ruby's string has a weird quirk where `'my_string' == obj` calls obj.==('my_string') if obj
  # responds to the `to_str` method. It does not actually call `to_str` however.
  #
  # See https://ruby-doc.org/core-2.4.0/String.html#method-i-3D-3D
  sig {returns(String)}
  def to_str
    Opus::Error.soft(
      'Implicit conversion of Enum instances to strings is not allowed. Call #serialize instead.',
      storytime: {class: self.class.name},
      notify: 'nroman',
    )
    serialize.to_s
  end

  sig {params(other: BasicObject).returns(T::Boolean).checked(:never)}
  def ==(other)
    case other
    when String
      comparison_assertion_failed(:==, other)
      self.serialize == other
    else
      super(other)
    end
  end

  sig {params(other: BasicObject).returns(T::Boolean).checked(:never)}
  def ===(other)
    case other
    when String
      comparison_assertion_failed(:===, other)
      self.serialize == other
    else
      super(other)
    end
  end

  sig {params(method: Symbol, other: T.untyped).void}
  private def comparison_assertion_failed(method, other)
    Opus::Error.soft(
      'Enum to string comparison not allowed. Compare to the Enum instance directly instead. See go/enum-migration',
      storytime: {
        class: self.class.name,
        self: self.inspect,
        other: other,
        other_class: other.class.name,
        method: method,
      },
      notify: 'nroman',
    )
  end


  ## Private implementation ##


  sig {params(serialized_val: SerializedVal).void}
  private def initialize(serialized_val=nil)
    raise 'T::Enum is abstract' if self.class == T::Enum
    if !self.class.started_initializing?
      raise "Must instantiate all enum values of #{self.class} inside 'enums do'."
    end
    if self.class.fully_initialized?
      raise "Cannot instantiate a new enum value of #{self.class} after it has been initialized."
    end

    serialized_val = serialized_val.frozen? ? serialized_val : serialized_val.dup.freeze
    @serialized_val = T.let(serialized_val, T.nilable(SerializedVal))
    @const_name = T.let(nil, T.nilable(Symbol))
  end

  sig {returns(NilClass).checked(:never)}
  private def assert_bound!
    if @const_name.nil?
      raise "Attempting to access Enum value on #{self.class} before it has been initialized." \
        " Enums are not initialized until the 'enums do' block they are defined in has finished running."
    end
  end

  sig {params(const_name: Symbol).void}
  def _bind_name(const_name)
    @const_name = const_name
    @serialized_val = const_to_serialized_val(const_name) if @serialized_val.nil?
    freeze
  end

  sig {params(const_name: Symbol).returns(String)}
  private def const_to_serialized_val(const_name)
    # Historical note: We convert to lowercase names because the majority of existing calls to
    # `make_accessible` were arrays of lowercase strings. Doing this conversion allowed for the
    # least amount of repetition in migrated declarations.
    const_name.to_s.downcase.freeze
  end

  sig {returns(T::Boolean)}
  def self.started_initializing?
    @started_initializing = T.let(@started_initializing, T.nilable(T::Boolean))
    @started_initializing ||= false
  end

  sig {returns(T::Boolean)}
  def self.fully_initialized?
    @fully_initialized = T.let(@fully_initialized, T.nilable(T::Boolean))
    @fully_initialized ||= false
  end

  # Entrypoint for allowing people to register new enum values.
  # All enum values must be defined within this block.
  sig {params(blk: T.proc.void).void}
  def self.enums(&blk)
    raise "enums cannot be defined for T::Enum" if self == T::Enum
    raise "Enum #{self} was already initialized" if @fully_initialized
    raise "Enum #{self} is still initializing" if @started_initializing

    @started_initializing = true

    yield

    @values = T.let(nil, T.nilable(T::Array[T.experimental_attached_class]))
    @mapping = T.let(nil, T.nilable(T::Hash[SerializedVal, T.experimental_attached_class]))

    # Freeze the Enum class and bind the constant names into each of the instances.
    @mapping = {}
    self.constants(false).each do |const_name|
      instance = self.const_get(const_name, false)
      if !instance.is_a?(self)
        raise "Invalid constant #{self}::#{const_name} on enum. " \
          "All constants defined for an enum must be instances itself (e.g. `Foo = new`)."
      end

      instance._bind_name(const_name)
      serialized = instance.serialize
      if @mapping.include?(serialized)
        raise "Enum values must have unique serializations. Value '#{serialized}' is repeated on #{self}."
      end
      @mapping[serialized] = instance
    end
    @values = @mapping.values.sort.freeze
    @mapping.freeze
    @fully_initialized = true
  end

  sig {params(child_class: Module).void}
  def self.inherited(child_class)
    super

    raise "Inheriting from children of T::Enum is prohibited" if self != T::Enum
  end
end
