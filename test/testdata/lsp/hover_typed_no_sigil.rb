class Foo 
    # ^ hover: # No hover information available
    # ^ hover: ---
    # ^ hover: The file is `# typed: false`

  def foo
    # ^ hover: # No hover information available
    # ^ hover: ---
    # ^ hover: The file is `# typed: false`
    x = 10
  # ^ hover: # No hover information available
  # ^ hover: ---
  # ^ hover: The file is `# typed: false`
    x
  end
end
