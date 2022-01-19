# typed: true

def rescued(x) = raise "to be caught" rescue "instance #{x}"
