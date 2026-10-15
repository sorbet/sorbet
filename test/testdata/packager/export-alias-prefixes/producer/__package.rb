# typed: strict
# enable-packager: true

class Producer < PackageSpec
  export Producer::Final
  export Producer::Via::Nested
  export Producer::Via::Value
  export Test::Producer::ToApp::Nested
end
