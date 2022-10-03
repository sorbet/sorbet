# typed: true
# enable-experimental-requires-ancestor: true

class R1; end
class R2 < R1; end
class R3; end

### Module requires unrelated classes

module Test1
  module M1
    extend T::Helpers
    requires_ancestor { R1 }
    requires_ancestor { R2 }
  end

  module M2 # error: `Test1::M2` requires unrelated classes `R1` and `R3` making it impossible to include
    extend T::Helpers
    requires_ancestor { R1 }
    requires_ancestor { R3 }
  end
end

### Module requires unrelated classes from included modules

module Test2
  module M1
    extend T::Helpers
    requires_ancestor { R1 }
  end

  module M2
    extend T::Helpers
    requires_ancestor { R3 }
  end

  module M3 # error: `Test2::M3` requires unrelated classes `R3` and `R1` making it impossible to include
    include M1
    include M2
  end
end

### Module requires unrelated classes from extended modules

module Test3
  module M1
    extend T::Helpers
    requires_ancestor { R1 }
  end

  module M2
    extend T::Helpers
    requires_ancestor { R3 }
  end

  module M3
# ^^^^^^^^^ error: `T.class_of(Test3::M3)` requires unrelated classes `R1` and `R3` making it impossible to inherit
# ^^^^^^^^^ error: `T.class_of(Test3::M3)` must inherit `R1` (required by `Test3::M1`)
# ^^^^^^^^^ error: `T.class_of(Test3::M3)` must inherit `R3` (required by `Test3::M2`)
    extend M1, M2
  end
end

### Abstract class requires unrelated classes

module Test4
  class C1
# ^^^^^^^^ error: `Test4::C1` requires unrelated class `R2` making it impossible to inherit
# ^^^^^^^^ error: `Test4::C1` requires unrelated class `R3` making it impossible to inherit
# ^^^^^^^^ error: `Test4::C1` requires unrelated classes `R2` and `R3` making it impossible to inherit
    extend T::Helpers
    abstract!
    requires_ancestor { R2 }
    requires_ancestor { R3 }
  end

  class C2 < C1
# ^^^^^^^^^^^^^ error: `Test4::C2` requires unrelated class `R2` making it impossible to inherit
# ^^^^^^^^^^^^^ error: `Test4::C2` requires unrelated class `R3` making it impossible to inherit
# ^^^^^^^^^^^^^ error: `Test4::C2` requires unrelated classes `R2` and `R3` making it impossible to inherit
    extend T::Helpers
    abstract!
  end
end

### Transitive unrelated classes from included modules

module Test5
  module M1
    extend T::Helpers
    requires_ancestor { R2 }
  end

  module M2 # error: `Test5::M2` requires unrelated classes `R3` and `R2` making it impossible to include
    extend T::Helpers
    include M1
    requires_ancestor { R3 }
  end

  class C1 < R2
# ^^^^^^^^^^^^^ error: `Test5::C1` requires unrelated class `R3` making it impossible to inherit
# ^^^^^^^^^^^^^ error: `Test5::C1` requires unrelated classes `R3` and `R2` making it impossible to inherit
    extend T::Helpers
    abstract!
    include M2
  end
end

### Abstract module requires unrelated classes

module Test6
  module M1
    extend T::Helpers
    abstract!
    requires_ancestor { R2 }
  end

  module M2
# ^^^^^^^^^ error: `Test6::M2` requires unrelated classes `R2` and `R3` making it impossible to include
    extend T::Helpers
    abstract!
    requires_ancestor { R2 }
    requires_ancestor { R3 }
  end

  module M3
# ^^^^^^^^^ error: `Test6::M3` requires unrelated classes `R2` and `R3` making it impossible to include
    include M2
    extend T::Helpers
    abstract!
  end
end

### Interface module requires unrelated classes

module Test7
  module M1
    extend T::Helpers
    interface!
    requires_ancestor { R2 }
  end

  module M2
# ^^^^^^^^^ error: `Test7::M2` requires unrelated classes `R2` and `R3` making it impossible to include
    extend T::Helpers
    interface!
    requires_ancestor { R2 }
    requires_ancestor { R3 }
  end

  module M3
# ^^^^^^^^^ error: `Test7::M3` requires unrelated classes `R2` and `R3` making it impossible to include
    include M2
    extend T::Helpers
    interface!
  end
end
