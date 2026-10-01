// Every string the rfxh CI sends to Telegram. Pure text, no I/O -- so the
// wording can be changed without touching the transport or the workflow.

const esc = (s) =>
  String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');

const bold = (s) => '<b>' + s + '</b>';

const link = (href, text) => '<a href="' + esc(href) + '">' + esc(text) + '</a>';

// The two links every message ends with, one bold span around both.
const links = (m) => bold(link(m.commitUrl, 'Commit') + ' • ' + link(m.workflowUrl, 'Workflow'));

// <code> is what makes Telegram offer tap-to-copy, which is the only reason
// the commit subject is inside one. An empty <blockquote> still renders as a
// hole in the message, so an empty subject emits no markup at all.
const subject = (s) => (s ? '\n\n<blockquote>' + bold('<code>' + esc(s) + '</code>') + '</blockquote>' : '');

// A job that never finished has no duration, and printing NaN is worse than
// printing a dash.
const dur = (s) => (s == null ? '—'
  : s < 60 ? s + 's'
  : Math.floor(s / 60) + 'm' + (s % 60 ? ' ' + String(s % 60).padStart(2, '0') + 's' : ''));

const wall = (jobs) => jobs.reduce((a, j) => a + (j.secs || 0), 0);

const rows = (jobs) => jobs
  .map((j) => bold(j.name + '  ' + (j.ok
    ? dur(j.secs)
    : 'failed' + (j.secs == null ? '' : ' after ' + dur(j.secs)))))
  .join('\n');

// One line naming the run: what was built, which run, and in what build type.
const head = (m) => bold(
  (m.release ? 'rfxh ' + esc(m.release) + ' Released!  #ci_' : 'rfxh #ci_') + esc(m.runNumber)
  + (m.buildType ? '  ' + esc(m.buildType) : ''));

// The run header, plus the release link directly under it when there is one.
const top = (m) => head(m) + (m.release ? '\n' + bold(link(m.releaseUrl, 'GitHub Release')) : '');

// Every caption ends the same way.
const tail = (m) => '\n\n' + links(m);

// Start message: header, subject, links. No job list exists yet.
const ok = (m) => top(m) + subject(m.subject) + tail(m);

// Failure: the header still identifies the run, then the bad news.
const failed = (m) => top(m) + '\n\n' + bold('FAILED') + subject(m.subject) + tail(m);

// The finished report. Per-target status, the summed wall clock as a total, then
// the links. This is the caption the start message gets edited into, so it is
// the only text the user reads for the whole run.
const rich = (m) => {
  const jobs = m.jobs || [];
  return top(m) + subject(m.subject) + '\n\n' + rows(jobs) + '\n\n' + bold(dur(wall(jobs))) + tail(m);
};

module.exports = { esc, bold, subject, head, links, dur, wall, rows, ok, failed, rich };