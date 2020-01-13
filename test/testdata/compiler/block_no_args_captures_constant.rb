# typed: true
# compiled: true
A = "hi"
def foo
  Kernel.puts A
  10.times do
    Kernel.puts A
  end
end

foo
