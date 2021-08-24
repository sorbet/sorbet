# frozen_string_literal: true
# typed: true

array = T.let([1,2,3,5], T::Array[T.untyped])
Thread.current[:override] = :x
Thread.current[:shortest] = :y
y = :toplevel
begin
  y = A.with_wrap(array) do
    p "with_wrap block #{Thread.current[:override]} #{Thread.current[:shortest]}"
    break :inside
  end
rescue
  p "caught exception"
end

p "y: #{y}"

p Thread.current[:override]
p Thread.current[:shortest]
p array
