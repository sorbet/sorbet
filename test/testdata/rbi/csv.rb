# typed: true
require 'csv'

csv = CSV::Table.new([CSV::Row.new(['1', '2'], [1, 2]), CSV::Row.new(['1', '2'], [2, 3])])
T.assert_type!(csv, CSV::Table[T.any(CSVRowModeElem, CSVColumnModeElem)])

csv = CSV::Table.new([CSV::Row.new(['1', '2'], [1, 2]), CSV::Row.new(['1', '2'], [2, 3])], headers: ['1', '2'])
T.assert_type!(csv, CSV::Table[T.any(CSVRowModeElem, CSVColumnModeElem)])

csv = CSV.parse("1,2,3\n3,,abc\n5,6,1", { headers: true, converters: %i[numeric] })

if csv.is_a?(CSV::Table)
  T.assert_type!(csv << [1, {}, nil, '1'], CSV::Table[T.any(CSVRowModeElem, CSVColumnModeElem)])

  T.assert_type!(csv == csv, T::Boolean)

  T.assert_type!(csv[0], T.any(T.nilable(CSV::Row), T::Array[T::nilable(BasicObject)]))
  T.assert_type!(csv[0..1], T.any(T::Array[CSV::Row], T::Array[T::Array[T::nilable(BasicObject)]]))
  T.assert_type!(csv[{}], T::Array[T::nilable(BasicObject)])

  T.assert_type!(csv[5] = CSV::Row.new(['1', '2'], [1, 2]), CSV::Row)

  T.assert_type!(csv.by_col, CSVColumnModeTable)
  T.assert_type!(csv.by_col!, CSVColumnModeTable)
  T.assert_type!(csv.by_col_or_row, CSV::Table[T.any(CSVRowModeElem, CSVColumnModeElem)])
  T.assert_type!(csv.by_col_or_row!, CSV::Table[T.any(CSVRowModeElem, CSVColumnModeElem)])
  T.assert_type!(csv.by_row, CSVRowModeTable)
  T.assert_type!(csv.by_row!, CSVRowModeTable)

  T.assert_type!(csv.delete(0), T.any(T.nilable(CSV::Row), T::Array[T.nilable(BasicObject)], T::Array[T.nilable(CSV::Row)], T::Array[T::Array[T.nilable(BasicObject)]]))
  T.assert_type!(csv.delete(0, 1), T.any(T.nilable(CSV::Row), T::Array[T.nilable(BasicObject)], T::Array[T.nilable(CSV::Row)], T::Array[T::Array[T.nilable(BasicObject)]]))
  T.assert_type!(csv.delete(0, {}, 1), T.any(T.nilable(CSV::Row), T::Array[T.nilable(BasicObject)], T::Array[T.nilable(CSV::Row)], T::Array[T::Array[T.nilable(BasicObject)]]))

  T.assert_type!(csv.delete_if, T::Enumerator[T.any(CSVRowModeElem, CSVColumnModeElem)])
  T.assert_type!(csv.delete_if, T::Enumerator[T.any(CSVRowModeElem, CSVColumnModeElem)])
  T.assert_type!(csv.delete_if, T::Enumerator[[T.nilable(BasicObject), T.any(CSVRowModeElem, CSVColumnModeElem)]])

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
  T.assert_type!(csv.by_row.dig(0), CSV::Row)
  T.assert_type!(csv.by_col.dig(0), T::Array[T.nilable(BasicObject)])
  T.assert_type!(csv.dig(0, 1), T.nilable(BasicObject))

  T.assert_type!(csv.headers, T::Array[BasicObject])

  T.assert_type!(csv.inspect, String)
  
  T.assert_type!(csv.to_a, T::Array[T::Array[T.nilable(BasicObject)]])

  T.assert_type!(csv.to_csv, String)
  T.assert_type!(csv.to_s, String)

  T.assert_type!(csv.values_at(0, {}), T.any(T.nilable(CSV::Row), T::Array[T.nilable(BasicObject)], T::Array[T.nilable(CSV::Row)], T::Array[T::Array[T.nilable(BasicObject)]]))
else
  T.assert_type!(csv, T::Array[T::Array[T.nilable(BasicObject)]])
end
