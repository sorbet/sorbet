# typed: true
class A
  cls = A
end

foo {|Cls| Cls = A}
