# typed: ignore

class Foo 
    # ^ hover: # No hover information available
    # ^ hover: ---
    # ^ hover: The file is `# typed: ignore`

  def foo
    # ^ hover: # No hover information available
    # ^ hover: ---
    # ^ hover: The file is `# typed: ignore`
    x = 10
  # ^ hover: # No hover information available
  # ^ hover: ---
  # ^ hover: The file is `# typed: ignore`
    x
  end
end
