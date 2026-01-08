## 6. Why was progressbar.BUILD removed?

The `third_party/progressbar.BUILD` file was orphaned - it was never used. The actual progressbar BUILD file lives at `third_party/progressbar/BUILD` (inside the progressbar directory). The deleted file was a leftover that wasn't referenced anywhere.
