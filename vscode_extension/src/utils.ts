/**
 * Compare two `string` arrays for deep, in-order equality.
 */
export function deepEqual(a: ReadonlyArray<string>, b: ReadonlyArray<string>) {
  return a.length === b.length && a.every((itemA, index) => itemA === b[index]);
}
