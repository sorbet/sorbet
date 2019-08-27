# typed: true
class MyTest
  def outside_method
  end

  [true, false].each do |value|
    puts value
    it "works outside" do
      outside_method
    end
  end

  [true, false].each do |value|
    puts value
    describe "some inner tests" do
      def inside_method
      end

      it "works inside" do
        outside_method
        inside_method
      end
    end
  end
end
