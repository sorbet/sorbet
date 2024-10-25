# typed: false

# "Match writes" let you bind regex capture groups directly into new variables.
# Sorbet doesn't treat this syntax in a special way, so it doesn't know that it introduces new local variables.

# Assigns the captured value to `new_local_var1`
/(?<new_local_var1> foo)/ =~ input1

# Test multiple captures
/(?<new_local_var2> bar) (?<new_local_var3> baz)/ =~ input2

# This does not work for Constants, @instance_var, @@class_var or @global_var
