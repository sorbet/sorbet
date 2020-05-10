# frozen_string_literal: true
# typed: true

class T::InexactStruct
  extend T::Sig

  include T::Props
  include T::Props::Serializable
  include T::Props::Constructor

  sig {params(other: Object).returns(T::Boolean)}
  def ==(other)
    return true if self.equal?(other)
    return false unless other.is_a?(self.class)

    self.class.props.all? { |prop, _| self.send(prop) == other.send(prop) }
  end
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
