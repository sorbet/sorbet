# typed: strict

class C < Module
  extend T::Generic
  has_attached_class!
end

class D < Module
  extend T::Generic
end
