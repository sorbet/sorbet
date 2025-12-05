# typed: true

# Outside the block, calling 'it' invokes the method
def it
  "method_value"
end

x = it # x is "method_value"
