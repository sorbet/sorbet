# typed: false

# Testing an ensure clause without a rescue or else clause
begin
  foo
ensure
  bar
end

# Testing an ensure clause in a method definition
def method_with_ensure
  method_1
  method_2
ensure
  ensured_method
end

# Testing an ensure clause in an empty method definition
def empty_method_with_ensure
ensure
  ensured_method
end

# Testing an ensure clause in a begin block in a method definition
def method_with_begin_and_ensure
  begin
    method_1
    method_2
  ensure
    ensured_method_in_begin
  end
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
