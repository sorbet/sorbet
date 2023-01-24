# typed: true
# enable-experimental-requires-ancestor: true

module R1; end
module R2; end

### Basic include

module Test1
  module M1
    extend T::Helpers
    requires_ancestor { R1 }
  end

  class C1 # error: `Test1::C1` must include `R1` (required by `Test1::M1`)
    include M1
  end

  class C2
    include M1
    include R1
  end
end

### Abstract include

module Test2
  module M1
    extend T::Helpers
    requires_ancestor { R1 }
  end

  module M2
    extend T::Helpers
    interface!
    requires_ancestor { R2 }
  end

  class C1
    extend T::Helpers
    abstract!
    include M1
    include M2
  end

   class C2 < C1
 # ^^^^^^^^^^^^^ error: `Test2::C2` must include `R1` (required by `Test2::M1`)
 # ^^^^^^^^^^^^^ error: `Test2::C2` must include `R2` (required by `Test2::M2`)
   end

  class C3 < C2
    include R1
    include R2
  end
end

### Transitive requirement

module Test3
  module M1
    extend T::Helpers
    requires_ancestor { R1 }
  end

  module M2
    include M1
  end

  module M3
    include M2
  end

  class C1 # error: `Test3::C1` must include `R1` (required by `Test3::M1`)
    include M3
  end

  class C2 < C1 # error: `Test3::C2` must include `R1` (required by `Test3::M1`)
  end

  class C3
    include M3
    include R1
  end
end

### Class requirement

module Test4
  module M1
    extend T::Helpers
    requires_ancestor { C1 }
  end

  class C1
    include M1
  end

  class C2 # error: `Test4::C2` must inherit `Test4::C1` (required by `Test4::M1`)
    include M1
  end

  class C3 < C1
  end
end

### Inherited requirement

module Test5
  module M1
    extend T::Helpers
    requires_ancestor { C2 }
  end

  class C1
    extend T::Helpers
    abstract!
  end

  class C2 < C1
    extend T::Helpers
    abstract!
  end

  class C3 < C1 # error: `Test5::C3` must inherit `Test5::C2` (required by `Test5::M1`)
    include M1
  end

  class C4 < C3 # error: `Test5::C4` must inherit `Test5::C2` (required by `Test5::M1`)
  end

  class C5 < C2
    include M1
  end

  class C6 < C5
  end
end

### Circular requirement

module Test6
  module M1
    extend T::Helpers
    requires_ancestor { M2 }
  end

  module M2
    extend T::Helpers
    requires_ancestor { M1 }
  end

  class C1 # error: `Test6::C1` must include `Test6::M2` (required by `Test6::M1`)
    include M1
  end

  class C2 # error: `Test6::C2` must include `Test6::M1` (required by `Test6::M2`)
    include M2
  end

  class C3
    include M1, M2
  end
end

### Circular requirement with inheritance

module Test7
  module M1
    extend T::Helpers
    requires_ancestor { M2 }
  end

  module M2
    extend T::Helpers
    requires_ancestor { C2 }
  end

  class C1
    extend T::Helpers
    abstract!
    requires_ancestor { M1 }
  end

  class C2 < C1
    extend T::Helpers
    abstract!
  end

  class C3 < C2
# ^^^^^^^^^^^^^ error: `Test7::C3` must include `Test7::M1` (required by `Test7::C1`)
# ^^^^^^^^^^^^^ error: `Test7::C3` must include `Test7::M2` (required by `Test7::M1`)
  end

  class C4 < C3
# ^^^^^^^^^^^^^ error: `Test7::C4` must include `Test7::M1` (required by `Test7::C1`)
# ^^^^^^^^^^^^^ error: `Test7::C4` must include `Test7::M2` (required by `Test7::M1`)
  end

  class C5 < C3
    include M1
    include M2
  end
end

### Requirements from include

module Test8
  module M1
    extend T::Helpers
    requires_ancestor { R1 }
  end

  module M2
    include M1
  end

  class C1 # error: `Test8::C1` must include `R1` (required by `Test8::M1`)
    include M2
  end

  class C2 < C1 # error: `Test8::C2` must include `R1` (required by `Test8::M1`)
  end

  class C3 < C2
    include R1
  end
end

### Requirements from extend

module Test9
  module M1
    extend T::Helpers
    requires_ancestor { R1 }
  end

  module M2 # error: `T.class_of(Test9::M2)` must include `R1` (required by `Test9::M1`)
    extend M1
  end

  class C1
    include M2
  end

  class C2 < C1 # error: `T.class_of(Test9::C2)` must include `R1` (required by `Test9::M1`)
    extend M1
  end

  class C3 < C2
    extend R1
  end

  class C4 < C3
  end

  class C5 # error: `Test9::C5` must include `R1` (required by `Test9::M1`)
    include M1
  end

  class C6
    include M1
    include R1
  end

  class C7
    class << self # error: `T.class_of(Test9::C7)` must include `R1` (required by `Test9::M1`)
      include M1
    end
  end

  class C8
    class << self
      include M1
      include R1
    end
  end

  class C9
    class << self # error: `T.class_of(T.class_of(Test9::C9))` must include `R1` (required by `Test9::M1`)
      extend M1
    end
  end
end
