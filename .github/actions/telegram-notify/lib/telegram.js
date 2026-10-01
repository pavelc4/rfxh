// Telegram Bot API transport. Knows nothing about rfxh, CI, or captions --
// hand it a chat and a list of files and it sends.
//
// ponytail: ceiling is that the Bot API only speaks multipart/form-data, so
// FormData does the uploading no matter what. Upgrade path if Telegram ever
// grows a real upload endpoint: only `api()` changes.

const fs = require('fs');
const path = require('path');

const API = 'https://api.telegram.org/bot';
const ALBUM_MAX = 10; // hard server limit, not a style choice

// Telegram reads the filename off Content-Disposition, so a bare Blob is
// rejected with "there is no document in the request".
const file = (p) => new File([fs.readFileSync(p)], path.basename(p));

const form = (ctx, fields) => {
  const f = new FormData();
  f.set('chat_id', ctx.chat);
  for (const k in fields) f.set(k, fields[k]);
  return f;
};

const html = (text) => ({ text, parse_mode: 'HTML', disable_web_page_preview: 'true' });

// An attachment described by reference: the bytes go up as form field fN and
// the JSON points at them. Used for both a lone document and an album item.
const doc = (k) => ({ type: 'document', media: 'attach://f' + k });

// A notification must never take the build down with it.
async function api(ctx, method, body) {
  let json = null;
  try {
    const res = await fetch(API + ctx.token + '/' + method, { method: 'POST', body });
    json = await res.json();
  } catch (e) {
    ctx.core.warning('telegram ' + method + ' unreachable: ' + e.message);
    return null;
  }
  if (!json || !json.ok) {
    ctx.core.warning('telegram ' + method + ' failed: ' + JSON.stringify(json));
    return null;
  }
  return json;
}

const sendText = (ctx, text) => api(ctx, 'sendMessage', form(ctx, html(text)));

const editText = (ctx, text, messageId) =>
  api(ctx, 'editMessageText', form(ctx, Object.assign({ message_id: messageId }, html(text))));

function sendDocument(ctx, p, caption) {
  const fields = { document: file(p) };
  if (caption) Object.assign(fields, cap(caption));
  return api(ctx, 'sendDocument', form(ctx, fields));
}

// Documents spell the body `caption` where a message spells it `text`.
const cap = (text) => ({ caption: text, parse_mode: 'HTML' });

// Swaps a plain text message for a document, in place.
const editMedia = (ctx, p, caption, messageId) => api(ctx, 'editMessageMedia', form(ctx, {
  message_id: messageId,
  media: JSON.stringify(Object.assign(doc(0), cap(caption))),
  f0: file(p),
}));

// The files go up as an album under the caption. The caption is already edited
// into the start message, so nothing here carries one: an album only has room
// for one caption and that slot is better left empty. More files than the limit
// means more chunks; a chunk of one cannot be an album and goes up alone.
async function sendAlbum(ctx, files) {
  for (let i = 0; i < files.length; i += ALBUM_MAX) {
    const chunk = files.slice(i, i + ALBUM_MAX);
    if (chunk.length === 1) {
      await sendDocument(ctx, chunk[0]);
      continue;
    }
    const f = form(ctx, {});
    f.set('media', JSON.stringify(chunk.map((p, k) => {
      f.set('f' + k, file(p));
      return doc(k);
    })));
    await api(ctx, 'sendMediaGroup', f);
  }
}

module.exports = { sendText, editText, sendDocument, editMedia, sendAlbum, ALBUM_MAX };