# typed: true
module A5;
end
module B5;
  def foo; super + "b"; end;
end # B5
class C5 # C5, A5
  def foo; "c"; end;
  include A5
end
module D5 # D5, A5, B5
  include B5
  include A5
end
class E5 < C5 # E5, D5, B5, C5, A5
  include D5
end
E5.new.foo
