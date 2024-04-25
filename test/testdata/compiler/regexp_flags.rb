# frozen_string_literal: true
# typed: true
# compiled: true

def noflags(x)
  x.match(/pattern/)
end

def i_flag(x)
  x.match(/pattern/i)
end

def ix_flag(x)
  x.match(/pattern # Comment/ix)
end

def ixm_flag(x)
  x.match(/pattern.more # Comment/ixm)
end

def x_flag(x)
  x.match(/pattern # Comment/x)
end

def xm_flag(x)
  x.match(/pattern.still more # Comment/xm)
end

def m_flag(x)
  x.match(/pattern.mflag/m)
end

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
