# typed: true

# Undeclared fields cannot be renamed

class Person
  attr_reader :age

  def initialize
    @age = 1
#    ^ apply-rename: [A] newName: @years_of_age placeholderText: @age
  end

  def information
    "John is #{@age} years old."
  end
end

class Config
  @@name = "Configs"
#   ^ apply-rename: [B] newName: @@package placeholderText: @@name

  def self.package_name
    @@name
  end

  class << self
    def initialize_version
      @version = "1.0"
#      ^ apply-rename: [C] newName: @version_string placeholderText: @version
    end

    def version
      @version
    end
  end
end
