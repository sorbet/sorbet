# frozen_string_literal: true
# typed: true
# compiled: true
module Space
  def self.[](key)
    :frob
  end

  def self.[]=(key, value)
    :blob
  end
end

p Space[:foo]
x = Space[:bar] = 2
p x
