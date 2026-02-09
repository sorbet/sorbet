# typed: true

def describe(name, &blk); end
def it(name, &blk); end
def before(name, &blk); end
def after(name, &blk); end
def test_each(arg, &blk); end
def test_each_hash(hash, &blk); end

X = "hello"

describe 'example' do
  x = "hello"

  it "outside test_each #{x}" do
    # No results
    # ^ hover: null
  end

  it "outside test_each #{y}" do
    #                     ^ error: Method `y` does not exist
    # No results
    # ^ hover: nulasd
  end

  it "outside test_each #{X}" do
    # No results
    # ^ hover: null
  end

  test_each(['foo']) do |x|
    it "bar #{x}" do
      # No results
      # ^ hover: null
    end
  end
end
