# typed: true
# compiled: true
module A
end
puts A

module B
  module A
  end
  puts A
end
