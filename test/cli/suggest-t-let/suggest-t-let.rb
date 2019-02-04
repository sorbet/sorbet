# typed: strict

x = false
[].each do
  x = true
end

y = nil
[].each do
  y = ''
end
