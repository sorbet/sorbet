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

# TODO(jez) I don't know why we are seeing zsuper methods added to the symbol table in this file. But also, I kind of don't care enough to fix them.
