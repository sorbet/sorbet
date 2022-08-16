# typed: strict
class MyTest
  it "allows let-ed constants inside of IT" do
    C2 = T.let(10, Integer)
  end
end
