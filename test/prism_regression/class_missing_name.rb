# typed: false
# disable-parser-comparison: true

# Missing constant path
class
end

# Invalid constant path without body
class
  def foo; end # Parsed as constant path
end

# Invalid constant path with body
class
  def foo; end # Parsed as constant path
  def bar; end # Parsed as body
end
