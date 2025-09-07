# xwinfocus

[![Release](https://img.shields.io/github/release/omilevskyi/xwinfocus.svg)](https://github.com/omilevskyi/xwinfocus/releases/latest)
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://raw.githubusercontent.com/omilevskyi/xwinfocus/refs/heads/main/LICENSE)
[![Build](https://github.com/omilevskyi/xwinfocus/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/omilevskyi/xwinfocus/actions/workflows/build.yml)
[![Cppcheck](https://github.com/omilevskyi/xwinfocus/actions/workflows/cppcheck.yml/badge.svg?branch=main)](https://github.com/omilevskyi/xwinfocus/actions/workflows/cppcheck.yml)
[![Powered By: nFPM](https://img.shields.io/badge/Powered%20by-nFPM-green.svg)](https://github.com/goreleaser/nfpm)

A lightweight X11 utility to focus an existing window by its **name** (`XClassHint.res_name`) and/or **class** (`XClassHint.res_class`), or launch a fallback command if the window is not found. Optionally, it can wait for a specified time and then activate a window. It also supports toggling back to the previously active window. It can list all client windows (ID / Name / Class).

> Works in X11 sessions (including Wayland via XWayland). Does **not** work on pure Wayland without XWayland.

This tiny tool was written alongside AI and was inspired by the following script:

```sh
#!/bin/sh

if [ -z "${1}" ]; then
 echo "Usage: xwinfocus.sh name.class what_run_if_non_existing" >&2
 exit 1
fi

WIN_ID=$(wmctrl -lx | awk '$3 == "'"${1}"'" { print $1; exit }')
test -z "${WIN_ID}" || exec wmctrl -ia "${WIN_ID}"
test -n "${2}" || exit 2
shift
exec ${@} &
```

## Verification of the authenticity

```sh
export VERSION=0.0.2
cosign verify-blob \
  --certificate-identity https://github.com/omilevskyi/xwinfocus/.github/workflows/release.yml@refs/heads/main \
  --certificate-oidc-issuer https://token.actions.githubusercontent.com \
  --certificate https://github.com/omilevskyi/xwinfocus/releases/download/v${VERSION}/CHECKSUM.sha256.pem \
  --signature https://github.com/omilevskyi/xwinfocus/releases/download/v${VERSION}/CHECKSUM.sha256.sig \
  https://github.com/omilevskyi/xwinfocus/releases/download/v${VERSION}/CHECKSUM.sha256
```
