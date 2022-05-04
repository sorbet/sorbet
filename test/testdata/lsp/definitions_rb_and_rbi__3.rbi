# typed: true

# The usage annotation here is kind of a hack: we want the only
# definition for go-to-definition to exist in the rb file defining
# MyClass, but the test harness will see this class if we don't
# say that it's a "use", even though semantically it is a def.
class MyClass
    # ^^^^^^^ usage: MyClass
  def magic_dynamically_defined_fun; end
end

class MyModule
    # ^^^^^^^^ usage: MyModule
  def magic_dynamically_defined_fun; end
end
