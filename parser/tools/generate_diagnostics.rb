#!/usr/bin/env ruby

diagnostics_rb, out_cpp = ARGV
load(diagnostics_rb)

File.open(out_cpp, 'w') do |fh|
  fh.puts "namespace ruby_typer {"
  fh.puts "namespace parser {"
  fh.puts "const char * dclass_strings[] = {"
  fh.puts MESSAGES.map { |_, msg| "\t#{msg.inspect}" }.join(",\n")
  fh.puts "};"
  fh.puts "}"
  fh.puts "}"
end
