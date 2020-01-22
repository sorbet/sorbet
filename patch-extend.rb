# typed: true

class Module
  def extend(other)
      Kernel.puts "┌─── self.extend(other) ─────"
      Kernel.puts "│ #{self}.extend(#{other})"
      Kernel.puts "│"
      Kernel.puts caller.map {|line| "│ #{line}"}
      Kernel.puts "└────────────────────────────"
      super
  end
end
