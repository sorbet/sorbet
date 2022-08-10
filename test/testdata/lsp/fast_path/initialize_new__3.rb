# typed: true
# spacer for exclude-from-file-update

class A
  sig {void}
  def self.example
    new(0)
    new('') # error: Expected `Integer` but found `String("")`
  end
end
