# frozen_string_literal: true
# typed: true
# compiled: true
# chmod file
require 'tempfile'
max = 200_000
tmp = Tempfile.new('chmod')
path = tmp.path
max.times do
  File.chmod(0777, path)
end
tmp.close!
