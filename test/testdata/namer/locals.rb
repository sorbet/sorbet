# @typed
class TestLocals
  def method
    a = 1
    c = 4

    proc do
      b = 2
      a + b
      proc do
        b + c
      end
    end

    b = 3

    proc do |a; b|
      a + b
    end
  end
end
