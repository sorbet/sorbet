import { workspace } from "vscode";
import { basename } from "path";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

/**
 * Save all __package.rb files with changes.
 *
 * @param context Sorbet extension context.
 * @return `true` if all the files were successfully saved.
 */
export async function savePackageFiles(
  context: SorbetExtensionContext,
): Promise<boolean> {
  const packageDocuments = workspace.textDocuments.filter(
    (document) =>
      document.isDirty && basename(document.fileName) === "__package.rb",
  );
  if (!packageDocuments.length) {
    context.log.trace("savePackageFiles: nothing to save");
    return true;
  }

  const allSaved = await Promise.all(
    packageDocuments.map((document) => {
      context.log.trace(`savePackageFiles: saving ${document.fileName}`);
      return document.save();
    }),
  );

  return allSaved.every((x) => x);
}
