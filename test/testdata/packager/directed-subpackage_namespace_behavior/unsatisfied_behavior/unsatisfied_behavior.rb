# typed: true

module UnsatisfiedBehavior # error: `UnsatisfiedBehavior` can only define behavior if every package nested under `UnsatisfiedBehavior` imports it
  def self.helper; end
end

UnsatisfiedBehavior.helper # error: Method `helper` does not exist on `T.class_of(UnsatisfiedBehavior)`
