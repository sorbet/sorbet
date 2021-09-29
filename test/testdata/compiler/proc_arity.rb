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
test_case("pos:      1 required", lambda {|a|})
test_case("pos:      2 required", lambda {|a,b|})

# defaulted args
test_case("pos:      1 defaulted", lambda {|a=0|})
test_case("pos:      2 defaulted", lambda {|a=0,b=nil|})

# positional and defaulted args
test_case("pos:      1 required, 1 defaulted", lambda {|a, b=nil|})
test_case("pos:      1 required, 2 defaulted", lambda {|a, b=nil, c=nil|})
test_case("pos:      2 required, 2 defaulted", lambda {|a, b, c=nil, d=nil|})
test_case("pos:      2 required, 2 defaulted, break", lambda {|a, b, c=nil, d=nil| break})

# keywords
test_case("kwargs:   1 required", lambda {|a:|})
test_case("kwargs:   2 required", lambda {|a:, b:|})
test_case("kwargs:   1 optional", lambda {|a: 10|})
test_case("kwargs:   2 optional", lambda {|a: 10, b: 20|})
test_case("kwargs:   1 required, 1 optional", lambda {|a:, b: 20|})
test_case("kwargs:   2 required, 1 optional", lambda {|a:, b:, c: 30|})
test_case("kwargs:   kwsplat", lambda {|**rest|})
test_case("kwargs:   1 required, kwsplat", lambda {|a:, **rest|})
test_case("kwargs:   1 optional, kwsplat", lambda {|a: 10, **rest|})
test_case("kwargs:   1 required, 1 optional, kwsplat", lambda {|a:, b: 10, **rest|})

# repeated args
test_case("splat:    repeated", lambda {|*a|})
test_case("splat:    1 required, repeated", lambda {|a, *b|})
test_case("splat:    2 required, repeated", lambda {|a, b, *c|})
test_case("splat:    1 required, 1 defaulted, repeated", lambda {|a, b=10, *c|})
test_case("splat:    2 required, 1 defaulted, repeated", lambda {|a, b, c=10, *d|})
test_case("splat:    2 required, 2 defaulted, repeated", lambda {|a, b, c=10, d=nil, *e|})
test_case("splat:    2 required, 2 defaulted, repeated, break", lambda {|a, b, c=10, d=nil, *e| break})

# combined
test_case("combined: 1 pos, 1 kw", lambda {|a, b:|})
test_case("combined: 2 pos, 1 kw", lambda {|a, b, c:|})
test_case("combined: 1 pos, 2 kw", lambda {|a, b:, c:|})
test_case("combined: 2 pos, 2 kw", lambda {|a, b, c:, d:|})
test_case("combined: 2 pos, 1 def, 1 kw", lambda {|a, b, c=10, d:|})
test_case("combined: 2 pos, 2 def, 1 kw", lambda {|a, b, c=10, d=20, e:|})
test_case("combined: 2 pos, 1 kw, 1 def", lambda {|a, b, c:, d: 10|})
test_case("combined: 2 pos, 1 kw, 2 def", lambda {|a, b, c:, d: 10, e: 20|})
test_case("combined: 2 pos, 1 def, 1 kw, 1 def", lambda {|a, b, c=10, d:, e: 20|})
test_case("combined: 2 pos, 1 def, splat, 1 kw, 1 def", lambda {|a, b, c=10, *rest, d:, e: 20|})
test_case("combined: 2 pos, 1 def, splat, 1 kw, 1 def, kwsplat", lambda {|a, b, c=10, *rest, d:, e: 20, **kwrest|})
test_case("combined: 2 pos, kwsplat", lambda {|a, b, **kwrest|})
test_case("combined: 2 kw, splat", lambda {|*a, b:, c:|})
