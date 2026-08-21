# typed: false

1
&& 2 # error: unexpected token
&& 3

1
|| 2 # error: unexpected token
|| 3

final = first_condition
  && second_condition # error: unexpected token
