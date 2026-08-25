#!/bin/sh -xe

[ "$#" -lt 2 ] && echo "Usage: sign_app.sh <app> <identity> <team_identifier>" && exit

src_app="$1"
identity="$2"
team_identifier="$3"

codesign -s "$identity" --force --preserve-metadata=entitlements --verbose=4 --deep "$src_app"

# Verify the signature
codesign -dv "$src_app"
codesign --verify -v "$src_app"

# Informational only: spctl's Gatekeeper exec assessment rejects apps
# signed with an "Apple Distribution" cert (App Store submission identity)
# since they're neither Developer ID-signed nor notarized/store-approved.
# That's expected here and doesn't indicate a broken signature -- the
# codesign --verify above already confirmed the signature itself is valid.
spctl -a -t exec -vv "$src_app" || true

# Validate that the key used for signing the binary matches the expected TeamIdentifier
# needed to pass the SocketApi through the sandbox
codesign -dv "$src_app" 2>&1 | grep "TeamIdentifier=$team_identifier"
exit $?