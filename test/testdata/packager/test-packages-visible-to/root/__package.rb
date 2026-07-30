# typed: strict
# enable-packager: true

class Root < PackageSpec
  export Root::A

  visible_to B
end
