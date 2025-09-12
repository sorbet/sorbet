# typed: true

def it(name, &blk); end
def test_each(arg, &blk); end

class Test

  test_each([1,2,3]) do |val|
    # Make sure that we can handle `it` with no block
    it "test case"
  # ^^^^^^^^^^^^^^ error: Only valid `it`
  end

end
