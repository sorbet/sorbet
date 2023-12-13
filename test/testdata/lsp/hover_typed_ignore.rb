# typed: ignore

class Foo
    # ^ hover: This file is `# typed: ignore`.
    # ^ hover: No Sorbet IDE features will work in this file.

  def foo
    # ^ hover: This file is `# typed: ignore`.
    # ^ hover: No Sorbet IDE features will work in this file.
    x = 10
  # ^ hover: This file is `# typed: ignore`.
    # ^ hover: No Sorbet IDE features will work in this file.
    x
  end
end
