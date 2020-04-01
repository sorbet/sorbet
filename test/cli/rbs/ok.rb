# typed: true

bar = Bar.new

bar.foo("foo", 42)
bar.bar1(nil)
bar.bar1("bar")
bar.bar2("bar", 42)
bar.bar2("bar")

bar.bar3 do |i|
  i.to_s
end

baz = Baz[Integer].new
baz.foo("foo", 42)
baz.bar1(nil)
baz.baz(42, "baz")
