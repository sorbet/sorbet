# typed: true
def foo
  bar
end

def bar
  bt = caller
  bt.map do |row|
    row.gsub(/^[^:]*:/, '')
  end
end

# puts foo
