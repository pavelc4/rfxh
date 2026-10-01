// Writes the `files` step output: the comma-separated absolute paths of every
// packed binary to send to Telegram.
// Usage: node collect-binaries.js [pack-dir]
//
// Paths are resolved here and not in the packaging job, because the artifact
// is downloaded onto a different runner -- any path captured earlier is stale.
const fs = require('fs');
const path = require('path');

const dir = process.argv[2] || 'pack';
const list = fs.existsSync(dir)
  ? fs.readdirSync(dir).filter((f) => f.endsWith('.gz')).map((f) => path.resolve(dir, f))
  : [];

fs.appendFileSync(process.env.GITHUB_OUTPUT, 'files=' + list.join(',') + '\n');
console.log(list.length + ' file(s): ' + list.join(', '));