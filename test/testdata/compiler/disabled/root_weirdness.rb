# frozen_string_literal: true
# typed: true
# compiled: true
p self

def on_object
  puts 'on_object'
end

def self.on_class_of_object
  puts 'self.on_class_of_object'
end

class << self
  def on_root_singleton
    puts 'on_root_singleton'
  end
end

on_object
on_class_of_object
on_root_singleton

begin
  1.on_object
rescue => e
  p e
end

begin
  1.on_class_of_object
rescue => e
  p e
end

begin
  1.on_root_singleton
rescue => e
  p e
end
