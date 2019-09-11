# typed: true
module A4;
end # A4
module B4;
  def foo; super + "b"; end;
end # B4
module C4 # C4, A4
  def foo; "c"; end;
  include A4
end
module D4 # D4, A4, F4
  include B4
  include A4
end
class E4 # E4, D4, B4 C4, A4, B4
  include C4
  include D4
end
E4.new.foo

