# typed: true

module Root
  class A
    package_private def foo
    end

    package_private_class_method def self.foo
    end
  end
end
