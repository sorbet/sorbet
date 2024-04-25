# frozen_string_literal: true
# typed: true
# compiled: false

require_relative './optional__2'

3.times do
  p Flag.active?("interpreted call")
  p Flag.active?("interpreted call2", {:attr => 1})
end
