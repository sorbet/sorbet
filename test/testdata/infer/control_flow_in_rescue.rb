# typed: strict

begin
  foo = 1
rescue
  foo + 2 if foo
end
