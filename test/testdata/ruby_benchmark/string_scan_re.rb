# frozen_string_literal: true
# typed: strong
# compiled: true
str = Array.new(1_000, 'abc').join(',')
1_000.times { str.scan(/abc/) }
