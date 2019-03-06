class SorbetRBIGeneration::RequireEverything
  def self.require_everything
  end
end

def at_exit(*args, &block)
  puts "Ignoring at_exit: #{args} #{block}"
end
