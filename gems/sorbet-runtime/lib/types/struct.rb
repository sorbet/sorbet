# frozen_string_literal: true
# typed: true

class T::InexactStruct
  include T::Props
  include T::Props::Serializable
  include T::Props::Constructor

  # Keyword-style construction (`Point.new(x: 1, y: 2)`) makes the VM pack the
  # keywords into a Hash once at this call boundary. Going through `Class#new`,
  # that Hash is repacked a second time to match the positional `hash`
  # parameter of the standard constructor, costing one extra transient Hash
  # allocation per constructed struct. With the standard constructor, forward
  # the packed Hash directly instead.
  #
  # Classes that define their own initialize (detected via the method_added
  # hook below) get the original `Class#new` semantics restored exactly, so
  # custom constructors see the same argument shapes as before.
  EMPTY_HASH = T.let({}.freeze, T::Hash[Symbol, T.untyped])
  private_constant :EMPTY_HASH

  def self.new(hash = EMPTY_HASH, &blk)
    instance = allocate
    instance.send(:initialize, hash, &blk)
    instance
  end

  # Resolved through subclasses' singleton ancestry: fires with self equal to
  # the class where a method is defined.
  def self.method_added(name)
    if name == :initialize && !equal?(T::ImmutableStruct) &&
       method(:new).owner.equal?(T::InexactStruct.singleton_class)
      # A custom initialize may expect keywords or positional arguments the
      # packed Hash above would not match; restore exact Class#new semantics.
      define_singleton_method(:new, Class.instance_method(:new))
    end
    super(name)
  end
end

class T::Struct < T::InexactStruct
  def self.inherited(subclass)
    super(subclass)
    original_method = subclass.singleton_class.instance_method(:inherited)
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

    original_method = subclass.singleton_class.instance_method(:inherited)
    T::Private::ClassUtils.replace_method(original_method, subclass.singleton_class, :inherited) do |s|
      super(s)
      raise "#{self.name} is a subclass of T::ImmutableStruct and cannot be subclassed"
    end
  end

  # Matches the one in WeakConstructor, but freezes the object. The frozen
  # constant default keeps zero-arg and keyword construction from allocating a
  # fresh `{}` on every call.
  EMPTY_HASH = T.let({}.freeze, T::Hash[Symbol, T.untyped])
  private_constant :EMPTY_HASH

  sig { params(hash: T::Hash[Symbol, T.untyped]).void.checked(:never) }
  def initialize(hash=EMPTY_HASH)
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
