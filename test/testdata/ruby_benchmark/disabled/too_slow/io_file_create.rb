# frozen_string_literal: true
# typed: true
# compiled: true
#
# Create files
#

max = 200_000
file = './tmpfile_of_bm_io_file_create'

max.times{
  f = open(file, 'w')
  f.close#(true)
}
File.unlink(file)

