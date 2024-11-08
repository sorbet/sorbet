# typed: true

[1,2,3].each do |x|
  x&.y, = 10
 # ^^ error: &. inside multiple assignment
end
