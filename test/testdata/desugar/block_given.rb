# typed: false

class SomethingElse; end

def method_with_block_param(&the_block_name)
  # Valid cases that we should detect as special calls to `block_given?`
  block_given?
  self.block_given?
  Kernel.block_given?
  ::Kernel.block_given?

  # Due to the current structure of Desugar.cc, we still detect this as a special `block_given?` call.
  block_given? { "with a block?!" }
  #            ^^^^^^^^^^^^^^^^^^^^ error: No body in block

  # Cases we should not detect as special calls to `block_given?`
  Object.block_given? # Normally not allowed: private method `block_given?' called for Object:Class
  SomethingElse.block_given?
  block_given?("with parameter")
end

def method_with_anonymous_block_param(&)
  # Valid cases that we should detect as special calls to `block_given?`
  block_given?
  self.block_given?
  Kernel.block_given?
  ::Kernel.block_given?

  # Due to the current structure of Desugar.cc, we still detect this as a special `block_given?` call.
  block_given? { "with a block?!" }
  #            ^^^^^^^^^^^^^^^^^^^^ error: No body in block

  # Cases we should not detect as special calls to `block_given?`
  Object.block_given? # Normally not allowed: private method `block_given?' called for Object:Class
  SomethingElse.block_given?
  block_given?("with parameter")
end

def method_with_implicit_block_param()
  # Valid cases that we should detect as special calls to `block_given?`
  block_given?
  self.block_given?
  Kernel.block_given?
  ::Kernel.block_given?

  # Due to the current structure of Desugar.cc, we still detect this as a special `block_given?` call.
  block_given? { "with a block?!" }
  #            ^^^^^^^^^^^^^^^^^^^^ error: No body in block

  # Cases we should not detect as special calls to `block_given?`
  Object.block_given? # Normally not allowed: private method `block_given?' called for Object:Class
  SomethingElse.block_given?
  block_given?("with parameter")
end
