# typed: true

class GraphQL::Schema::Enum; end
module GraphQL::Schema::Member::AcceptsDefinition::ToGraphQLExtension; end
module Opus::Manage::GraphQL
  ControlledContentName = T.let(
    T.unsafe(nil),
    T.class_of(GraphQL::Schema::Enum),
  )
end
