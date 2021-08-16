# frozen_string_literal: true
# typed: strict

class B < PackageSpec
  import A

  export B::BClass
end
