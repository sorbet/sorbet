# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def takes_block(arg0, &blk)
    puts arg0
    yield
  end
end

class B < A
  def takes_block(arg0, &blk)
    super do
      puts "inside A"
    end
    yield
  end
end

blk = -> {
  puts 423
}
A.new.takes_block(147, &blk)
B.new.takes_block(147, &blk)
