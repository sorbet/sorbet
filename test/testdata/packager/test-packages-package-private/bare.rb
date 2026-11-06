# typed: true

module Root
  class Bare
    package_private
  # ^^^^^^^^^^^^^^^ error: `package_private` with no arguments is not allowed

    def after_bare_package_private; end

    def self.shadowed; end

    package_private_class_method
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `package_private_class_method` with no arguments is not allowed

    def shadowed; end
    def self.after_bare_package_private_class_method; end

    def self.calls_package_private_in_method_body
      package_private
    end
  end

  class BarePrivate
    private

    def before_bare_package_private_class_method; end

    package_private_class_method
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `package_private_class_method` with no arguments is not allowed

    def after_bare_package_private_class_method; end
  end
end
