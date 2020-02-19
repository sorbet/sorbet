# frozen_string_literal: true
# typed: true
# compiled: true

module A
  private_class_method def self.pcm
    puts 218
  end

  def self.main
    pcm
  end
end

A.main
