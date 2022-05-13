# typed: true

class Foo
  def bar
    my_local_variable = 123
#   ^ apply-rename: [A] newName: my_new_local_variable placeholderText: my_local_variable
    process(my_local_variable)
    my_local_variable
  end

  def baz
    my_local_variable = 123
    my_local_variable.to_s
  end

  private

  def process(a)
  end
end

class DifferentScopeLevels
  def foo
    variable = "123"
#   ^ apply-rename: [B] newName: new_variable placeholderText: variable

    other = [1, 2, 3].map do |variable|
      variable.to_s
    end

    variable
  end

  def bar
    5.times do
      local = 123
    # ^ apply-rename: [C] newName: new_local placeholderText: local
      local * 5
    end

    local = "something different"
  # ^ apply-rename: [F] newName: new_local placeholderText: local
    local
  end

  def cond
    if Random.rand(2).even?
      x = 0
    # ^ apply-rename: [D] newName: y placeholderText: x
    else
      x = ""
    end
  end

  def higher_level
    higher = 123
  # ^ apply-rename: [E] newName: new_higher placeholderText: higher

    5.times do
      higher = 321
    end

    higher
  end
end

class MethodParameter
  def add(a, b)
        # ^ apply-rename: [G] newName: c placeholderText: a
    a + b
  end
end

class BlockParameter
  def foo
    variable = 1

    [1, 2, 3].map do |variable|
                    # ^ apply-rename: [H] newName: new_variable placeholderText: variable
      variable.to_s
    end

    variable
  end

  def bar
    variable = 1

    [1, 2, 3].map do |variable|
      variable.to_s
    # ^ apply-rename: [I] newName: new_variable placeholderText: variable
    end

    variable
  end
end
