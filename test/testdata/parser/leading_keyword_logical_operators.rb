# typed: false

1
and 2
and 3

1
or 2
or 3

# These are identifiers, not keyword operators.
1
andfoo

2
orfoo

# Comments may appear before a leading keyword operator.
true
  # comment before and
  and false

false
  # comment before or
  or true
