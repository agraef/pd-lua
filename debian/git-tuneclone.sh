#!/bin/sh

## script to initialize a cloned repository
## with per (local) repository settings.

# - ignore quilt's .pc/ directory
# - enable the "--follow-tags" mode for pushing

error() {
 echo "$@" 1>&2
}

NAME=$(dpkg-parsechangelog -S Source)

if [ "x${NAME}" = "x" ]; then
 error "unable to determine package name"
 error "make sure you run this script within a source package dir"
 exit 1
fi

if [ ! -d ".git" ]; then
 error "it seems like this source package is not under git control"
 exit 1
fi

echo "tuning git-repository for ${NAME}"
git config push.followTags true && echo "enabled push.followTags"

GITEXCLUDE=".git/info/exclude"
egrep "^/?\.pc/?$" "${GITEXCLUDE}" >/dev/null 2>&1 \
  || (echo "/.pc/" >> "${GITEXCLUDE}" && echo "ignoring /.pc/")

for branch in pristine-tar upstream master; do
 git checkout "${branch}"
done
