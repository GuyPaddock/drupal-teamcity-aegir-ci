# drupal-teamcity-aegir-ci
Resources for users setting up a Drupal Continuous Integration environment using Aegir and JetBrains TeamCity.

## Watch the Video
Before exploring the resources in this repo, you will want to watch the step-by-step video by Guy Paddock of [Red Bottle Design](http://redbottledesign.com). You can find the video here:

https://www.youtube.com/watch?v=oBdVFyRifc4

## What's Included
- **steps.txt**: A description of the steps involved in setting up the CI environment, along with miscellaneous notes for each step of the process.
- **teamcity/sudoers.d/teamcity**: A sudoers file for TeamCity to be able to kick-off Drush tasks as the `aegir` user.
- **teamcity/init.d/teamcity-agent**: An init script for starting the TeamCity agent at system boot.
- **teamcity/build-types/drupal_ci.xml**: A complete TeamCity project template for setting-up new Drupal CI projects in TeamCity in minutes.
