# typed: false
# disable-parser-comparison: true

# Missing constant path
module
end

# Invalid constant path without body
module
  def foo; end # Parsed as constant path
end

# Invalid constant path with body
module
  def foo; end # Parsed as constant path
  def bar; end # Parsed as body
end
