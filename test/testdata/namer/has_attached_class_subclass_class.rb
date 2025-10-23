# typed: true
# disable-fast-path: true

class A < Class
  extend T::Generic
  has_attached_class!
end

class B < Class # error: is a subclass of `Class` which is not allowed
  extend T::Generic
end
