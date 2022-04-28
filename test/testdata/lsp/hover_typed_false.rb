# typed: false

class Foo
    # ^ hover: This file is `# typed: false`.
    # ^ hover: Hover, Go To Definition, and other features are disabled in this file.

  def foo
    # ^ hover: This file is `# typed: false`.
    # ^ hover: Hover, Go To Definition, and other features are disabled in this file.
    x = 10
  # ^ hover: This file is `# typed: false`.
  # ^ hover: Hover, Go To Definition, and other features are disabled in this file.
    x
  end
end
