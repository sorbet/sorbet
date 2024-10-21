# typed: false

# Testing an ensure clause without a rescue or else clause
begin
  foo
ensure
  bar
end

# Testing an ensure clause with multiple methods
begin
  method_1
  method_2
ensure
  ensured_method_1
  ensured_method_2
end

# Testing a rescue clause with an else and ensure clause
begin
  method_for_rescue_and_ensure
rescue
  "rescued rescue"
else
  "rescued else"
ensure
  "ensure"
end
