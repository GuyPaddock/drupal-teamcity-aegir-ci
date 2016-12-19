# Drupal Distribution Packaging and Deployment Extensions for Drush
This folder contains two Drush extensions to streamline a CI-based distribution
packaging process:

- `pantheon_autodeploy` - Automates the process of deploying the results of
  assembling an installation profile to a Pantheon-based Drupal environment.
- `distro_versioner` - Automates the process of setting the version number in
  all custom modules, features, and themes that are part of a distribution to
  match the version number of the distribution.

Neither extension requires TeamCity or Aegir to function, but fits nicely with
a TeamCity-based Aegir CI environment like that configured using steps elsewhere
in this repository.

## Installation of the Extensions
Both extensions must be deployed together. To deploy them, simply copy the
contents of this folder into the Drush search path
(`~/.drush/pantheon_autodeploy` or
`/usr/share/drush/commands/pantheon_autodeploy` are good options).

_If you will be using Pantheon Auto-deploy, it has additional requirements, as
discussed below._

## Pantheon Auto-deploy
This extension works by:
1. Cloning the Pantheon site repo into a local temporary folder.
2. Copying the appropriate files from the platform in which Drush is being run
   into the site repo.
3. Committing the changes, with an option to tag the commit with the version of
   the distribution.
4. Automatically pushing the code through the Pantheon workflow
  (dev -> test -> live) as desired.

### Additional Requirements
If you will be using Pantheon Auto-deploy, you must ensure that the following
commands are also installed on the server and accessible to the account
performing deployments (e.g. `aegir` or `teamcity`):
- [Drush](http://www.drush.org/en/master/) (of course)
- [GIT](https://git-scm.com/)
- [Pantheon Terminus CLI](https://github.com/pantheon-systems/terminus)
   (Pantheon Auto-deploy was tested with version 0.13.6).

Additionally, ensure that you:
1. Have configured SSH key-based authentication for accessing the site's
   Pantheon GIT repository from the user account _that will be running
   deployments_; AND
2. Have authenticated with Terminus from the user account _that will be running
   deployments,_ including assigning a machine key.

### How to Use
Configuration can come from the command-line or be specified in the `.info` file
for the distribution.

After deploying the extension, run `drush help pad` and look at `sample.info` in
this repository for more details on how to use.

### Integrating with CI
When using Pantheon Auto-deploy as part of a CI process, make sure that it is
the last step of the build. You will likely find the `--use-info-for-branch`,
`--source-build-number`, and `--source-commit-hash` options useful for ease
of configuration and traceability.

Here is a common configuration for deployment in a profile `.info` file:
```
; <developmentOnly>
;===============================================================================
; Pantheon Auto-deploy settings
;===============================================================================
autodeploy[defaults][target][site-id] = "SITE-UUID-FROM-PANTHEON"
autodeploy[defaults][tag-after-commit] = false

autodeploy[branches][develop][target][environment] = dev

autodeploy[branches][release/.*][target][environment] = test

autodeploy[branches][master][target][environment] = live
autodeploy[branches][master][tag-after-commit] = true
; </developmentOnly>
```

This maps the popular [GIT Flow](https://github.com/nvie/gitflow) branch
management strategy to Pantheon, as follows:
 - Everything under `autodeploy[defaults]` sets default values which can be
   overridden by branch-specific configurations. This means:
   - `autodeploy[defaults][target][site-id]` sets the ID of the default site
    on Pantheon to use as the target for deployments.

   - `autodeploy[defaults][tag-after-commit]` configures Pantheon Auto-deploy
     to, by default, not to tag deployments with the version of the
     distribution.

     This is a safe default because it ensure that there are no
     GIT errors from trying to tag different commits with the same version.

   - The `develop` branch is mapped to the `dev` environment on Pantheon, so
     any changes commited to `develop` will automatically get deployed to the
     development environment.

   - All release branches (e.g. `release/1.0`, `release/1.3`, etc.) are mapped
     to the `test` environment on Pantheon, so any changes committed during the
     release process will automatically get deployed to the development and test
     environments (per Pantheon's required workflow).

   - The `master` branch is mapped to the `live` environment on Pantheon, so
     releases merged into `master` are automatically deployed into the live
     production environment at the conclusion of the release cycle, and tagged
     on Pantheon with the release version. As expected, the releases will get
     deployed to the development and test environments first before being
     deployed live.

     **NOTE: Deploying to a live environment automatically can be dangerous.
     See the note that follows about back-ups and best practices for
     deployments.**

   - The `<developmentOnly>` tags cause this section to be stripped when
     releasing the distribution, since it's not publicly useful information.

### A Note About Back-ups
Automatic deployments are an inherently risky prospect, especially if
deployments are automatically made to the live production environment.
Therefore, we strongly encourage you to only use Pantheon Auto-deploy to push to
the "dev" and "test" environments, leaving actual deployment to the "live"
environment to a trained professional using the Pantheon web interface.

At a minimum, we strongly encourage you to enable automatic back-ups on
Pantheon and ensure that they happen frequently, to minimize the impact of a
bad deployment.

That said, Pantheon Auto-deploy is offered WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Distro Version-er
This extension automates the often-tedious search-and-replace task of updating
the version numbers in all modules, features, and themes that need to be
included in a distribution. On Drupal.org, this happens automatically when a
distro is being packaged, but you don't have that luxury when you're rolling
your own distribution for internal consumption.

### How to Use
- Update all of the `.info` files in the installation profile so that instead
  of specifying a version number, they instead specify `PROFILE_VERSION`.

- Ensure that a version number is specified in the `.info` file of the
  installation profile.

- Assemble the distribution (i.e. use `drush make` to build a Drupal platform
  from the installation profile).

- Run `drush vdst <profile_name>` from within the root of the assembled
  distribution (i.e. the root of the Drupal platform). This will automatically
  update all `.info` files under the profile named `<profile_name>` in the
  distribution to match the version number of the installation profile itself.

Modules under folders called `contrib/` and `libraries/` are automatically
excluded from the replacement operation.

The search string that the extension looks for (normally, `PROFILE_VERSION`) can
be set to something else using the `--replace-string` option.
