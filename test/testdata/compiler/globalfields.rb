# typed: true
# compiled: true

class A
  def write(v)
    $f = v
  end
  def read
    $f
  end
end

a = A.new
puts a.read
a.write("value")
puts a.read
puts $f
F = 1
