# frozen_string_literal: true
# typed: true

module T::FinalStruct
  def self.included(other)
    T::Private::ClassUtils.replace_method(other.singleton_class, :inherited) do |s|
      super(s)

      reason = if self.ancestors.include?(T::Struct)
        "is a subclass of T::Struct"
      else
        "includes T::FinalStruct"
      end

      raise "#{self.name} #{reason} and cannot be subclassed"
    end
  end
end

class T::InexactStruct
  include T::Props
  include T::Props::Serializable
  include T::Props::Constructor
end

class T::Struct < T::InexactStruct
  def self.inherited(subclass)
    super(subclass)
    subclass.include(T::FinalStruct)
  end
end
