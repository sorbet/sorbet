# typed: true
# Test `defines_behavior` logic

class A # no
end

class B < A # yes
end

module Mod # no
end

module HasConst # no
  A = 1
end

class C # yes
  include Mod
end

class D # yes
  extend Mod
end

class HasDef # yes
  def a_method; end
end

class SelfClassDef # yes
  class << self
    def singleton_method
    end
  end
end

class SelfClass # yes
  # Just having `class << self` is considered sufficient to define
  # behavior.
  class << self
  end
end
