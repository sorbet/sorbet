# typed: true

class A
  def outer_helper; end

  describe "inside describe" do
    def my_helper; end

    xit do
      my_helper
    end

    it "example", focus: true do
      my_helper
    end

    example do
      my_helper
    end
  end

  example do # error: Method `example` does not exist
    my_helper # error: Method `outer_helper` does not exist
  end
end
