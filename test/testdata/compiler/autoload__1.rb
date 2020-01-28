# frozen_string_literal: true
# typed: true
# compiled: true

dir = File.expand_path(__dir__.to_s)
autoload(:A, dir + '/autoload__2')
puts Object.const_get('A')
