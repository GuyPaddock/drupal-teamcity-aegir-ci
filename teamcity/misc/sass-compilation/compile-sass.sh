#!/bin/bash --login

# Fail fast on errors.
set -e;
set -u;

export HOME="/opt/teamcity";
export PATH="$HOME/.rbenv/bin:$HOME/.rbenv/plugins/ruby-build/bin:$PATH";

export NODEJS_VERSION="%nodejs.version%"
export NVM_DIR="${HOME}/.nvm"

eval "$(rbenv init -)";

sass_themes=`find . -maxdepth 4 -type d -path './themes/custom/*/sass' -exec dirname '{}' ';'`;

# Initialize NVM, if available.
if [ -s "${NVM_DIR}/nvm.sh" ]; then
  # BUGBUG: NVM's own scripts use unbound variables :(
  set +u;

  echo "Initializing NVM..."
  source "${NVM_DIR}/nvm.sh";

  echo "Setting-up NodeJS..."
  nvm install "${NODEJS_VERSION}";

  # Install our required tooling
  if [ ! -x "$(command -v npm-cache)" ]; then
    echo "Installing required global packages..."
    npm install -g grunt gulp npm-cache;
  fi;

  # Back to fail-fast behavior
  set -u;
fi;

if [ -z "$sass_themes" ]; then
  echo "Skipping SASS compilation; no SASS themes were detected in the installation profile.";
else
  starting_path=`pwd`;

  # Initialize Ruby
  set +u; # Allow unbound variables because Ruby is lazy.

  for theme_name in $sass_themes; do
    echo "Compiling SASS in $theme_name...";
    cd $theme_name;

    if [ -f "Gemfile" ]; then
      echo "Using the Ruby Compass compiler for SASS.";

      if [ -f ".ruby-version" ]; then
        # Honor Ruby version in the theme
        export theme_ruby_version=`head -n 1 .ruby-version`;
        export selected_ruby_version=`rbenv install --list "$theme_ruby_version" 2>&1 | grep "  $theme_ruby_version" | tail -n 1 | tr -d ' '`;

        echo "Honoring theme-specific Ruby version '${selected_ruby_version}'";
      else
        selected_ruby_version='2.2.3';

        echo "Using the default Ruby version '${selected_ruby_version}'";
      fi;

      rbenv install --skip-existing "${selected_ruby_version}";
      bundle install;
      npm-cache install npm;

      grunt -no-color compass;

    elif [ -f "gulpfile.js" ]; then
      echo "Using the libSass compiler with Gulp.";

      npm-cache install npm;

      # Compile the Sass (+ CoffeeScript, if available)
      gulp all:dev || gulp sass:dev;

      # Now, generate docs if we have them.
      gulp sass:doc || true;
    else
      echo "Unable to detect either Ruby SASS or Gulp-based libSass compilation" >&2;
    fi;

    # Clean-up temporary artifacts from the theme build
    rm -rf .sass-cache;
    rm -rf node_modules;

    # Switch back to original path before moving on to the next theme.
    cd $starting_path;
  done;
fi;
