require 'tempfile'
file = Tempfile.new(['foo', '.rb'])
file.write("puts 'hello world'")
file.close
path = T.unsafe(file.path)
puts File.read(path)
require path
