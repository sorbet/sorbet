# typed: true

def test
  opts = {}
  opts[:k] = :v
  if opts[:k]
    puts "yes" # <<-- we complain that this is unreachable
  end
end
