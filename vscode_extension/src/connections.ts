import { ChildProcess } from "child_process";
import { Log } from "./log";

/**
 * Attempts to stop the given child process. Tries a SIGTERM, then a SIGKILL.
 */
export async function stopProcess(p: ChildProcess, log: Log): Promise<void> {
  return new Promise<void>((resolve) => {
    let hasExited = false;
    log.debug("Stopping process", p.pid);
    function onExit() {
      if (!hasExited) {
        hasExited = true;
        resolve();
      }
    }
    p.on("exit", onExit);
    p.on("error", onExit);
    p.kill("SIGTERM");
    setTimeout(() => {
      if (!hasExited) {
        log.debug(
          "Process did not respond to SIGTERM with 1s. Sending a SIGKILL.",
          p.pid,
        );
        p.kill("SIGKILL");
        setTimeout(resolve, 100);
      }
    }, 1000);
  });
}
