# typed: true

def self.f; ::Bar = 1; end # error: dynamic constant assignment
