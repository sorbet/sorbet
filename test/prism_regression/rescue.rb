# typed: false

# Testing a simple rescue clause
begin
  bar
rescue
  "rescued"
end

# Testing a simple rescue clause in a method definition
def method_with_rescue
  123
rescue
  "rescued"
end

# Testing a rescue clause with multiple body statements
begin
  "string1"
  "string2"
rescue
  "rescued"
end

# Testing a rescue clause with a specific exception
begin
  "string1"
rescue RuntimeError
  "rescued Foo"
end

# Testing a rescue clause with multiple exceptions and a variable assignment
begin
  "string1"
rescue RuntimeError, NotImplementedError => e
  "rescued Foo #{e}"
end

# Testing a rescue clause with an else clause
begin
  "string1"
rescue RuntimeError
  "rescued Foo"
else
  "rescued else"
end

# Testing multiple rescue clauses with different exceptions
begin
  "string1"
rescue RuntimeError => e
  "rescued Foo #{e}"
rescue NotImplementedError => e
  "rescued Bar #{e}"
rescue => e
  "rescued #{e}"
end

# Testing multiple rescue clauses with different exceptions and a final else clause
begin
  "string1"
rescue RuntimeError => e
  "rescued Foo #{e}"
rescue NotImplementedError => e
  "rescued Bar #{e}"
rescue => e
  "rescued #{e}"
else
  "rescued else"
end

# Testing a rescue clause with an else and ensure clause
begin
  "string1"
rescue
  "rescued rescue"
else
  "rescued else"
ensure
  "ensure"
end

# Testing rescue modifiers
problematic_code rescue puts "rescued"
problematic_code rescue nil
problematic_code rescue raise rescue puts "rescued again"
