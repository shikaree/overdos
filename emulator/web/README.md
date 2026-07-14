# ÖverDOS in the browser

[`overdos.html`](overdos.html) is a **self-contained** port of the same
emulator to JavaScript + Canvas. The MIPS machine, the VT100 terminal, and the
ÖverDOS boot image (embedded as base64) are all in that single file — no build
step, no dependencies, no network access.

## Run

Just open `overdos.html` in a modern browser.

Some browsers restrict `file://` pages; if it doesn't start, serve it over HTTP
with the tiny bundled server (needs [Node.js](https://nodejs.org/)):

```sh
node server.js        # serves http://localhost:8099/overdos.html
```

## Controls

Same as the native emulator: type `ls` to list programs, run one by name, and
use **`` ` ``** (then any key) to switch keyboard focus between processes. The
`å ä ö` keys work in both directions.
