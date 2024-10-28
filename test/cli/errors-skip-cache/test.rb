# typed: true

def foo(*)
  [1,2,3].each do |*|
  end
end
