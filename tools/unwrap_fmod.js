#!/usr/bin/env node
// unwrap_fmod.js — collapse USE_FMOD / USE_OPUS preprocessor branches in the
// Barony C++ tree, choosing the OPENAL (or non-FMOD) side of every branch.
//
// The build defines USE_OPENAL and NOT USE_FMOD/USE_OPUS, so this is a pure
// dead-branch removal: for every conditional that involves USE_FMOD/USE_OPUS,
// keep the branch that is taken when USE_FMOD/USE_OPUS are undefined.
//
// RULES (deterministic, line-oriented):
//   * target guards:  #ifdef USE_FMOD, #if defined(USE_FMOD) [| ...],
//     #ifndef USE_FMOD, #if !defined(USE_FMOD), #ifdef USE_OPUS,
//     #if defined(USE_OPUS)
//   * any #if line whose condition mentions USE_FMOD or USE_OPUS is a target.
//     Conditions may combine with USE_OPENAL (e.g. `USE_FMOD || USE_OPENAL`)
//     or EDITOR. We never touch conditions that only mention USE_OPENAL alone.
//   * eval target conditions with USE_FMOD=false, USE_OPUS=false,
//     USE_OPENAL=true, EDITOR=true (game build) — the latter two only matter
//     for the rare mixed conditions; the result is the same on the OPENAL path.
//   * collapse: keep the evaluated-true arm; drop the evaluated-false arm and
//     the guard lines. For `#ifdef USE_FMOD` (false) with an `#else`, keep the
//     else arm. With no else, drop the whole block.
//   * #if defined(USE_FMOD) || defined(USE_OPENAL) => true (OPENAL is defined)
//     => keep the body (both arms identical in these cases, but keep body).
//   * `#elif` chains are handled recursively over each arm.
//   * nested unrelated guards are preserved untouched.
//
// Run:  node tools/unwrap_fmod.js src/...
// (pass explicit file list; safe to re-run — no-op when no target guards remain)

const fs = require('fs');
const path = require('path');

function parseCond(expr) {
  // Evaluate with the live config. Only these macros are known; anything
  // else fails conservative (arm treated as false).
  let e = expr;
  e = e.replace(/\bUSE_FMOD\b/g, 'false');
  e = e.replace(/\bUSE_OPUS\b/g, 'false');
  e = e.replace(/\bUSE_OPENAL\b/g, 'true');
  e = e.replace(/\bEDITOR\b/g, 'true');
  e = e.replace(/\bdefined\s*\(\s*(false|true)\s*\)/g, '$1');
  e = e.replace(/\bdefined\b/g, '');
  // After substitution the expression must reduce to true/false/&&/||/!/()/space.
  if (!/^[\s()!&|a-z]*$/i.test(e)) {
    return { ok: false };
  }
  try {
    // eslint-disable-next-line no-new-func
    const v = new Function(`return (${e});`)();
    return { ok: true, value: !!v };
  } catch (err) {
    return { ok: false };
  }
}

function classifyDirective(line) {
  // Normalizes #ifdef / #ifndef into #if with a synthesized defined(...) cond.
  // returns {kind:'if'|'elif'|'else'|'endif', cond} or null
  line = line.replace(/\r$/,'');
  const m = line.match(/^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$/);
  if (!m) return null;
  const raw = m[1];
  let kind = raw;
  let cond = m[2].trim();
  if (raw === 'ifdef') {
    kind = 'if';
    cond = `defined(${cond.split(/\s/)[0]})`;
  } else if (raw === 'ifndef') {
    kind = 'if';
    cond = `!defined(${cond.split(/\s/)[0]})`;
  } else if (raw === 'else' || raw === 'endif') {
    cond = '';
  } else if (raw === 'elif') {
    kind = 'elif';
  }
  return { kind, cond };
}

// Split a block into arms. Returns [{armCond, lines:[...]}] where armCond is
// null for else. Nested blocks inside arms are kept as-is (their directives
// are preserved), because we only collapse top-level target conditionals.
function splitArms(lines, startIdx) {
  // lines: the directive `#if ...` is at startIdx. Returns list of arms and
  // the index just past the matching #endif (exclusive).
  const arms = [];
  let cur = { cond: null, lines: [] };
  let depth = 0;
  let first = true;
  let i = startIdx;
  for (; i < lines.length; i++) {
    const l = lines[i];
    const d = classifyDirective(l);
    if (first) {
      // the opening #if
      cur.cond = d.cond;
      first = false;
      depth = 1;
      continue;
    }
    if (d) {
      if (d.kind === 'if' && (/(USE_FMOD|USE_OPUS)/.test(l) || /^#\s*if(def|ndef)?\s+USE_(FMOD|OPUS)/.test(l))) {
        // nested target #if: keep as an opaque nested block by recursing later.
        // For now treat like any directive and increment depth.
        depth++;
        cur.lines.push(l);
        continue;
      } else if (d.kind === 'if') {
        depth++;
        cur.lines.push(l);
        continue;
      } else if (d.kind === 'elif') {
        if (depth === 1) {
          arms.push(cur);
          cur = { cond: d.cond, lines: [] };
          continue;
        } else {
          cur.lines.push(l);
          continue;
        }
      } else if (d.kind === 'else') {
        if (depth === 1) {
          arms.push(cur);
          cur = { cond: null, lines: [] };
          continue;
        } else {
          cur.lines.push(l);
          continue;
        }
      } else if (d.kind === 'endif') {
        depth--;
        if (depth === 0) {
          arms.push(cur);
          return { arms, endIdx: i + 1 };
        } else {
          cur.lines.push(l);
          continue;
        }
      }
    }
    cur.lines.push(l);
  }
  return { arms, endIdx: lines.length };
}

function evalArm(cond) {
  if (cond === null) return true; // else arm
  const p = parseCond(cond);
  return p.ok && p.value;
}

// Collapse target conditionals in a single buffer. Returns new text.
function collapse(text) {
  const lines = text.split('\n');
  const out = [];
  let i = 0;
  while (i < lines.length) {
    const l = lines[i];
    const d = classifyDirective(l);
    if (d && (d.kind === 'if') && /(USE_FMOD|USE_OPUS)/.test(l)) {
      // is this a target? only if the condition mentions the macros (already true)
      const { arms, endIdx } = splitArms(lines, i);
      // choose the first true arm
      let chosen = null;
      for (const arm of arms) {
        if (evalArm(arm.cond)) { chosen = arm; break; }
      }
      if (chosen) {
        // recurse into the chosen arm to collapse nested targets
        const nested = collapse(chosen.lines.join('\n'));
        out.push(nested);
      }
      i = endIdx;
      continue;
    }
    out.push(l);
    i++;
  }
  return out.join('\n');
}

for (const f of process.argv.slice(2)) {
  const txt = fs.readFileSync(f, 'utf8');
  const out = collapse(txt);
  if (out !== txt) {
    fs.writeFileSync(f, out);
    console.log('rewrote ' + f);
  }
}
