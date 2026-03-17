# typed: true
# disable-parser-comparison: true

lambda do |;| end
#           ^ error: expected a local variable name in the block parameters

lambda { |;| }
#          ^ error: expected a local variable name in the block parameters

-> (;) {}
#    ^ error-with-dupes: expected a local variable name in the block parameters
