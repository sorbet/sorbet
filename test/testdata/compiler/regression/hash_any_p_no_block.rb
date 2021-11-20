# frozen_string_literal: true
# compiled: true
# typed: true

# This checks that the Hash#any? intrinsic isn't inheriting the block from the
# context of the call_any_p method that it's called from. The failure mode here
# is that Hash#any? picks up that there was a block passed in, and uses it as
# its own.

module M
  def self.call_any_p(&blk)
    {x: 10}.any?
  end
end

puts M.call_any_p {puts "block called"}
