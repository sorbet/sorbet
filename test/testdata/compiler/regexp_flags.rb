# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL OPT

def noflags(x)
  x.match(/pattern/)
end

# INITIAL-LABEL: define internal i64 @"func_Object#7noflags"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_0
# INITIAL-LITERAL: }

# OPT-LABEL: define internal i64 @"func_Object#7noflags"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_0
# OPT-NOT: ic_new
# OPT-LITERAL: }

def i_flag(x)
  x.match(/pattern/i)
end

# INITIAL-LABEL: define internal i64 @"func_Object#6i_flag"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_1
# INITIAL-LITERAL: }

# OPT-LABEL: define internal i64 @"func_Object#6i_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_1
# OPT-NOT: ic_new
# OPT-LITERAL: }

def ix_flag(x)
  x.match(/pattern # Comment/ix)
end

# INITIAL-LABEL: define internal i64 @"func_Object#7ix_flag"(
# INITIAL: load i64, i64* @"rubyRegexpFrozen_pattern # Comment_3"
# INITIAL-LITERAL: }

# OPT-LABEL: define internal i64 @"func_Object#7ix_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @"rubyRegexpFrozen_pattern # Comment_3"
# OPT-NOT: ic_new
# OPT-LITERAL: }

def ixm_flag(x)
  x.match(/pattern.more # Comment/ixm)
end

# INITIAL-LABEL: define internal i64 @"func_Object#8ixm_flag"(
# INITIAL: load i64, i64* @"rubyRegexpFrozen_pattern.more # Comment_7"
# INITIAL-LITERAL: }

# OPT-LABEL: define internal i64 @"func_Object#8ixm_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @"rubyRegexpFrozen_pattern.more # Comment_7"
# OPT-NOT: ic_new
# OPT-LITERAL: }

def x_flag(x)
  x.match(/pattern # Comment/x)
end

# INITIAL-LABEL: define internal i64 @"func_Object#6x_flag"(
# INITIAL: load i64, i64* @"rubyRegexpFrozen_pattern # Comment_2"
# INITIAL-LITERAL: }

# OPT-LABEL: define internal i64 @"func_Object#6x_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @"rubyRegexpFrozen_pattern # Comment_2"
# OPT-NOT: ic_new
# OPT-LITERAL: }

def xm_flag(x)
  x.match(/pattern.still more # Comment/xm)
end

# INITIAL-LABEL: define internal i64 @"func_Object#7xm_flag"(
# INITIAL: load i64, i64* @"rubyRegexpFrozen_pattern.still more # Comment_6"
# INITIAL-LITERAL: }

# OPT-LABEL: define internal i64 @"func_Object#7xm_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @"rubyRegexpFrozen_pattern.still more # Comment__6
# OPT-NOT: ic_new
# OPT-LITERAL: }

def m_flag(x)
  x.match(/pattern.mflag/m)
end

# INITIAL-LABEL: define internal i64 @"func_Object#6m_flag"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern.mflag_4
# INITIAL-LITERAL: }

# OPT-LABEL: define internal i64 @"func_Object#6m_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern.mflag_4
# OPT-NOT: ic_new
# OPT-LITERAL: }

# Run some execution tests, too.
p noflags("pattern")
p noflags("PATTERN")
p i_flag("pattern")
p i_flag("PATTERN")
p ix_flag("pattern")
p ix_flag("PATTERN")
p ixm_flag("pattern\nmore")
p ixm_flag("PATTERN\nMORE")
p x_flag("pattern")
p x_flag("PATTERN")
p xm_flag("pattern\nstillmore")
p xm_flag("PATTERN\nstillmore")
p m_flag("pattern\nmflag")
p m_flag("PATTERN\nmflag")
