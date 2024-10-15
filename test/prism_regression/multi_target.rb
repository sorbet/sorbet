# typed: false

p1, (x2, y2) = [[1, 2], [3, 4]]

Proc.new do |p1, (x2, y2)|
end

for p1, (x2, y2) in a
end

p2, (head, *) = [[1, 2], [3, 4]]
p3, (head, *tail) = [[1, 2], [3, 4]]
