# typed: true

module Root
  class A
    package_private def foo
    end

    package_private_class_method def self.foo
    end
  end

  module MultipleModifiers
    class InstanceMethodDemo
      public    package_private def public_pp; end
      private   package_private def private_pp; end
      protected package_private def protected_pp; end

      package_private public          def pp_public; end
      package_private protected       def pp_protected; end
      package_private package_private def pp_pp; end
      package_private private         def pp_private; end
    end

    x = InstanceMethodDemo.new
    x.public_pp
    x.protected_pp
    x.private_pp
    # ^^^^^^^^^^ error: Non-private call to private method `private_pp` on `Root::MultipleModifiers::InstanceMethodDemo`

    x.pp_public
    x.pp_protected
    x.pp_pp
    x.pp_private
    # ^^^^^^^^^^ error: Non-private call to private method `pp_private` on `Root::MultipleModifiers::InstanceMethodDemo`

    class ClassMethodDemo
      # Class method modifiers cannot be nested the same way, because they return `T.self_type` instead of the method name.

      # public_class_method package_private_class_method  def self.public_pp; e
      # private_class_method package_private_class_method def self.private_pp; e

      # package_private_class_method public_class_method          def self.pp_private; e
      # package_private_class_method package_private_class_method def self.pp_protected; e
      # package_private_class_method private_class_method         def self.pp_public; e
    end
  end
end
