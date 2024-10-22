# typed: false

# 1
$1

# Numbered references that are too large result in this value being 0.
$4294967296

# In context
if "foobar" =~ /foo(.*)/ then
  puts "The matching word was #{$1}"
end
