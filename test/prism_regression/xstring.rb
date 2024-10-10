# typed: false

`ruby -e 'puts "Hello, world!"'`

%x(ruby -e 'puts "Hello, parentheses!"')
%x{ruby -e 'puts "Hello, braces!"'}
%x[ruby -e 'puts "Hello, square brackets!"']
%x/ruby -e 'puts "Hello, slashes!"'/

`ruby -e 'puts "Hello, #{}"'`
`ruby -e 'puts "Hello, #{'interpolation'}"'`
`ruby -e 'puts "Hello, #{foo}"'`
`ruby -e 'puts "Hello, #{`echo "backticks"`}"'`
