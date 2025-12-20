# frozen_string_literal: true
# typed: true
# compiled: true
require 'tmpdir'
max = 100_000
Dir.mktmpdir('bm_dir_empty_p') do |dir|
  max.times { Dir.empty?(dir) }
end
