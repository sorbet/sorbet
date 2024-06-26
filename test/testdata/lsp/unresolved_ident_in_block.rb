# typed: true

class A
  def self.random_method
# ^^^^^^^^^^^^^^^^^^^^^^ def: random_method
  end

  random_method { @foo = 1 }
# ^^^^^^^^^^^^^ usage: random_method
end
