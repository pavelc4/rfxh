// sha256 sidecar for one built binary, so the artifact carries its own
// checksum. Usage: node checksum.js build/rfxh
//
// The label in the sidecar is the binary's basename, which is what makes the
// file checkable with `sha256sum -c` after it lands next to the binary.
const c = require('crypto');
const f = require('fs');
const path = require('path');

const bin = process.argv[2];
if (!bin) throw new Error('usage: node checksum.js <binary>');

const name = path.basename(bin);
const sum = c.createHash('sha256').update(f.readFileSync(bin)).digest('hex');
f.writeFileSync(bin + '.sha256', sum + '  ' + name + '\n');
console.log(sum + '  ' + name);