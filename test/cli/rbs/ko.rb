# typed: true

bar = Bar.new
bar.foo(42)
bar.bar1(nil, nil)
bar.bar1(42, "string")
bar.bar2(42, 42)
bar.bar3 do |i|
  i
end

baz = Baz[String].new
baz.baz(42, "baz")
