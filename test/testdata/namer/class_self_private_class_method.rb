# typed: true

class A
  class << self
    private_class_method def self.example; end
  end
end
