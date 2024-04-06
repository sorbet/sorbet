# typed: strict

class Root::Nested < PackageSpec # error: Package `Root::Nested` is missing imports
  # does not import Root
end
