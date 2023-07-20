#!/bin/bash -x

set +e

## get Studio version information for artifact and tag name
STUDIO_MAJOR_VERSION=$(grep ^STUDIO_MAJOR_VERSION version | cut -f2 -d"=")
STUDIO_MINOR_VERSION=$(grep ^STUDIO_MINOR_VERSION version | cut -f2 -d"=")
STUDIO_PATCH_LEVEL=$(grep ^STUDIO_PATCH_LEVEL version | cut -f2 -d"=")

## get GAMS version information for artifact, tag name and description
export GAMS_DISTRIB_MAJOR_VERSION=$(grep ^GAMS_DISTRIB_MAJOR version | cut -f2 -d"=")

## get changelog content
CHANGELOG=""
foundFirst="false"
while IFS="" read -r line
do
    if [[ $line =~ ^Version.* ]] && [[ $foundFirst == "true" ]]; then
        break
    elif [[ $line =~ ^Version.* ]]; then
        foundFirst="true"
    elif [[ $line =~ ^=.* ]]; then
    	 continue
    else
        CHANGELOG+=$line$'\n'
    fi
done < CHANGELOG

## GitHub parameters
GITHUB_ORGA=GAMS-dev
GITHUB_REPO=studio

## delete old release if needed
RELEASE_DELETED=0
github-release delete --user $GITHUB_ORGA --repo $GITHUB_REPO --tag ${TAG_NAME} && RELEASE_DELETED=0 || RELEASE_DELETED=1
if [[ $RELEASE_DELETED -eq 0 ]]; then
  echo "Release deleted"
else
  echo "No release to delete"
fi

## create a GitHub release
if [[ ${TAG_NAME} =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    github-release release --user $GITHUB_ORGA --repo $GITHUB_REPO --tag ${TAG_NAME} \
--description "${CHANGELOG}

This version of GAMS Studio requires GAMS ${GAMS_DISTRIB_MAJOR_VERSION}. To download GAMS, please visit https://www.gams.com/latest/. To learn more about GAMS Studio, please visit https://www.gams.com/latest/docs/T_STUDIO.html"
elif [[ ${TAG_NAME} =~ ^v[0-9]+\.[0-9]+\.[0-9]+-rc\.?[0-9]*$ ]]; then
    github-release release --user $GITHUB_ORGA --repo $GITHUB_REPO --tag ${TAG_NAME} --pre-release \
--description "${CHANGELOG}

This version of GAMS Studio requires GAMS ${GAMS_DISTRIB_MAJOR_VERSION}. To download GAMS, please visit https://www.gams.com/latest/. To learn more about GAMS Studio, please visit https://www.gams.com/latest/docs/T_STUDIO.html"
fi

## upload artifacts to GitHub
if [[ ${TAG_NAME} =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    cd artifacts
    mv $(find *arm_64.dmg) "GAMS_Studio-$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL-arm_64.dmg"
    mv $(find *x86_64.dmg) "GAMS_Studio-$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL-x86_64.dmg"
    mv $(find *AppImage) "GAMS_Studio-$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL-x86_64.AppImage"
    mv $(find *zip) "GAMS_Studio-$STUDIO_MAJOR_VERSION.$STUDIO_MINOR_VERSION.$STUDIO_PATCH_LEVEL-x86_64.zip"
    sleep 16
    parallel github-release --verbose upload -R --user $GITHUB_ORGA --repo $GITHUB_REPO --tag ${TAG_NAME} --name {} --file {} ::: *.*
fi
