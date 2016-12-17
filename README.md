# Resource Kit for Setting up a Drupal CI Environment with Aegir and TeamCity
This repository contains resources for users setting up a Drupal Continuous Integration environment using Aegir and JetBrains TeamCity.

## Watch the Video
Before exploring the resources in this repo, you will want to watch the step-by-step video by Guy Paddock of [Red Bottle Design](http://redbottledesign.com). You can find the video here:

https://www.youtube.com/watch?v=oBdVFyRifc4

## What's Included
- **steps.md**: A description of the steps involved in setting up the CI environment, along with miscellaneous notes for each step of the process.
- **teamcity/sudoers.d/teamcity**: A sudoers file for TeamCity to be able to kick-off Drush tasks as the `aegir` user.
- **teamcity/init.d/teamcity-agent**: An init script for starting the TeamCity agent at system boot.
- **teamcity/build-types/drupal_ci.xml**: A complete TeamCity project template for setting-up new Drupal CI projects in TeamCity in minutes.
- **drupal/profiles/sample**: A sample installation profile you can use as a base for new Drupal CI projects.

## Background
Not many things are as important to an agile development team as continuous integration—the concept of merging-in and testing fresh code from developers as soon as the code has been checked in, to catch defects and head off barriers to the team as soon as possible.

With Drupal, CI can sometimes be challenging. Fortunately, there are many tools out there that can help. In fact, there are a lot of tutorials out there for setting up a CI server with Jenkins or Hudson and Aegir. Unfortunately, there are surprisingly few out there for TeamCity, a little-known product from JetBrains that features a free tier and works exceptionally well for Drupal development.

Guy Paddock of RedBottle Design in Rochester presented at the WNYDUG meeting on March 25th, 2015, where he covered how you can set up a CI server from end-to-end—from source control to deployment—using easy-to-maintain Installation Profiles, Aegir, and TeamCity. Guy also covered several modules that are helpful for handling everything from site configuration to content migration/content import.
