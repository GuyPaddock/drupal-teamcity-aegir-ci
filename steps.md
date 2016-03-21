NOTES:
  - TeamCity CI server can be either a Windows or Linux host without causing any problems.
  - The build agent must be Ubuntu Linux for best results.

Steps to install Aegir + TeamCity (Roughly in the same order as the video)
==========================================================================
1. *[On Build Agent]* Install Ubuntu with SSH, Apache, MySQL, and PHP (LAMP).

2. *[On Build Agent]* Add Aegir repo:

        echo "deb http://debian.aegirproject.org stable main" | sudo tee -a /etc/apt/sources.list.d/aegir-stable.list
        wget -q http://debian.aegirproject.org/key.asc -O- | sudo apt-key add -
        sudo apt-get update

3. *[On Build Agent]* Install Aegir (reference: https://virtualopps.wordpress.com/2012/07/09/how-to-install-aegir-in-ubuntu-12-04/)

        sudo apt-get install aegir2

4. *[On Build Agent]* Fix if Aegir install fails with Apache config error:
https://www.drupal.org/node/2275467#comment-8895085

5. *[On Build Agent and CI Server]* Install Java:

        sudo apt-get install default-jre

6. *[On Build Agent and CI Server]* Create Teamcity user account and switch to it:
        sudo useradd --system --create-home --home-dir "/opt/teamcity" --comment "TeamCity build daemon" teamcity
        sudo -i -u teamcity

7. *[On CI Server]* Install TeamCity:
https://www.jetbrains.com/teamcity/download/

8. *[On CI Server]* **(Linux host only)** Add an init script to start TeamCity CI server at boot on port 80:
  - Add init script (`/etc/init.d/teamcity-server`) and `chmod 755` it (see sample `teamcity/init.d/teamcity-server` in this repo).
  - Register script to start at boot with `sudo update-rc.d teamcity-server defaults 98 02`.
  - Install Authbind to allow TeamCity to listen on port 80 without being root:

            sudo apt-get install authbind
            sudo touch /etc/authbind/byport/80
            sudo chown teamcity /etc/authbind/byport/80
            sudo chmod 755 /etc/authbind/byport/80

  - Modify `/opt/teamcity/conf/server.xml` and change Connector port from `8111` to `80` (see: https://confluence.jetbrains.com/display/TCD9/Installing+and+Configuring+the+TeamCity+Server#InstallingandConfiguringtheTeamCityServer-ChangingServerPort)

9. *[On Build Agent and CI Server]* Set a password for teamcity user account.

10. *[On Build Agent]* Add the `teamcity` user to `aegir` and `www-data` groups
    so that creation of build artifacts does not fail when encountering
    `drushrc.php` and `settings.php` files protected by Drupal and Aegir:
     
            sudo usermod -G aegir,www-data -a teamcity

11. *[On CI Server]* Install version control (git, mercurial, etc).

12. *[On Build Agent]* Install TeamCity agent (use Agent Push feature from CI Server for best results).

13. *[On Build Agent]* Install sudoers file for TeamCity to be able to run commands as Aegir -- see `teamcity/sudoers.d/teamcity`.

14. *[On Build Agent]* Add Aegir version to buildAgent.properties to ensure that only Aegir agents run Drupal builds (see: https://confluence.jetbrains.com/display/TCD9/Build+Agent+Configuration):

        agent.aegir.version=6.x-2.5

15. *[On Build Agent]* Add an init script to start the TeamCity Agent at boot:
  - Add init script (`/etc/init.d/teamcity-agent`) and `chmod 755` it (see sample `teamcity/init.d/teamcity-agent` in my repo).
  - Register script to start at boot with `sudo update-rc.d teamcity-agent defaults 98 02`.

16. *[On Build Agent]* Start agent.

17. *[On CI Server]* Add TeamCity CI profile (`drupal_ci.xml` under `/opt/teamcity/.BuildServer/config/projects/_Root/buildTypes`;  see `teamcity/build-types/drupal_ci.xml` in my repo). All you need to do is drop the file in that folder. It will then appear as a new template under Administration -&gt; Projects -&gt; &lt;Root project&gt; -&gt; Build Configuration Templates.

18. *[On CI Server]* Set-up CI build template (change host name, identified as `project.site.root_domain`; and e-mail address, identified as `project.site.email`, as appropriate for your environment).

19. *[On CI Server]* Set-up your first project (Administration -&gt; Projects -&gt; Create Project) with a build configuration that uses the Drupal CI template. Be sure to fill-in any `UNSET` fields with appropriate values for the profile you are installing.

20. *[On CI Server]* Configure VCS roots for the new build configuration. Ensure that the file `build-PROFILE_NAME.make` will be checked-out in the root of the check-out folder. For example, the "checkout pattern" for the sample profile in this repro is `+:drupal/profiles/sample => .`, assuming the VCS root is set to the root of this repo (`https://github.com/GuyPaddock/drupal-teamcity-aegir-ci`).

21. *[On CI Server]* Set the TeamCity Branch spec to `+:*` if you want builds when there are commits on any branch.

22. *[On Build Agent]* Install Drush make local (Make sure to run the last command to clear the drush cache!):

        sudo mkdir -p /usr/local/share/drush/commands
        sudo git clone https://github.com/helior/make_local.git /usr/local/share/drush/commands/make_local
        drush cc drush

23. *[On CI Server]* Run your first build!

24. *[On Build Agent]* You can log-in to your new site using the log-in link
    from within the Aegir Hostmaster Front-end.
    
25. *[On CI Server]* You can view and download all of the files created as part
    of the new install profile by going under the "Artifacts" tab on the build
    in TeamCity.
    
    The ZIP file that gets created can be distributed as the full platform --
    you just need to delete the site for the build from under "sites/", leaving
    the "default" site folder present. The ZIP can be unpacked on a server and
    installed by running install.php.
    
Happy Drupal continuous integration!
