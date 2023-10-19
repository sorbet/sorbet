# typed: true

def describe(name, &blk); end
def it(name, &blk); end
def before(name, &blk); end
def after(name, &blk); end
def test_each(arg, &blk); end
def test_each_hash(hash, &blk); end

describe 'example' do
  test_each(['foo']) do |x|
    it "bar #{x}" do
      # No results
      # ^ hover: null
    end
  end
end
