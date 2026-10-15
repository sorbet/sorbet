# typed: strict
# enable-packager: true
# enable-package-directed: true

class Producer < PackageSpec
  export Producer::Final
  export Producer::Via::Nested
  export Producer::Via::Value
end
