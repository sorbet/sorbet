# typed: true

begin
  raise "hi"
  a = 3 # error: This code is unreachable
ensure
  a + 4 # error: Method `+` does not exist on `NilClass`
end
