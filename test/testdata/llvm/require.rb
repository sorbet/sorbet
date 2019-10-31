require 'tempfile'
args = T.unsafe(Array.new)
args << 'foo'
args << '.rb'
file = Tempfile.new(args)
file.write("puts 'hello world'")
file.close
path = T.unsafe(file.path)
puts File.read(path)
require path
