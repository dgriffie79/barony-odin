#!/usr/bin/env node
// unwrap_aliases.js — replace Entity/Stat reference members (T& name) with
// inline reference-returning getters, and rewrite member-access call sites
// (`->name` / `.name`) to call the getter.
//
// Getter: `inline T& name() { return backing[idx]; }`
//         `inline const T& name() const { return backing[idx]; }`
//
// gateVelZ is a special case (backs onto a real member `vel_z`, not an array).
//
// Bare references (`name` with implicit `this->`) are intentionally NOT
// rewritten here: the compiler will flag the ones that need `this->name()`,
// which avoids corrupting the ~16 local variables that shadow alias names.
//
// Run: node tools/unwrap_aliases.js

const fs = require('fs');
const path = require('path');

const MAP = JSON.parse(fs.readFileSync('/tmp/alias_map.json','utf8'));

const cppType = (t) => {
  switch(t){
    case 'Sint32': case 'Uint32': case 'int': return 'int';
    case 'bool': return 'bool';
    case 'float': return 'float';
    case 'double': case 'real_t': return 'real_t';
    default: return t;
  }
};

function getterDef(name, b){
  const T = cppType(b.type);
  const expr = b.direct ? b.arr : `${b.arr}[${b.idx}]`;
  return [
    `\tinline ${T}& ${name}() { return ${expr}; }`,
    `\tinline const ${T}& ${name}() const { return ${expr}; }`,
  ];
}

// --- 1. rewrite entity.hpp / stat.hpp: remove member decls, add getters ---
function transformHeader(file, owner){
  let txt = fs.readFileSync(file,'utf8');
  const lines = txt.split('\n');

  // collect getters for THIS owner, in declaration order
  const getters = [];
  const seen = new Set();
  for(const l of lines){
    const m = l.match(/^\s*(Sint32|Uint32|int|bool|real_t|float|double)\s*&\s*(\w+)\s*(?:=\s*([A-Za-z_]\w*)\s*\[\s*(\d+)\s*\])?\s*;/);
    if(m && MAP[m[2]] && MAP[m[2]].owner === owner && !seen.has(m[2])){
      seen.add(m[2]);
      getters.push(...getterDef(m[2], MAP[m[2]]));
    }
  }

  // rebuild: drop member lines, inject getters once before first removed line
  const out = [];
  let injected = false;
  for(const l of lines){
    const m = l.match(/^\s*(Sint32|Uint32|int|bool|real_t|float|double)\s*&\s*(\w+)\s*(?:=\s*([A-Za-z_]\w*)\s*\[\s*(\d+)\s*\])?\s*;/);
    if(m && MAP[m[2]] && MAP[m[2]].owner === owner){
      if(!injected){
        out.push(getters.join('\n'));
        injected = true;
      }
      continue;
    }
    out.push(l);
  }
  fs.writeFileSync(file, out.join('\n'));
}

// --- 2. remove ctor init-list entries ---
function transformCtor(file){
  let txt = fs.readFileSync(file,'utf8');
  const lines = txt.split('\n');
  const out = [];
  for(const l of lines){
    // array-backed: name(arr[idx]),  — optionally trailing comma
    let m = l.match(/^\s*(\w+)\s*\(\s*(skill|fskill|MISC_FLAGS)\s*\[\s*(\d+)\s*\]\s*\)\s*,?\s*$/);
    // direct-member: name(vel_z),
    let m2 = l.match(/^\s*(\w+)\s*\(\s*vel_z\s*\)\s*,?\s*$/);
    if((m && MAP[m[1]] && !MAP[m[1]].direct) || (m2 && MAP[m2[1]] && MAP[m2[1]].direct)){
      continue; // drop the line
    }
    out.push(l);
  }
  fs.writeFileSync(file, out.join('\n'));
}

// --- 3. rewrite member-access call sites in .cpp ---
function transformCpp(file){
  let txt = fs.readFileSync(file,'utf8');
  const orig = txt;
  for(const name of Object.keys(MAP)){
    // ->name  (allow whitespace after ->)
    const reA = new RegExp('(->\\s*)('+name+')\\b','g');
    txt = txt.replace(reA, '$1$2()');
    // .name (member via object) but NOT ->name, NOT an existing call .name(
    const reD = new RegExp('(\\.\\s*)('+name+')\\b(?!\\s*\\()','g');
    txt = txt.replace(reD, '$1$2()');
  }
  if(txt !== orig) fs.writeFileSync(file, txt);
}

transformHeader('src/entity.hpp', 'entity');
transformHeader('src/stat.hpp', 'stat');
transformCtor('src/entity_shared.cpp');
transformCtor('src/stat_shared.cpp');

const cpps = [];
(function walk(d){
  for(const e of fs.readdirSync(d,{withFileTypes:true})){
    const p = path.join(d,e.name);
    if(e.isDirectory()){ if(!/builddir|\.git|\.pi/.test(p)) walk(p); }
    else if(/\.cpp$/.test(e.name)) cpps.push(p);
  }
})('src');
for(const f of cpps) transformCpp(f);

console.log('done: headers rewritten, ctor init-lists trimmed, call sites updated');
