# typed: true
# disable-fast-path: true
class Base
  extend T::Generic

  Elem = type_template
end

class Child < Base # error: Type `Elem` declared by parent `T.class_of(Base)` must be re-declared in `T.class_of(Child)`
end
