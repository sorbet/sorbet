# typed: strict

# Migrate Producer to explicit test packages, so its specification runs once.
class Producer::Test < PackageSpec
  test!
  import Producer
end
