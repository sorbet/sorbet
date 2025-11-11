# typed: false


# #!/opt/rubies/3.4.2/bin/ruby
#
# e = Enumerator::Product.new(
#   [:skip, [], ["else line 1", "else line 2"]],
#   [:skip, [], ["ensure line 1"], ["ensure line 1", "ensure line 2"]],
#   [false],
#   [:skip, [], ["rescue line 1"], ["rescue line 1", "rescue line 2"]],
#   [:skip, [], ["begin line 1"], ["begin line 1", "begin line 2"]],
# )
#
# e.each do |else_lines, ensure_lines, rescue_clause, rescue_lines, begin_lines |
#   s = "begin\n"
#
#   if begin_lines != :skip
#     s += begin_lines.map { "  #{_1.inspect}\n" }.join
#   end
#
#   if rescue_lines != :skip
#     s += rescue_clause ? "rescue StandardError => e\n" : "rescue\n"
#     s += rescue_lines.map { "  #{_1.inspect}\n" }.join
#   end
#
#   if ensure_lines != :skip
#     s += "ensure\n"
#     s += ensure_lines.map { "  #{_1.inspect}\n" }.join
#   end
#
#   # Can't have `else` unless there was also a `rescue` first, and no `ensure`
#   if else_lines != :skip && rescue_lines != :skip && ensure_lines == :skip
#     s += "else\n"
#     s += else_lines.map { "  #{_1.inspect}\n" }.join
#   end
#
#   s += "end"
#
#   puts s
#   puts
# end

begin
end

begin
end

begin
  "begin line 1"
end

begin
  "begin line 1"
  "begin line 2"
end

begin
rescue
end

begin
rescue
end

begin
  "begin line 1"
rescue
end

begin
  "begin line 1"
  "begin line 2"
rescue
end

begin
rescue
  "rescue line 1"
end

begin
rescue
  "rescue line 1"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
end

begin
ensure
end

begin
ensure
end

begin
  "begin line 1"
ensure
end

begin
  "begin line 1"
  "begin line 2"
ensure
end

begin
rescue
ensure
end

begin
rescue
ensure
end

begin
  "begin line 1"
rescue
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
end

begin
rescue
  "rescue line 1"
ensure
end

begin
rescue
  "rescue line 1"
ensure
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
ensure
  "ensure line 1"
end

begin
ensure
  "ensure line 1"
end

begin
  "begin line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
ensure
  "ensure line 1"
end

begin
rescue
ensure
  "ensure line 1"
end

begin
rescue
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
end

begin
end

begin
  "begin line 1"
end

begin
  "begin line 1"
  "begin line 2"
end

begin
rescue
else
end

begin
rescue
else
end

begin
  "begin line 1"
rescue
else
end

begin
  "begin line 1"
  "begin line 2"
rescue
else
end

begin
rescue
  "rescue line 1"
else
end

begin
rescue
  "rescue line 1"
else
end

begin
  "begin line 1"
rescue
  "rescue line 1"
else
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
else
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
else
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
else
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
else
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
else
end

begin
ensure
end

begin
ensure
end

begin
  "begin line 1"
ensure
end

begin
  "begin line 1"
  "begin line 2"
ensure
end

begin
rescue
ensure
end

begin
rescue
ensure
end

begin
  "begin line 1"
rescue
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
end

begin
rescue
  "rescue line 1"
ensure
end

begin
rescue
  "rescue line 1"
ensure
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
ensure
  "ensure line 1"
end

begin
ensure
  "ensure line 1"
end

begin
  "begin line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
ensure
  "ensure line 1"
end

begin
rescue
ensure
  "ensure line 1"
end

begin
rescue
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
end

begin
end

begin
  "begin line 1"
end

begin
  "begin line 1"
  "begin line 2"
end

begin
rescue
else
  "else line 1"
  "else line 2"
end

begin
rescue
else
  "else line 1"
  "else line 2"
end

begin
  "begin line 1"
rescue
else
  "else line 1"
  "else line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
else
  "else line 1"
  "else line 2"
end

begin
rescue
  "rescue line 1"
else
  "else line 1"
  "else line 2"
end

begin
rescue
  "rescue line 1"
else
  "else line 1"
  "else line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
else
  "else line 1"
  "else line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
else
  "else line 1"
  "else line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
else
  "else line 1"
  "else line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
else
  "else line 1"
  "else line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
else
  "else line 1"
  "else line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
else
  "else line 1"
  "else line 2"
end

begin
ensure
end

begin
ensure
end

begin
  "begin line 1"
ensure
end

begin
  "begin line 1"
  "begin line 2"
ensure
end

begin
rescue
ensure
end

begin
rescue
ensure
end

begin
  "begin line 1"
rescue
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
end

begin
rescue
  "rescue line 1"
ensure
end

begin
rescue
  "rescue line 1"
ensure
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
end

begin
ensure
  "ensure line 1"
end

begin
ensure
  "ensure line 1"
end

begin
  "begin line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
ensure
  "ensure line 1"
end

begin
rescue
ensure
  "ensure line 1"
end

begin
rescue
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
end

begin
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end

begin
  "begin line 1"
  "begin line 2"
rescue
  "rescue line 1"
  "rescue line 2"
ensure
  "ensure line 1"
  "ensure line 2"
end
