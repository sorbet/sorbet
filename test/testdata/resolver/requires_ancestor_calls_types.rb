# typed: true
# enable-experimental-requires-ancestor: true

module RA1
  def foo; end
end

class RA2
  def bar; end
end

### Resolve calls on implicit `self`

module Test1
  module M1
    extend T::Helpers
    requires_ancestor { RA1 }

    def m1
      foo
    end
  end
end

### Resolve calls on explicit `self`

module Test2
  module M1
    extend T::Helpers
    requires_ancestor { RA1 }

    def m1
      self.foo
    end
  end
end

### Resolve calls on `T.self_type`

module Test3
  module M1
    extend T::Helpers
    extend T::Sig
    requires_ancestor { RA1 }

    def m1
      get_me.foo
      get_m1.foo
    end

    sig { returns(T.self_type) }
    def get_me
      self
    end

    sig { returns(M1) }
    def get_m1
      self
    end
  end
end

### Resolve calls on `T.any`

module Test4
  module M1
    extend T::Helpers
    extend T::Sig
    requires_ancestor { Kernel }
    requires_ancestor { RA1 }

    def m1
      any_m1_ra1.foo

      m = any_m1_ra2
      if m.is_a?(M1)
        m.foo
      else
        m.bar
      end
    end

    sig { returns(T.any(M1, RA1)) }
    def any_m1_ra1
      self
    end

    sig { returns(T.any(M1, RA2)) }
    def any_m1_ra2
      self
    end
  end
end

### Resolve calls on `T.all`

module Test5
  module M1
    extend T::Helpers
    extend T::Sig
    requires_ancestor { RA2 }
    include RA1

    def m1
      all_m1_ra1.foo
    end

    sig { returns(T.all(M1, RA1)) }
    def all_m1_ra1
      self
    end
  end
end

### Resolve calls on `T.nilable`

module Test6
  module M1
    extend T::Helpers
    extend T::Sig
    requires_ancestor { RA1 }

    def m1
      m = m1?
      m.foo if m
      T.must(m).foo
    end

    sig { returns(T.nilable(M1)) }
    def m1?
      self
    end
  end
end

### Resolve calls on tuples

module Test7
  module M1
    extend T::Helpers
    extend T::Sig
    requires_ancestor { RA1 }
    requires_ancestor { RA2 }

    def m1
      m = tuple
      T.must(m.first).foo
      T.must(m.last).bar
    end

    sig { returns([M1, M1]) }
    def tuple
      [self, self]
    end
  end
end

### Resolve calls on shapes

module Test8
  module M1
    extend T::Helpers
    extend T::Sig
    requires_ancestor { RA1 }
    requires_ancestor { RA2 }

    def m1
      m = shape
      m[:ra1].foo
      m[:ra2].bar
    end

    sig { returns({ra1: M1, ra2: M1}) }
    def shape
      {ra1: self, ra2: self}
    end
  end
end

### Resolve calls on generics

module Test9
  module M1
    extend T::Helpers
    requires_ancestor { RA1 }
  end

  class C1
    extend T::Generic
    extend T::Sig
    Elem = type_member {{upper: M1}}

    sig { params(m1: Elem).returns(Elem) }
    def elem(m1)
      m1.foo
      m1
    end
  end

  class C2
    include M1
    include RA1
  end

  def self.main
    C1[M1].new.elem(C2.new).foo
  end
end

### Resolve calls on singleton

module Test10
  module M1
    extend T::Helpers
    requires_ancestor { RA1 }
  end

  module M2
    extend M1
    extend RA1
    extend T::Sig

    def self.m2
      foo
    end

    def m2
      T.attached_class.foo
    # ^^^^^^^^^^^^^^^^ error: `Test10::M2` must declare `has_attached_class!` before module instance methods can use `T.attached_class
      T.class_of(M2).foo
      #              ^^^ error: Call to method `foo` on `T.class_of(Test10::M2)`
    end
  end
end

### Resolve calls on struct types

module Test11
  module M1
    extend T::Helpers
    requires_ancestor { RA1 }
  end

  class C1
    include M1
    include RA1
  end

  class S1 < T::Struct
    prop :m1, M1
  end

  def self.main
    S1.new(m1: C1.new).m1.foo
  end
end
