# typed: true

[1,2,3].each do |x|
  foo(&bar) do puts(x) end
#     ^^^^  parser-error: both block argument and literal block are passed
#      ^^^  error: Method `bar` does not exist
# ^^^       error: Method `foo` does not exist
end
