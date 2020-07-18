# typed: strict
# enable-packager: true

# Edge case: We technically allow packages to export their imports. It's easier to implement packages this way
# (although this practice should be discouraged!)

class A < PackageSpec
  import B

  export B::BClass
end
