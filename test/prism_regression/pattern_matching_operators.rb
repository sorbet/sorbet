# typed: false

# matching expressions to patterns
match in 1
no_match in 1, 2
[match_1, match_2] in 1, 2

# assigning expressions to patterns
1 in var
[1, 2] in var_a, var_b

# guards don't work outside of case-based pattern matching
# so this should be normal conditional assignment
[3, 4] in c, d if d == c*2


# matching expressions to patterns
match => 10
no_match => 10, 20
[match_1, match_2] => 10, 20

# assigning expressions to patterns
10 => var_c
[10, 20] => var_c, var_d

# guards don't work outside of case-based pattern matching
# so this should be normal conditional assignment
[30, 40] => var_c, var_d if var_d == var_c*2
