import * as assert from "assert";
import * as path from "path";

import { deepEqual } from "../utils";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  test("deepEqual", () => {
    assert.ok(deepEqual([], []), "Empty");
    assert.ok(deepEqual(["a", "b", "c"], ["a", "b", "c"]), "Simple");

    assert.ok(!deepEqual(["a", "b", "c"], []), "Prefix");
    assert.ok(!deepEqual(["a", "b", "c"], ["a", "b"]), "Prefix");
    assert.ok(!deepEqual(["a", "b", "c"], ["c", "b", "a"]), "Out-of-order");
  });
});
