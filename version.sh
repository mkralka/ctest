#!/bin/sh
## Generate the version string for this build.
##
## usage:
##    version.sh RELEASE_TAG_PREFIX OFFICIAL_VERSION
##

rel_tag_prefix="$1"
version="$2"

# If the git command or repo is not found, must be from a dist tarball
# (so there is no revision)
if ! git --help > /dev/null 2>&1; then
	echo "$version"
	exit 0
fi
if test "x$(git rev-parse --is-inside-work-tree)" != "xtrue"; then
	echo "$version"
	exit 0
fi

# Look for the most recent release tag. If the release in that tag is the
# same as the version we're checking for
if ref="$(git describe --match $rel_tag_prefix\* --all --long 2> /dev/null)"; then 
	tagged_version="$(echo "$ref" | sed "s|\(tags/\)\{0,1\}$rel_tag_prefix\(.*\)-[^-]\{1,\}-g[^-]\{1,\}$|\2|")"
	commits_since_tag="$(echo "$ref" | sed "s|\(tags/\)\{0,1\}$rel_tag_prefix.*-\([^-]\{1,\}\)-g[^-]\{1,\}$|\2|")"
else
	# No tags. Assume nothing as been released.
	tagged_version=''
	commits_since_tag="$(git rev-list HEAD --count)"
fi

# Look for *any* dirty/staged files.
# Untracked files are less important, because untracked files can only
# affect the build if there is a dirty file (adding them to a makefile) or
# because they were not added to the project. Assume the latter never happens.
dirty=false
if ! git diff-files --quiet --ignore-submodules --; then
	# unstaged files
	dirty=true
elif ! git diff-index --cached --quiet HEAD --ignore-submodules --; then
	# stagged, but uncommitted changes
	dirty=true
fi


# If the tag matches the official version and there have been no changes, then
# it's safe to use the official version.
if test "x$tagged_version" = "x$version" -a $commits_since_tag -eq 0 && ! $dirty; then
	echo "$version"
	exit 0
fi

# If the tag matches the official version but there have been changes, the
# official version is out of date. Abort!
# adding a new segment.
if test "x$tagged_version" = "x$version"; then
	echo "ERROR: The version in configure.ac is the same as the tagged version," >&2
	echo "ERROR: but there are changes. Update configure.ac." >&2
	exit 1
fi

revision="-beta.$commits_since_tag"

if $dirty; then
	revision="$revision.$(date -u '+%Y%m%d.%H%M%S')"
fi

echo "$version$revision"
