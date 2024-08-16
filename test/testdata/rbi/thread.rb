# typed: strong

t = Thread.new do
  Thread.handle_interrupt(StandardError => :never) do
    puts Thread.pending_interrupt?(StandardError)
  end
end
t.raise StandardError
t.raise StandardError.new
t.raise StandardError, "test"
t.raise "test"
