# frozen_string_literal: true
# typed: true

class T::InexactStruct
  include T::Props
  include T::Props::Serializable
  include T::Props::Constructor

  def deconstruct_keys(keys)
    h = serialize

    return h.to_h { |k, v| [k.to_sym, v] } if keys.nil?
    raise TypeError, "wrong argument type #{keys.class} (expected Array or nil)" unless keys.is_a?(Array)
    return {} if h.count < keys.count

    result = {}

    keys.each do |k, v|
      case k
      when String then  result[k] = h.fetch(k)
      when Symbol then  result[k] = h.fetch(k.to_s)
      when Integer then result[k] = h.values.fetch(k)
      end
    rescue KeyError, IndexError
      break
    end

    result
  end

  def deconstruct = serialize.values
end

class T::Struct < T::InexactStruct
  def self.inherited(subclass)
    super(subclass)
    T::Private::ClassUtils.replace_method(subclass.singleton_class, :inherited, true) do |s|
      super(s)
      raise "#{self.name} is a subclass of T::Struct and cannot be subclassed"
    end
  end
end

class T::ImmutableStruct < T::InexactStruct
  extend T::Sig

  def self.inherited(subclass)
    super(subclass)

    T::Private::ClassUtils.replace_method(subclass.singleton_class, :inherited, true) do |s|
      super(s)
      raise "#{self.name} is a subclass of T::ImmutableStruct and cannot be subclassed"
    end
  end

  # Matches the one in WeakConstructor, but freezes the object
  sig { params(hash: T::Hash[Symbol, T.untyped]).void.checked(:never) }
  def initialize(hash={})
    super

    freeze
  end

  # Matches the signature in Props, but raises since this is an immutable struct and only const is allowed
  sig { params(name: Symbol, cls: T.untyped, rules: T.untyped).void }
  def self.prop(name, cls, **rules)
    return super if (cls.is_a?(Hash) && cls[:immutable]) || rules[:immutable]

    raise "Cannot use `prop` in #{self.name} because it is an immutable struct. Use `const` instead"
  end

  def with(changed_props)
    raise "Cannot use `with` in #{self.class.name} because it is an immutable struct"
  end
end
