# typed: true
# enable-experimental-requires-ancestor: true

module HelperModule
  extend T::Helpers

  requires_ancestor { Module }

  def helper
    class_eval "helper"
  end
end

module HelperModule2
  extend T::Helpers

  requires_ancestor { ::Module }

  def helper
    class_eval "helper"
  end
end

module HelperTModule
  extend T::Helpers

  requires_ancestor { T::Module }

  def helper
    class_eval "helper" # error: Method `class_eval` does not exist on `HelperTModule`
  end
end

module HelperTModuleUntyped
  extend T::Helpers

  requires_ancestor { T::Module[T.untyped] }
                   #  ^^^^^^^^^^^^^^^^^^^^ error: `HelperTModuleUntyped` can't require generic ancestor (unsupported)

  def helper
    class_eval "helper" # error: Method `class_eval` does not exist on `HelperTModuleUntyped`
  end
end

module HelperTModuleString
  extend T::Helpers

  requires_ancestor { T::Module[String] }
                   #  ^^^^^^^^^^^^^^^^^ error: `HelperTModuleString` can't require generic ancestor (unsupported)

  def helper
    class_eval "helper" # error: Method `class_eval` does not exist on `HelperTModuleString`
  end
end

module HelperNoRequiredAncestor
  def helper
    class_eval "helper" # error: Method `class_eval` does not exist on `HelperNoRequiredAncestor`
  end
end
