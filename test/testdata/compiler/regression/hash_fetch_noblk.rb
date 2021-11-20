# frozen_string_literal: true
# compiled: true
# typed: true

# This checks that the Hash#fetch intrinsic isn't inheriting the block from the
# context of the call_fetch method that it's called from. The failure mode here
# is that Hash#fetch picks up that there was a block passed in, and uses it as
# its own.

module M
  def self.call_fetch(&blk)
    {}.fetch(:x)
  end
end

begin
  puts M.call_fetch { puts "block called" }
rescue KeyError
  puts "got KeyError so block wasn't called"
end
