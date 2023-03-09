# typed: strict

ARGF.to_s
ARGF.inspect
ARGF.argv

ARGF.each_line
ARGF.each_line(sep=$/) { |line| nil }
ARGF.each_line(sep=$/, 1) { |line| nil }
ARGF.each_line do |line|
  puts ARGF.filename if ARGF.lineno == 1
  puts "#{ARGF.lineno}: #{line}"
end

ARGF.fileno
ARGF.to_i
ARGF.to_io
ARGF.to_write_io
ARGF.each
ARGF.each_byte
ARGF.each_char
ARGF.each_codepoint
ARGF.read
ARGF.readpartial
ARGF.read_nonblock
ARGF.readlines
ARGF.to_a
ARGF.gets
ARGF.readline
ARGF.getc
ARGF.getbyte
ARGF.readchar
ARGF.readbyte
ARGF.tell
ARGF.seek(1)
ARGF.rewind
ARGF.pos
ARGF.pos = 1
ARGF.eof
ARGF.eof?
ARGF.binmode
ARGF.binmode?
ARGF.write('string')
ARGF.print
ARGF.putc "A"
ARGF.putc 65
ARGF.puts
ARGF.printf
ARGF.filename
ARGF.path
ARGF.file
ARGF.skip
ARGF.close
ARGF.closed?
ARGF.lineno
ARGF.lineno = 1
ARGF.inplace_mode
ARGF.inplace_mode = '.bak'
ARGF.external_encoding
ARGF.internal_encoding
ARGF.set_encoding
