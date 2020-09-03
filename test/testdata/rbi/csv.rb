# typed: true
require 'csv'

T.assert_type!(CSV.foreach('source.csv', headers: true), T::Enumerator[T.any(T::Array[T.nilable(BasicObject)], CSV::Row)])
T.assert_type!(CSV.foreach('source.csv', headers: true, col_sep: ','), T::Enumerator[T.any(T::Array[T.nilable(BasicObject)], CSV::Row)])
CSV.foreach('source.csv') do |row|
  T.assert_type!(row, T.any(T::Array[T.untyped], CSV::Row))
end

csv = CSV::Table.new([CSV::Row.new(['1', '2'], [1, 2]), CSV::Row.new(['1', '2'], [2, 3])])
T.assert_type!(csv, CSV::Table)

csv = CSV::Table.new([CSV::Row.new(['1', '2'], [1, 2]), CSV::Row.new(['1', '2'], [2, 3])], headers: ['1', '2'])
T.assert_type!(csv, CSV::Table)

CSV.parse("1,2,3\n3,,abc\n5,6,1") do |line|
  T.assert_type!(line, T.any(CSV::Row, T::Array[T.untyped]))
end

csv = CSV.parse("1,2,3\n3,,abc\n5,6,1", { headers: true, converters: %i[numeric] })

if csv.is_a?(CSV::Table)
  T.assert_type!(csv << [1, {}, nil, '1'], CSV::Table)

  T.assert_type!(csv == csv, T::Boolean)

  T.assert_type!(csv[0], T.any(T.nilable(CSV::Row), T::Array[T::nilable(BasicObject)]))
  T.assert_type!(csv[0..1], T.any(T::Array[CSV::Row], T::Array[T::Array[T::nilable(BasicObject)]]))
  T.assert_type!(csv[{}], T::Array[T::nilable(BasicObject)])

  T.assert_type!(csv[5] = CSV::Row.new(['1', '2'], [1, 2]), CSV::Row)

  T.assert_type!(csv.by_col, CSV::Table)
  T.assert_type!(csv.by_col!, CSV::Table)
  T.assert_type!(csv.by_col_or_row, CSV::Table)
  T.assert_type!(csv.by_col_or_row!, CSV::Table)
  T.assert_type!(csv.by_row, CSV::Table)
  T.assert_type!(csv.by_row!, CSV::Table)

  T.assert_type!(csv.delete(0), T.any(T.nilable(CSV::Row), T::Array[T.nilable(BasicObject)], T::Array[T.nilable(CSV::Row)], T::Array[T::Array[T.nilable(BasicObject)]]))
  T.assert_type!(csv.delete(0, 1), T.any(T.nilable(CSV::Row), T::Array[T.nilable(BasicObject)], T::Array[T.nilable(CSV::Row)], T::Array[T::Array[T.nilable(BasicObject)]]))
  T.assert_type!(csv.delete(0, {}, 1), T.any(T.nilable(CSV::Row), T::Array[T.nilable(BasicObject)], T::Array[T.nilable(CSV::Row)], T::Array[T::Array[T.nilable(BasicObject)]]))

  T.assert_type!(csv.delete_if, T::Enumerator[T.any(CSV::Row, T::Array[T.nilable(BasicObject)])])
  T.assert_type!(csv.delete_if, T::Enumerator[T.any(CSV::Row, T::Array[T.nilable(BasicObject)])])
  T.assert_type!(csv.delete_if, T::Enumerator[[T.nilable(BasicObject), T.any(CSV::Row, T::Array[T.nilable(BasicObject)])]])

  csv.by_col.delete_if do |header, entry|
  end

  csv.delete_if do |entry|
  end

  csv.delete_if do |header, entry|
  end

  csv.by_row.delete_if do |entry|
  end

  csv.delete_if.with_index do |entry, index|
    T.assert_type!(index, Integer)
  end

  T.assert_type!(csv.dig(0), T.any(CSV::Row, T::Array[T.nilable(BasicObject)]))
  T.assert_type!(csv.by_row.dig(0), T.any(CSV::Row, T::Array[T.nilable(BasicObject)]))
  T.assert_type!(csv.by_col.dig(0), T.any(CSV::Row, T::Array[T.nilable(BasicObject)]))
  T.let(csv.dig(0, 1), T.untyped)

  T.assert_type!(csv.headers, T::Array[BasicObject])

  T.assert_type!(csv.inspect, String)

  T.assert_type!(csv.mode, Symbol)

  T.assert_type!(csv.push([[1, {}, nil, '1'], CSV::Row.new(['1', '2'], [1, 2])]), CSV::Table)

  T.assert_type!(csv.to_a, T::Array[T::Array[T.nilable(BasicObject)]])

  T.assert_type!(csv.to_csv, String)
  T.assert_type!(csv.to_s, String)

  T.assert_type!(csv.values_at(0, {}), T.any(T.nilable(CSV::Row), T::Array[T.nilable(BasicObject)], T::Array[T.nilable(CSV::Row)], T::Array[T::Array[T.nilable(BasicObject)]]))

  csv.by_col.each do |header, entry|
  end

  csv.each do |entry|
  end

  csv.each do |header, entry|
  end

  csv.by_row.each do |entry|
  end

  # errors
  csv.table # error: Method `table` does not exist on `CSV::Table`
else
  T.assert_type!(csv, T::Array[T::Array[T.nilable(BasicObject)]])
end
