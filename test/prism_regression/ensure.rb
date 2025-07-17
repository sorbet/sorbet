# typed: false

# Testing an ensure clause without a rescue or else clause
begin
  foo
ensure
  bar
end

# Testing an ensure clause in a method definition
def method_with_ensure
  "string1"
  "string2"
ensure
  "ensured"
end

# Testing an ensure clause in an empty method definition
def empty_method_with_ensure
ensure
  "ensured"
end

# Testing an ensure clause in a begin block in a method definition
def method_with_begin_and_ensure
  begin
    "string1"
    "string2"
  ensure
    "ensured"
  end
end

# Testing an ensure clause with multiple methods
begin
  "string1"
  "string2"
ensure
  "ensured1"
  "ensured2"
end

# Testing a rescue clause with an else and ensure clause
begin
  "string1"
  "string2"
rescue
  "rescued rescue"
else
  "rescued else"
ensure
  "ensure"
end
