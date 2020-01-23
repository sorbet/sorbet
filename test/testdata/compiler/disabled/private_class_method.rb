# frozen_string_literal: true
# typed: true
# compiled: true
module A
  private_class_method def self.pcm(); end;
  def self.call()
          pcm;
  end
end

A.call()
