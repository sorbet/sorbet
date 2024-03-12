/**
 * Compare two `string` arrays for deep, in-order equality.
 */
export function deepEqual(a: ReadonlyArray<string>, b: ReadonlyArray<string>) {
  return a.length === b.length && a.every((itemA, index) => itemA === b[index]);
}

export function deepEqualEnv(
  a: Readonly<NodeJS.ProcessEnv>,
  b: Readonly<NodeJS.ProcessEnv>,
) {
  const keysA = Object.keys(a);
  const keysB = Object.keys(b);

  return (
    keysA.length === keysB.length &&
    keysA.every((key) => a[key] === b[key]) &&
    keysB.every((key) => a[key] === b[key])
  );
}
