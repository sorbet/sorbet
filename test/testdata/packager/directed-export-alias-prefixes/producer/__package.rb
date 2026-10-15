# typed: strict
# enable-packager: true
# enable-package-directed: true

class Producer < PackageSpec
  export Producer::Final
  export Producer::Via::Nested # error: Cannot export through class alias
  export Producer::Via::Value # error: Cannot export through class alias
end
