# typed: strict

begin
  raise "hi"
  a = 3
ensure
  a + 4 # error: Method `+` does not exist on `NilClass`
end
