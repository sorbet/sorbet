# typed: strict
# enable-packager: true

class Producer < PackageSpec
  export Producer::Final
  export Producer::Via::Nested # error: Cannot export through class alias
  export Producer::Via::Value # error: Cannot export through class alias
  export Test::Producer::ToApp::Nested # error: Cannot export through class alias
end
