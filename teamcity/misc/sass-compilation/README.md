# Sample Build Step for SASS Compilation
If you are building custom themes for Drupal that use SASS for styling, you may
find it useful to have TeamCity compile SASS into CSS for you automatically
during CI builds instead of having to remember to check-in updated CSS files
each time you modify SASS files. This also helps to catch errors in SASS code
or identify missing image resources sooner.

The included `compile-sass.sh` sample illustrates how to do this. Copy the
contents of the file into a new build step in the "Drupal CI" build-type of the
Root project in TeamCity. The new build step should be first, so it executes
**before** the step entitled
"Assemble installation profile platform and register it with Aegir". _This
assumes you've already set-up Drupal CI using the steps in the root of this
repository._

The same is able to compile SASS code using either the older Ruby-based
`compass` package invoked using `grunt`, or the newer `libSass` package invoked
using `Gulp`. Each SASS theme must contain a folder called `sass/` in order to
be detected.

The conventions followed were heavily influence by the default structure of
child themes for version 4 of the [Omega](https://www.drupal.org/project/omega)
theme for Drupal.
