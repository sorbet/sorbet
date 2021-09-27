# compiled: true
# typed: true
# frozen_string_literal: true

def test_case(name, p)
  puts "#{name} arity: #{p.arity}"
end

# empty args
test_case("empty", lambda {})

# block arg
test_case("block", lambda {|&blk|})

# positional args
test_case("1 required", lambda {|a|})
test_case("2 required", lambda {|a,b|})

# defaulted args
test_case("1 defaulted", lambda {|a=0|})
test_case("2 defaulted", lambda {|a=0,b=nil|})

# positional and defaulted args
test_case("1 required, 1 defaulted", lambda {|a, b=nil|})
test_case("1 required, 2 defaulted", lambda {|a, b=nil, c=nil|})
test_case("2 required, 2 defaulted", lambda {|a, b, c=nil, d=nil|})
test_case("2 required, 2 defaulted, break", lambda {|a, b, c=nil, d=nil| break})

# keywords
test_case("1 required", lambda {|a:|})
test_case("2 required", lambda {|a:, b:|})
test_case("1 optional", lambda {|a: 10|})
test_case("2 optional", lambda {|a: 10, b: 20|})
test_case("1 required, 1 optional", lambda {|a:, b: 20|})
test_case("2 required, 1 optional", lambda {|a:, b:, c: 30|})

# repeated args
test_case("repeated", lambda {|*a|})
test_case("1 required, repeated", lambda {|a, *b|})
test_case("2 required, repeated", lambda {|a, b, *c|})
test_case("1 required, 1 defaulted, repeated", lambda {|a, b=10, *c|})
test_case("2 required, 1 defaulted, repeated", lambda {|a, b, c=10, *d|})
test_case("2 required, 2 defaulted, repeated", lambda {|a, b, c=10, d=nil, *e|})
test_case("2 required, 2 defaulted, repeated, break", lambda {|a, b, c=10, d=nil, *e| break})
