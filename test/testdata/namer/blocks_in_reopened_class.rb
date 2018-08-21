# typed: true

# The two blocks below need to end up with distinct symbols
class A
  B = [].map { |x| x }
end

class A
  C = [].map { |x, y| x }
end
