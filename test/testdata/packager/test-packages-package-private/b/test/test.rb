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
