# typed: true

class A < Class
  extend T::Generic
  has_attached_class!
end

class B < Class
  extend T::Generic
end
