# typed: ignore

class Foo
    # ^ hover: This file is `# typed: ignore`.
    # ^ hover: Hover, Go To Definition, and other features are disabled in this file.

  def foo
    # ^ hover: This file is `# typed: ignore`.
    # ^ hover: Hover, Go To Definition, and other features are disabled in this file.
    x = 10
  # ^ hover: This file is `# typed: ignore`.
  # ^ hover: Hover, Go To Definition, and other features are disabled in this file.
    x
  end
end
