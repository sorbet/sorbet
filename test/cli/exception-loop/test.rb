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

def example1
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
end

def example2
  # works to pin this outside the loop
  e = T.let(nil, T.nilable(T.any(A, B)))

  begin
    raises_a
  rescue A => e
  end

  2.times do
    begin
      puts(e.class)
      T.reveal_type(e) # error: `T.nilable(T.any(A, B))`
      raises_b
    rescue B => e
    end
  end
end
