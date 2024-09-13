# typed: false

# The `it` keyword was introduced in Ruby 3.4, which isn't supported by Sorbet yet.
# https://bugs.ruby-lang.org/issues/18980
#
# For now, we'll just treat it like a local variable read or method call.

Proc.new do
  it # Prior to Ruby 3.4, this would just be a regular method call
end

Proc.new do
  it = 123 # Prior to Ruby 3.4, this would just be a local variable write
  it       # ... and this is a local variable read.
end
