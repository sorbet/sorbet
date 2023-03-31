# typed: true

class A1
  extend T::Generic
  has_attached_class! # error: `has_attached_class!` can only be used inside a `module`, not a `class`
end

module A2
  extend T::Generic
  has_attached_class!(:in)
  #                   ^^^ error: `has_attached_class!` cannot be declared `:in`, only invariant or `:out`
end

module A3
  has_attached_class!
 #^^^^^^^^^^^^^^^^^^^ error: does not exist
 #^^^^^^^^^^^^^^^^^^^ error: does not exist
end
