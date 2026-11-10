# typed: strict

# `package_private!` calls define behavior on `Root::A` and `Root::A::Nested`, which packages are
# nested under, so `Root::A` has to opt in.
class Root::A < PackageSpec
  imported_by_nested_packages!
end
