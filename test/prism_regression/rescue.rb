# typed: false

begin
  bar
rescue
  "rescued"
end

begin
  foo
  bar
rescue
  "rescued"
end

begin
  foo
rescue RuntimeError
  "rescued Foo"
end

begin
  foo
rescue RuntimeError, NotImplementedError => e
  "rescued Foo #{e}"
end

begin
  foo
rescue RuntimeError
  "rescued Foo"
else
  "rescued else"
end

begin
  foo
rescue RuntimeError => e
  "rescued Foo #{e}"
rescue NotImplementedError => e
  "rescued Bar #{e}"
rescue => e
  "rescued #{e}"
end

begin
  foo
rescue RuntimeError => e
  "rescued Foo #{e}"
rescue NotImplementedError => e
  "rescued Bar #{e}"
rescue => e
  "rescued #{e}"
else
  "rescued else"
end

problematic_code rescue puts "rescued"
problematic_code rescue nil
problematic_code rescue raise rescue puts "rescued again"
