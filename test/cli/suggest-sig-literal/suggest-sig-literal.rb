# typed: strict
def index_for_live(fields)
  [[:deleted_at, 1]] + fields
end
