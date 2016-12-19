; Top-level "Sample" installation profile Drush Makefile.
;
; This file "bootstraps" the build of a complete Drupal platform that includes.
; This file triggers Drush to download the version of Drupal specified in
; "drupal-org-core.make" and then start building the rest of the installation
; profile under "profiles/sample". From there, Drush make follows
; "drupal-org.make" to download the contrib modules and patches that the
; profile requires.
; 
; Copyright (C) 2015-2017  Red Bottle Design, LLC
;
; This program is free software: you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the Free
; Software Foundation, either version 3 of the License, or (at your option)
; any later version.
;
; This program is distributed in the hope that it will be useful, but WITHOUT
; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
; FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
; more details.
;
; You should have received a copy of the GNU General Public License along
; with this program.  If not, see <http://www.gnu.org/licenses/>.
;
; @author Guy Paddock (guy@redbottledesign.com)

api = 2
core = 7.x

; Include the definition for how to build Drupal core directly, including patches:
includes[] = drupal-org-core.make

; Download the Sample install profile and recursively build all its
; dependencies.
;
; NOTE: This requires the Drush Make Local module
; (https://www.drupal.org/project/make_local and
;  https://github.com/helior/make_local).
;
; ALSO NOTE: Paths are relative to the CI checkout folder.
projects[sample][type] = "profile"
projects[sample][version] = 1.x-dev
projects[sample][download][type] = local
projects[sample][download][source] = .
