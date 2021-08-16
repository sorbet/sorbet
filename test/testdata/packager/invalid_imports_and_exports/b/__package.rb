# frozen_string_literal: true
# typed: strict

# Despite A having invalid imports, B should still typecheck fine.

class B < PackageSpec
  import A
  export B::BClass
  export B::BModule
end
