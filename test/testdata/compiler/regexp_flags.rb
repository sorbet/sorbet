# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL OPT

def noflags(x)
  x.match(/pattern/)
end

# INITIAL-LABEL: define i64 @"func_Object#7noflags"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_0
# INITIAL-LITERAL: }

# OPT-LABEL: define i64 @"func_Object#7noflags"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_0
# OPT-NOT: ic_new
# OPT-LITERAL: }

def i_flag(x)
  x.match(/pattern/i)
end

# INITIAL-LABEL: define i64 @"func_Object#6i_flag"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_1
# INITIAL-LITERAL: }

# OPT-LABEL: define i64 @"func_Object#6i_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_1
# OPT-NOT: ic_new
# OPT-LITERAL: }

def ix_flag(x)
  x.match(/pattern/ix)
end

# INITIAL-LABEL: define i64 @"func_Object#7ix_flag"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_3
# INITIAL-LITERAL: }

# OPT-LABEL: define i64 @"func_Object#7ix_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_3
# OPT-NOT: ic_new
# OPT-LITERAL: }

def ixm_flag(x)
  x.match(/pattern/ixm)
end

# INITIAL-LABEL: define i64 @"func_Object#8ixm_flag"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_7
# INITIAL-LITERAL: }

# OPT-LABEL: define i64 @"func_Object#8ixm_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_7
# OPT-NOT: ic_new
# OPT-LITERAL: }

def x_flag(x)
  x.match(/pattern/x)
end

# INITIAL-LABEL: define i64 @"func_Object#6x_flag"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_2
# INITIAL-LITERAL: }

# OPT-LABEL: define i64 @"func_Object#6x_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_2
# OPT-NOT: ic_new
# OPT-LITERAL: }

def xm_flag(x)
  x.match(/pattern/xm)
end

# INITIAL-LABEL: define i64 @"func_Object#7xm_flag"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_6
# INITIAL-LITERAL: }

# OPT-LABEL: define i64 @"func_Object#7xm_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_6
# OPT-NOT: ic_new
# OPT-LITERAL: }

def m_flag(x)
  x.match(/pattern/m)
end

# INITIAL-LABEL: define i64 @"func_Object#6m_flag"(
# INITIAL: load i64, i64* @rubyRegexpFrozen_pattern_4
# INITIAL-LITERAL: }

# OPT-LABEL: define i64 @"func_Object#6m_flag"(
# OPT-NOT: ic_new
# OPT: load i64, i64* @rubyRegexpFrozen_pattern_4
# OPT-NOT: ic_new
# OPT-LITERAL: }
