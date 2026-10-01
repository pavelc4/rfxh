// One .gz per downloaded artifact directory, named after the directory so the
// Telegram filename still says which target it is.
// Usage: node gzip.js [artifacts-dir] [out-dir]
//
// Telegram caps a single upload at 50 MB, so compression is not optional.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const src = process.argv[2] || 'artifacts';
const out = process.argv[3] || 'pack';

fs.mkdirSync(out, { recursive: true });
for (const entry of fs.readdirSync(src)) {
  const dir = path.join(src, entry);
  if (!fs.statSync(dir).isDirectory()) continue;
  // The .sha256 sidecar is uploaded separately; only the binary gets packed.
  const bin = fs.readdirSync(dir).find((f) => !f.endsWith('.sha256'));
  if (!bin) continue;
  const gz = path.join(out, entry + '.gz');
  fs.writeFileSync(gz, zlib.gzipSync(fs.readFileSync(path.join(dir, bin))));
  console.log(entry + '/' + bin + ' -> ' + gz);
}