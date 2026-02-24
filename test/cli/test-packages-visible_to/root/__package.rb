# typed: strict

class Root < PackageSpec
  export Root::A

  visible_to B
end
