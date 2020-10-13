# typed: true
# frozen_string_literal: true

require_relative './rename__class_definition.rb'

foo = Foo.new
#     ^ apply-rename: [A] newName: Bar
