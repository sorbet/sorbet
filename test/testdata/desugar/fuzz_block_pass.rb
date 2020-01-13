# typed: false
next &a # error: Block argument should not be given
next a, &b # error: Block argument should not be given
break &a # error: Block argument should not be given
break a, &b # error: Block argument should not be given
return &a # error: Block argument should not be given
return a, &b # error: Block argument should not be given
