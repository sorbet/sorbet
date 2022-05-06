# typed: true
extend T::Sig

def it(name, &blk); end
def test_each(val, &blk); end
def foo(x); end

[1, 2].each do |x|
  it "asdf #{x}" do
    z = foo(x)
  end
end

[[1, 2], [3, 4]].each do |(x, y)|
  it "asdf #{x} #{y}" do
    z = foo(x) + foo(y)
  end
end

{1 => 2, 3 => 4}.each do |x, y|
  it "asdf #{x} #{y} hash" do
  end
end

{1 => 2, 3 => 4}.each do |a|
  it "asdf #{a} hash" do
  end
end

test_each([1,2]) do |x|
  it "asdf #{x} test_each" do
    z = foo(x)
  end
end
