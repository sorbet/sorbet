# frozen_string_literal: true
# typed: true
# compiled: true

class NormalClass
end

p NormalClass.class
p NormalClass.new.class

class OverrideClass
  def class
    "SURPRISE"
  end

  def self.class
    "UNEXPECTED"
  end
end

p OverrideClass.class
p OverrideClass.new.class
