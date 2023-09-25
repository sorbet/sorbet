# typed: true
extend T::Sig

class A < StandardError; end
class B < StandardError; end

def raises_a
  raise A.new
end
def raises_b
  raise B.new
end

begin
  raises_a
rescue A => e
end

2.times do
  begin
    puts(e.class)
    T.reveal_type(e) # error: `T.nilable(A)`
    raises_b
  rescue B => e
    #         ^ error: Changing the type of a variable in a loop is not permitted
  end
end
