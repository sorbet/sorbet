# typed: true

[1,2,3].each do |x = x + 1|
              #      ^      parser-error: circular argument reference x
              # ^           parser-error: unmatched "|"
              #           ^ parser-error: missing arg to "|" operator
              #        ^    error: Method `+` does not exist
end
