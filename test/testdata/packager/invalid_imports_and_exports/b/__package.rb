# typed: strict

# Despite A having invalid imports, B should still typecheck fine.

class B < PackageSpec
  import A
  export BClass
  export_methods BModule
end
