# typed: true

[1,2,3].each do |x = x + 1|
              #      ^      error: circular argument reference x
              # ^           error: unmatched "|"
              #           ^ error: missing arg to "|" operator
              #        ^    error: Method `+` does not exist
end
