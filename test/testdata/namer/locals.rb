class TestLocals
  def method
    a = 1
    c = 4

    proc do # error: Method proc does not exist on TestLocals
      b = 2
      a + b # error: Method + does not exist on Integer
      proc do # error: Method proc does not exist on TestLocals
        b + c # error: Method + does not exist on Integer
      end
    end

    b = 3

    proc do |a; b| # error: Method proc does not exist on TestLocals
      a + b
    end
  end
end
