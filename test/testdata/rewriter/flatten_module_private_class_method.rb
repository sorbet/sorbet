# typed: true

module A
  def self.thing
    private_class_method def self.bar
    end
  end
end

class B
  def self.thing
    private_class_method def self.bar
    end
  end
end
