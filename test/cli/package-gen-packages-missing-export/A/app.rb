# typed: strict

module A
  class App
    def self.run
      B::App.run
    end
  end
end
