# frozen_string_literal: true
# typed: true
# compiled: true

# Tests different interactions between the rescue/else/ensure parts of exception
# handling.

class A
  # Wrapper to catch exceptions that escape
  def self.test(&blk)
    begin
      blk.call
    rescue => e
      puts "Caught #{e.message}"
    end
  end
end

# No exception raised, the else block runs
A.test do
  begin
    puts "body"
  rescue
  else
    puts "else"
  end
end

# No exception raised, the ensure block runs
A.test do
  begin
    puts "body"
  ensure
    puts "ensure"
  end
end

# Exception raised, the ensure block runs
A.test do
  begin
    raise "foo"
  rescue => e
    puts "Caught: #{e.message}"
  ensure
    puts "ensure"
  end
end

class E < StandardError; end

# Exception conditionally raised that isn't caught by any rescue block
[true,false].each do |b|
  A.test do
    begin
      raise "foo" if b
    rescue E => e
      puts "Caught: #{e.message}"
    else
      puts "else"
    ensure
      puts "ensure"
    end
  end
end
