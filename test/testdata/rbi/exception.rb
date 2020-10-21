# typed: strict

StandardError.new
StandardError.new(nil)
StandardError.new('msg')
StandardError.new(:msg)
ex = StandardError.new("bees")
RuntimeError.new(ex)
