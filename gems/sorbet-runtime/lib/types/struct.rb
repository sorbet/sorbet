# frozen_string_literal: true
# typed: true

class T::InexactStruct
  # Avoids adding a call cache entry to every struct class; see T::Private::ClassUtils.
  INSTANCE_METHOD = Module.instance_method(:instance_method)
  private_constant :INSTANCE_METHOD

  include T::Props
  include T::Props::Serializable
  include T::Props::Constructor
end

class T::Struct < T::InexactStruct
  def self.inherited(subclass)
    super(subclass)
    original_method = INSTANCE_METHOD.bind_call(subclass.singleton_class, :inherited)
    T::Private::ClassUtils.replace_method(original_method, subclass.singleton_class, :inherited) do |s|
      super(s)
      raise "#{self.name} is a subclass of T::Struct and cannot be subclassed"
    end
  end
end

class T::ImmutableStruct < T::InexactStruct
  extend T::Sig

  def self.inherited(subclass)
    super(subclass)

    original_method = INSTANCE_METHOD.bind_call(subclass.singleton_class, :inherited)
    T::Private::ClassUtils.replace_method(original_method, subclass.singleton_class, :inherited) do |s|
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
