# typed: true
# enable-experimental-requires-ancestor: false

### Does not suggest helper module if disabled

module Test1
  module M1
    requires_ancestor { M2 } # error: Method `requires_ancestor` does not exist on `T.class_of(Test1::M1)`
  end

  module M2; end
end

### Does not resolve methods if disabled

module Test2
  module M1
    extend T::Helpers
    requires_ancestor { M2 }

    def m1
      m2 # error: Method `m2` does not exist on `Test2::M1`
    end
  end

  module M2
    def m2; end
  end
end

### Does not require ancestor if disabled

module Test3
  module M1
    extend T::Helpers
    requires_ancestor { M2 }
  end

  module M2; end

  module M3
    include M1
  end
end
