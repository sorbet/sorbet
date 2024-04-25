# frozen_string_literal: true
# typed: true
# compiled: true

begin
  p 'running begin'
  raise KeyError, 'wrong key'
rescue IndexError
  p "found it!"
end
