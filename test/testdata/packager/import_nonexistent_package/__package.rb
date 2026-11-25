# typed: strict
# enable-packager: true

class Root < PackageSpec
  import NonExistent
  #      ^^^^^^^^^^^ error: Unable to resolve constant `NonExistent`
  import Root::B
end
