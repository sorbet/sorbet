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
