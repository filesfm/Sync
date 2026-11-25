#!/bin/sh

# this script replaces the line
#  appname = 'Nextcloud'
# with the correct branding name in the syncstate.py script
# It also replaces the occurrences in the class name so several
# branding can be loaded (see #6524)
# The replacement also lands in Python class names (e.g. MenuExtension_Nextcloud),
# so it must be a valid identifier fragment - strip anything that isn't a letter,
# digit or underscore rather than using the display name as-is.
safe_name=$(printf '%s' "$1" | sed -e 's/[^A-Za-z0-9_]/_/g')
sed -i.org -e "s/Nextcloud/$safe_name/g" syncstate.py
