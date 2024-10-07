# typed: false

`ruby -e 'puts "Hello, world!"'`

%x(ruby -e 'puts "Hello, parentheses!"')
%x{ruby -e 'puts "Hello, braces!"'}
%x[ruby -e 'puts "Hello, square brackets!"']
%x/ruby -e 'puts "Hello, slashes!"'/
