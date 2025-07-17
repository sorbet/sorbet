# typed: false

# Comments represent the value in the parse tree

42        # "42"
4_2       # "4_2"

+42       # "+42"
+4_2      # "+4_2"
-42       # "-42"
-4_2      # "-4_2"

# ~ should not be a method call
~42       # "~42"

# Hexadecimal literal
0xcafe    # "51966"
0xca_fe   # "0"
0XCAFE    # "51966"
+0xcafe   # "+51966"
+0xca_fe  # "+0"
-0xcafe   # "-51966"
-0xca_fe  # "-0"

# Decimal literal
0d123     # "123"
0d1_23    # "1_23"
0D123     # "123"
+0d123    # "+123"
+0d1_23   # "+1_23"
-0d123    # "-123"
-0d1_23   # "-1_23"

# Octal literal
0o777     # "511"
0o7_77    # "0"
0O777     # "511"
+0o777    # "+511"
-0o777    # "-511"
+00777    # "+511"
-00777    # "-511"

# Binary literal
0b101010    # "42"
0b10_1010   # "0"
0B101010    # "42"
+0b101010   # "+42"
+0b10_1010  # "+0"
-0b101010   # "-42"
-0b10_1010  # "-0"
