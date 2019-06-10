# typed: true

module Opus
  class DupChild < Parent; end
  class Child < Parent; end
end

class Opus::Parent; end

module Opus::Mixin; end

class Opus::Mixed
  include Opus::Mixin
end

module Opus::MixedModule
  include Opus::Mixin
end

class OtherChild < NonexistentParent; end

class Opus::DupChild < Opus::Parent; end # Only appears once in output