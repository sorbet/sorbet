# typed: true
# enable-experimental-requires-ancestor: true

### Resolve calls on directly required module

module Test1
  module M1
    extend T::Helpers
    requires_ancestor { M2 }

    def m1
      m2
    end
  end

  module M2
    def m2; end
  end
end

### Resolve calls on module directly included by requirement

module Test2
  module M1
    extend T::Helpers
    requires_ancestor { M3 }

    def m1
      m2
      m3
    end
  end

  module M2
    def m2; end
  end

  module M3
    include M2
    def m3; end
  end
end

### Resolve calls on module transitively included by requirement

module Test3
  module M1
    extend T::Helpers
    requires_ancestor { M4 }

    def m1
      m2
      m3
      m4
    end
  end

  module M2
    def m2; end
  end

  module M3
    include M2
    def m3; end
  end

  module M4
    include M3
    def m4; end
  end
end

### Resolve calls on transitively required modules

module Test4
  module M1
    def m1; end
  end

  module M2
    extend T::Helpers
    requires_ancestor { M1 }

    def m2
      m1
    end
  end

  module M3
    extend T::Helpers
    requires_ancestor { M2 }

    def m3
      m1
      m2
    end
  end

  module M4
    extend T::Helpers
    requires_ancestor { M3 }

    def m4
      m1
      m2
      m3
    end
  end

  class C1
    include M1
    include M2
    include M3
    include M4

    def c1
      m1
      m2
      m3
      m4
    end
  end
end

### Resolve calls on directly included abstract class

module Test5
  module M1
    extend T::Helpers
    requires_ancestor { C1 }

    def m1
      c1
    end
  end

  class C1
    extend T::Helpers
    abstract!
    def c1; end
  end
end

### Resolve calls on directly included class

module Test6
  module M1
    extend T::Helpers
    requires_ancestor { C2 }

    def m1
      c1
    end
  end

  class C1
    extend T::Helpers
    abstract!
    def c1; end
  end

  class C2 < C1
  end
end

### Resolve calls on transitive superclasses

module Test7
  module M1
    extend T::Helpers

    requires_ancestor { C3 }

    def m1
      c3
    end
  end

  module M2
    include M1

    def m2
      m1
      c3
    end
  end

  class C1
    extend T::Helpers
    abstract!
    include M2

    def c1
      m1
      m2
      c3
    end
  end

  class C2 < C1
    extend T::Helpers
    abstract!

    def c2
      m1
      m2
      c3
      c1
    end
  end

  class C3 < C2
    def c3; end
  end
end
