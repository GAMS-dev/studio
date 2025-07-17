#!/bin/bash -x

set +e

## get Studio version information for artifact and tag name
STUDIO_MAJOR_VERSION=$(grep ^STUDIO_MAJOR_VERSION version | cut -f2 -d"=")
STUDIO_MINOR_VERSION=$(grep ^STUDIO_MINOR_VERSION version | cut -f2 -d"=")
STUDIO_PATCH_LEVEL=$(grep ^STUDIO_PATCH_LEVEL version | cut -f2 -d"=")

## change to artifact location and name
cd artifacts
mv $(find *arm_64.dmg) "GAMS_Studio-$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL-arm_64.dmg"
mv $(find *x86_64.dmg) "GAMS_Studio-$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL-x86_64.dmg"
mv $(find *AppImage) "GAMS_Studio-$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL-x86_64.AppImage"
mv $(find *zip) "GAMS_Studio-$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL-x86_64.zip"

## folder name which matches the current tag, like v0.10.2 or v0.11.0-rc
FOLDER_NAME="${TAG_NAME}"

## create GAMS Studio version directory
mkdir -p $FOLDER_NAME

## move artifacts to correct location
mv *.dmg *.AppImage *.zip $FOLDER_NAME

## URL to S3 studio
URL=${S3_URL}/studio/

## S3 upload to gams.com
s3cmd sync --acl-public ./ $URL --access_key=${S3_GAMSFILES_BUILDMEISTER_ACCESS_KEY} --secret_key=${S3_GAMSFILES_BUILDMEISTER_SECRET_KEY}

## S3 content listing
s3cmd ls -r $URL --access_key=${S3_GAMSFILES_BUILDMEISTER_ACCESS_KEY} --secret_key=${S3_GAMSFILES_BUILDMEISTER_SECRET_KEY}

## go to WORKSPACE for svn gams.com/studio.txt update
cd ..

## Update gams.com for final releases
if [[ ${TAG_NAME} =~ ^v[0-9]+\.[0-9]+\.[0-9]+.*$ ]]; then
    ## prepare GAMS Studio version line
    DATE=$(date +"%y%m%d")
    LINE="[$STUDIO_MAJOR_VERSION$STUDIO_MINOR_VERSION$STUDIO_PATCH_LEVEL.$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL,$DATE]"

    ## get Studio svn release list folder
    svn checkout --username ${SVN_USER} --password ${SVN_PASSWD} ${SVN_URL} release

    ## update studio.txt without duplicates
    if grep -Fxq $LINE release/studio.txt
    then
      echo "This version $LINE is already available in studio.txt"
    else
      printf "$LINE\\n$(cat release/studio.txt)" > release/studio.txt
    fi

    ## upload changed Studio release list
    cd release
    svn commit --username ${SVN_USER} --password ${SVN_PASSWD} -m "Jenkins GAMS Studio version update."
fi
