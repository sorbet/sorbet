# frozen_string_literal: true
# typed: true
# compiled: true

puts caller_locations(0,1)&.fetch(0)&.absolute_path()
