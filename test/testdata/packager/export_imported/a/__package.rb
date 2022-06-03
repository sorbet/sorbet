# frozen_string_literal: true
# typed: strict
# enable-packager: true

# Edge case: We technically allow packages to export their imports. It's easier to implement packages this way
# (although this practice should be discouraged!)

class A < PackageSpec
  import B

  export B::BClass
# ^^^^^^^^^^^^^^^^ error: Cannot export `B::BClass` because it is owned by another package
end
