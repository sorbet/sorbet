# typed: strict

class A < PackageSpec
  import Root
# ^^^^^^^^^^^ error: Package `Root` includes explicit visibility modifiers

  visible_to 'tests'
end
