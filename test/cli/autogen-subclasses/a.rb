# typed: true

module Opus
  class DupChild < Parent; end
  class Child < Parent; end
  class IgnoredChild < IgnoredParent; end
end

class Opus::Parent; end
class Opus::IgnoredParent; end # Subclassed, but not included in --autogen-subclasses-parent
class Opus::NeverSubclassed; end # Included in --autogen-subclasses-parent, but never subclassed

module Opus::Mixin; end

class Opus::Mixed
  include Opus::Mixin
end

module Opus::MixedModule
  include Opus::Mixin
end

class Opus::MixedDescendant # Mixes in Opus::Mixed via Opus::MixedModule
  include Opus::MixedModule
end

class OtherChild < NonexistentParent; end

class Opus::DupChild < Opus::Parent; end # Only appears once in output

class Opus::Risk::Model::Mixins::RiskSafeMachine; end

class FooSafeMachine < Opus::Risk::Model::Mixins::RiskSafeMachine; end

module MyMixin; end
class MyClass
  X = Y
  include MyMixin
end
