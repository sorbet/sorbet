# frozen_string_literal: true
# typed: true
# compiled: true

def noflags(x)
  x.match(/pattern/)
end

# INITIAL-LITERAL: }

# OPT-LITERAL: }

def i_flag(x)
  x.match(/pattern/i)
end

# INITIAL-LITERAL: }

# OPT-LITERAL: }

def ix_flag(x)
  x.match(/pattern # Comment/ix)
end

# INITIAL-LITERAL: }

# OPT-LITERAL: }

def ixm_flag(x)
  x.match(/pattern.more # Comment/ixm)
end

# INITIAL-LITERAL: }

# OPT-LITERAL: }

def x_flag(x)
  x.match(/pattern # Comment/x)
end

# INITIAL-LITERAL: }

# OPT-LITERAL: }

def xm_flag(x)
  x.match(/pattern.still more # Comment/xm)
end

# INITIAL-LITERAL: }

# OPT-LITERAL: }

def m_flag(x)
  x.match(/pattern.mflag/m)
end

# INITIAL-LITERAL: }

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
