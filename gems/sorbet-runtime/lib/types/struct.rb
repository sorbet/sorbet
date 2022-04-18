# frozen_string_literal: true
# typed: true

class T::InexactStruct
  include T::Props
  include T::Props::Serializable
  include T::Props::Constructor
end

class T::Struct < T::InexactStruct
  def self.inherited(subclass)
    super(subclass)
    T::Private::ClassUtils.replace_method(subclass.singleton_class, :inherited) do |s|
      super(s)
      raise "#{self.name} is a subclass of T::Struct and cannot be subclassed"
    end
  end
end

class T::ImmutableStruct
  include T::Props::Const
  include T::Props::Serializable
  include T::Props::Constructor

  def self.inherited(subclass)
    super(subclass)
    T::Private::ClassUtils.replace_method(subclass.singleton_class, :inherited) do |s|
      super(s)
      raise "#{self.name} is a subclass of T::ImmutableStruct and cannot be subclassed"
    end
  end

  def self.include(mod)
    if mod == T::Props::Prop || mod == T::Props
      raise "#{self.name} is a subclass of T::ImmutableStruct and cannot include T::Props"
    end

    super
  end

  def initialize(hash={})
    super
    freeze
  end
end
