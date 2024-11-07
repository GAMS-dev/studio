#!/bin/bash

QT_VERSION_SHORT=$(echo ${QT_VERSION} | grep -oP "\d+\.\d+")
QT_UPLOADED=$(s3cmd ls -r ${S3_URL}/qt/ --access_key=${S3_ACCESS_KEY} --secret_key=${S3_SECRET_KEY} | grep -o qt-everywhere-src-${QT_VERSION}.tar.xz)

## Upload Qt everywhere package
if [[ "$QT_UPLOADED" == "" ]]; then
    echo "Uploading qt-everywhere-src-${QT_VERSION}.tar.xz..."
    mkdir qt && cd qt
    curl -LO https://download.qt.io/archive/qt/$QT_VERSION_SHORT/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.tar.xz
    ## S3 Put Qt everywhere package
    s3cmd --access_key=${S3_ACCESS_KEY} --secret_key=${S3_SECRET_KEY} put -P qt-everywhere-src-${QT_VERSION}.tar.xz ${S3_URL}/qt/qt-everywhere-src-${QT_VERSION}.tar.xz
    cd ..
else
    echo "Skipped qt-everywhere-src-${QT_VERSION}.tar.xz upload... already available"
fi

## Update README.md Qt version
git clone https://${GITLAB_USER}:${GITLAB_CI_UPDATE_TOKEN}@git.gams.com/devel/studio.git studio
cd studio
README="README.md"
git config user.name ${GITLAB_USER}
git config user.email ${GITLAB_USER_EMAIL}
git checkout $CI_COMMIT_REF_NAME
perl -pi -e "s/\d+\.\d+\.\d+\.tar\.xz/${QT_VERSION}\.tar\.xz/g" $README

## README.md commit and push
CHANGES=$(git diff --name-only $README)
if [[ "$CHANGES" == "" ]]; then
    echo "NO CHANGES... NOTHING TO PUSH"
else
    echo "update $README Qt everywhere package link"
    git add $README
    git commit -m "pipeline updated $README Qt everywhere package link"
    # prevent ci run via -o ci.skip
    git push -o ci.skip
fi
