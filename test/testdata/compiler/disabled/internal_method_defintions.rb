# frozen_string_literal: true
# typed: true
# compiled: true

def internal
  p "outside"
end

def wrapper
  def internal
    p "inside"
  end
end

p internal
