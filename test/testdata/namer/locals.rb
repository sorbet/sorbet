class TestLocals
  def method
    a = 1

    proc do
      b = 2
      a + b
    end

    b = 3

    proc do |a; b|
      a + b
    end
  end
end
