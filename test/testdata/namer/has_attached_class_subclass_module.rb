# typed: strict
# disable-fast-path: true

class C < Module
  extend T::Generic
  has_attached_class!
end

class D < Module # error: `has_attached_class!` declared by parent `Module` must be re-declared in `D`
  extend T::Generic
end
