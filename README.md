# xwinfocus

[![Release](https://img.shields.io/github/release/omilevskyi/xwinfocus.svg)](releases/latest)
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://raw.githubusercontent.com/omilevskyi/xwinfocus/refs/heads/main/LICENSE)
[![Build](https://github.com/omilevskyi/xwinfocus/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/omilevskyi/xwinfocus/actions/workflows/build.yml)
[![Cppcheck](https://github.com/omilevskyi/xwinfocus/actions/workflows/cppcheck.yml/badge.svg?branch=main)](https://github.com/omilevskyi/xwinfocus/actions/workflows/cppcheck.yml)
[![Powered By: nFPM](https://img.shields.io/badge/Powered%20by-nFPM-green.svg)](https://github.com/goreleaser/nfpm)

Activate or list X11 windows, or run fallback command

This is a tiny tool written alongside AI that does almost the same as the following script:

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
