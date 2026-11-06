# typed: true

module Root::B::Test
  class BTest
    def test_b
      Root::A.new.foo
    # ^^^^^^^^^^^^^^^ error: Method `foo` on `Root::A` is package-private
      Root::A.foo
    # ^^^^^^^^^^^ error: Method `foo` on `T.class_of(Root::A)` is package-private
    end
  end

  class BareTest
    def test_bare
      Root::Bare.new.after_bare_package_private
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `after_bare_package_private` on `Root::Bare` is package-private and cannot be called from package `Root::B::Test`
      Root::Bare.new.shadowed
      Root::Bare.shadowed
    # ^^^^^^^^^^^^^^^^^^^ error: Method `shadowed` on `T.class_of(Root::Bare)` is package-private and cannot be called from package `Root::B::Test`
      Root::Bare.after_bare_package_private_class_method
    end

    def test_bare_private
      Root::BarePrivate.new.before_bare_package_private_class_method
    #                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Non-private call to private method `before_bare_package_private_class_method` on `Root::BarePrivate`
      Root::BarePrivate.new.after_bare_package_private_class_method
    end
  end

  class Child < Root::ModifierParent
    def test_inherited_package_private
      package_private_private
    # ^^^^^^^^^^^^^^^^^^^^^^^ error: Method `package_private_private` on `Root::ModifierParent` is package-private and cannot be called from package `Root::B::Test`
      private_package_private
    # ^^^^^^^^^^^^^^^^^^^^^^^ error: Method `private_package_private` on `Root::ModifierParent` is package-private and cannot be called from package `Root::B::Test`
      package_private_protected
    # ^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `package_private_protected` on `Root::ModifierParent` is package-private and cannot be called from package `Root::B::Test`
      protected_package_private
    # ^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `protected_package_private` on `Root::ModifierParent` is package-private and cannot be called from package `Root::B::Test`
      package_private_public
    # ^^^^^^^^^^^^^^^^^^^^^^ error: Method `package_private_public` on `Root::ModifierParent` is package-private and cannot be called from package `Root::B::Test`
      public_package_private
    # ^^^^^^^^^^^^^^^^^^^^^^ error: Method `public_package_private` on `Root::ModifierParent` is package-private and cannot be called from package `Root::B::Test`
    end
  end
end
