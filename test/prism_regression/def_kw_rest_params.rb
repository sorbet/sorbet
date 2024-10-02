# typed: false

def foo(**a); end

def foo(**); end

def foo(**nil); end # Disallows keyword params, which is now the default in Ruby 3. Ruby 2 could might still have this.
