# typed: true
# disable-parser-comparison: true

lambda do |;| end
#         ^ error: unmatched "|" in block argument list
#           ^ error: unexpected token "|"

lambda { |;| }
#        ^ error: unmatched "|" in block argument list
#          ^ error: unexpected token "|"

-> (;) {}
#    ^ error-with-dupes: unexpected token ")"
