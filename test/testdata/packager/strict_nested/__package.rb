# typed: strict
# enable-packager: true

class Root < PackageSpec
  import Root::Nested

  export Root::MyClass
end
