#!/bin/bash

## Setup studio repository
git clone https://${GITLAB_USER}:${GITLAB_CI_UPDATE_TOKEN}@git.gams.com/devel/studio.git studio
cd studio
git config user.name ${GITLAB_USER}
git config user.email ${GITLAB_USER_EMAIL}
git checkout $CI_COMMIT_REF_NAME
cd ..

## Update copyright and license
git clone https://${GITLAB_USER}:${GITLAB_CI_UPDATE_TOKEN}@git.gams.com/afust/license-update-script.git script
./script/update-license.py -p $CI_PROJECT_DIR/studio/src --studio-license
./script/update-license.py -p $CI_PROJECT_DIR/studio/platform --studio-license

## Commit and push updates
cd studio
CHANGES=$(git diff --name-only)
if [[ "$CHANGES" == "" ]]; then
    echo "NO CHANGES... NOTHING TO PUSH"
else
    echo "UPDATE copyright, license"
    git add -u
    git commit -m "pipeline update copyright, license"
    # prevent ci run via -o ci.skip
    git push -o ci.skip
fi