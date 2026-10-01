// Flattens every downloaded artifact into <out>/<artifact-name> and re-labels
// the checksum with that same name, so each sidecar sits next to the asset it
// describes.
// Usage: node stage-assets.js [artifacts-dir] [out-dir]
const c = require('crypto');
const fs = require('fs');
const path = require('path');

const src = process.argv[2] || 'artifacts';
const out = process.argv[3] || 'release';

const walk = (d) => fs.readdirSync(d, { withFileTypes: true })
  .flatMap((e) => (e.isDirectory() ? walk(path.join(d, e.name)) : [path.join(d, e.name)]));

fs.mkdirSync(out, { recursive: true });
for (const file of walk(src)) {
  // Anything sitting directly in src/ is not an artifact payload, and naming
  // it after src would stage a junk release asset called "artifacts".
  if (path.dirname(file) === src) continue;
  if (file.endsWith('.sha256')) continue; // recomputed below, under the new name
  const name = path.basename(path.dirname(file));
  fs.copyFileSync(file, path.join(out, name));
  const sum = c.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
  fs.writeFileSync(path.join(out, name + '.sha256'), sum + '  ' + name + '\n');
  console.log('staged ' + name);
}