# typed: true

# Multiple visibility modifiers apply at once. The left-most one (which is called last) wins.

class Demo
  # Standard Ruby visibility modifiers.
  # Interactions with package_private are tested separately in `test/testdata/packager/test-packages-package-private/a.rb`
  public public       def public_private; end
  public protected    def public_protected; end
  public private      def public_private; end

  protected public    def protected_public; end
  protected protected def protected_protected; end
  protected private   def protected_private; end

  private public      def private_public; end
  private protected   def private_protected; end
  private private     def private_private; end
end

x = Demo.new

x.public_private
x.public_protected
x.public_private

x.protected_public
x.protected_protected
x.protected_private
# ^^^^^^^^^^^^^^^^^ error: Non-private call to private method `protected_private` on `Demo`
# TODO: `protected` is not properly clearing the `isPrivate` flag https://github.com/sorbet/sorbet/issues/10089

x.private_public
# ^^^^^^^^^^^^^^ error: Non-private call to private method `private_public` on `Demo`
x.private_protected
# ^^^^^^^^^^^^^^^^^ error: Non-private call to private method `private_protected` on `Demo`
x.private_private
# ^^^^^^^^^^^^^^^ error: Non-private call to private method `private_private` on `Demo`

class ClassMethodDemo
  # Class method modifiers cannot be nested the same way, because they return `T.self_type` instead of the method name.

  # public_class_method public_class_method   def self.public_public; end
  # public_class_method private_class_method  def self.public_private; end

  # private_class_method public_class_method  def self.private_public; end
  # private_class_method private_class_method def self.private_private; end
end
