# typed: true

def self.rescued(x) = raise "to be caught" rescue "class #{x}"
