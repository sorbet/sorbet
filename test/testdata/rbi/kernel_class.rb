# typed: true

module A
  include Kernel
  def foo
    self.class
  end
end
