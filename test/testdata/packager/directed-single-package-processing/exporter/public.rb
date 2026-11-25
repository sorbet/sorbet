# typed: strict

class Exporter::Public
end

Exporter::Alias = Exporter::Public
Exporter::StaticField = T.let(0, Integer)
Exporter::TypeAlias = T.type_alias {Integer}
