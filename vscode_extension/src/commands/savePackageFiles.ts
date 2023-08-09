import { workspace } from "vscode";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

/**
 * Save all open __package.rb files
 *
 * @param context Sorbet extension context.
 * @return true if all the files were successfully saved
 */
export async function savePackageFiles(
  context: SorbetExtensionContext,
): Promise<boolean> {
  context.log.trace("savePackageFiles");
  const allSaved = await Promise.all(
    workspace.textDocuments.map(async (document) => {
      context.log.warning(`editor.document.filename: ${document.fileName}`);
      if (document.fileName.endsWith("/__package.rb")) {
        return document.save();
      }

      return true;
    }),
  );

  return allSaved.every((x) => x);
}
