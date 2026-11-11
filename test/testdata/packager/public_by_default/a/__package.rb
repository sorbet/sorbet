# typed: strict
# enable-packager: true

class Root::A < PackageSpec
  # An export of a constant that doesn't exist, and a duplicate export.
  export Root::A::Exported
  export Root::A::DoesNotExist
  export Root::A::Exported
end
