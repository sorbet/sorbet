# typed: false
next &a # error-with-dupes: Block argument should not be given
next a, &b # error-with-dupes: Block argument should not be given
break &a # error-with-dupes: Block argument should not be given
break a, &b # error-with-dupes: Block argument should not be given
return &a # error-with-dupes: Block argument should not be given
return a, &b # error-with-dupes: Block argument should not be given
