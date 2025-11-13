# typed: true

# Important! To correct test the location of the `Begin` locations:
#   1. This method needs to have multiple lines (otherwise the `Begin` nodes is skipped)
#   2. The node with a problematic start/end location needs to be the first or last node in the body

# TODO: Delete when https://github.com/sorbet/sorbet/issues/9630 is fixed
def multi_assign_parens_not_in_location
  (a, b) = 1, 2
  FILLER_LINE
end
