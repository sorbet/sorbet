# frozen_string_literal: true
# compiled: true
# typed: true

module M
  def self.call_any_p(&blk)
    {x: 10}.any?
  end
end

puts M.call_any_p {puts "block called"}
