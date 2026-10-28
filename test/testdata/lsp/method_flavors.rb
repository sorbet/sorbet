# typed: true
class Foo
  def instance_method; end
  def self.dotted_class_method; end
  class << self
    def nested_class_method; end
  end
end
