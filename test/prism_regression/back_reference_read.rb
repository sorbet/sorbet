# typed: false

# The result of the last match
$&

# The string preceding the match in the last match
$`

# The string following the match in the last match
$'

# The results of the highest-numbered group matched
$+

# In context
if "foobar" =~ /foo(.*)/ then
  puts "The last matching word was #{$+}"
end
