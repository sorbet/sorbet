# typed: false

begin
  123
rescue
  "rescued"
end

begin
  123
rescue RuntimeError
  "rescued Foo"
end

begin
  123
rescue RuntimeError, NotImplementedError => e
  "rescued Foo #{e}"
end

begin
  123
rescue RuntimeError
  "rescued Foo"
else
  "rescued else"
end

begin
  123
rescue RuntimeError => e
  "rescued Foo #{e}"
rescue NotImplementedError => e
  "rescued Bar #{e}"
rescue => e
  "rescued #{e}"
end

problematic_code rescue puts "rescued"
problematic_code rescue nil
problematic_code rescue raise rescue puts "rescued again"
