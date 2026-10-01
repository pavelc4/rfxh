// Decides which Telegram call answers a given notification, and with what.
// The transport (telegram.js) and the wording (message.js) stay out of here.

const { execSync } = require('child_process');
const fs = require('fs');

const tg = require('./telegram.js');
const msg = require('./message.js');

function readMeta(env) {
  let subject = '';
  try {
    subject = execSync('git log -1 --pretty=%s', {
      encoding: 'utf8',
      stdio: ['ignore', 'pipe', 'ignore'],
    }).trim();
  } catch (e) {
    subject = ''; // shallow clone or no git: caption just omits it
  }
  return {
    subject,
    runNumber: env.TG_RUN_NUMBER,
    commitUrl: env.TG_COMMIT_URL,
    workflowUrl: env.TG_WORKFLOW_URL,
  };
}

function readFiles(env) {
  return (env.TG_FILES || '')
    .split(',')
    .map((p) => p.trim())
    .filter((p) => p && fs.existsSync(p) && !p.endsWith('.sha256'));
}

async function run(core, env) {
  const ctx = { core, token: env.TG_TOKEN, chat: env.TG_CHAT };
  const editId = env.TG_EDIT_ID;
  const meta = readMeta(env);

  if (env.TG_TYPE === 'start') {
    const r = await tg.sendText(ctx, env.TG_CAPTION || msg.ok(meta));
    if (r) core.setOutput('message_id', r.result.message_id);
    return;
  }

  const caption = env.TG_CAPTION || (env.TG_TYPE === 'failure'
    ? msg.failed(meta)
    : msg.rich(meta));
  const files = readFiles(env);

  // The start message is reused as the finished caption so the chat never grows
  // duplicates. One file can be swapped straight into it with editMessageMedia;
  // an album cannot be, so those go up as an album under the edited caption.
  if (files.length === 0) {
    await (editId ? tg.editText(ctx, caption, editId) : tg.sendText(ctx, caption));
  } else if (files.length === 1) {
    await (editId
      ? tg.editMedia(ctx, files[0], caption, editId)
      : tg.sendDocument(ctx, files[0], caption));
  } else {
    await (editId ? tg.editText(ctx, caption, editId) : tg.sendText(ctx, caption));
    await tg.sendAlbum(ctx, files);
  }
}

module.exports = { run, readMeta, readFiles };