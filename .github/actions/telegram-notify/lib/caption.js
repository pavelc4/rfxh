// Builds the caption for a finished run: this run's per-target status and wall
// clock, fetched from the Actions job list. Called straight from the workflow
// so a step here does exactly one thing -- set the `caption` output.
//
// Set RELEASE_TAG in the step env to get a release link; a plain build run
// leaves it unset. BUILD_TYPE is the CMake build type the binaries were
// configured with, shown in the header so a caption says what it built.
const msg = require('./message.js');
const send = require('./send.js');

// The jobs that report on the build rather than being part of it.
const NOT_A_TARGET = new Set(['setup', 'notify', 'workflow_dispatch']);

const secs = (j) => j.started_at && j.completed_at
  // Null while a job is still in flight, which message.js renders as a dash
  // instead of a NaN duration.
  ? Math.max(0, Math.round((Date.parse(j.completed_at) - Date.parse(j.started_at)) / 1000))
  : null;

module.exports = async function caption(github, context, core) {
  const { owner, repo } = context.repo;
  const base = context.serverUrl + '/' + owner + '/' + repo;
  const runId = process.env.GITHUB_RUN_ID;

  const { data } = await github.rest.actions.listJobsForWorkflowRun({
    owner,
    repo,
    run_id: Number(runId),
  });

  const jobs = data.jobs
    .filter((j) => !NOT_A_TARGET.has(j.name))
    .map((j) => ({
      name: j.name.replace(/^build-/, ''),
      ok: j.conclusion === 'success',
      secs: secs(j),
    }))
    .sort((a, b) => a.name.localeCompare(b.name));

  const tag = process.env.RELEASE_TAG;

  core.setOutput('caption', msg.rich(Object.assign(
    send.readMeta({
      TG_RUN_NUMBER: process.env.GITHUB_RUN_NUMBER,
      TG_COMMIT_URL: base + '/commit/' + process.env.GITHUB_SHA,
      TG_WORKFLOW_URL: base + '/actions/runs/' + runId,
    }),
    { jobs, buildType: process.env.BUILD_TYPE },
    tag ? { release: tag, releaseUrl: base + '/releases/tag/' + tag } : null
  )));
};