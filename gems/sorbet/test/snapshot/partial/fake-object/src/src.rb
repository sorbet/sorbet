def bad_object
  Module.new do
    def self.name
      'Object'
    end
  end
end

class A
  include bad_object
end
