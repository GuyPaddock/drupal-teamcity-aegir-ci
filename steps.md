NOTES:
  TeamCity CI server can be either a Windows or Linux host without causing any problems.
  The build agent must be Ubuntu Linux for best results.

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

6. *[On Build Agent and CI Server]* Create Teamcity user account and switch to it.

7. *[On CI Server]* Install TeamCity:
https://www.jetbrains.com/teamcity/download/

8. *[On CI Server]* **(Linux host only)** Add an init script to start TeamCity CI at boot on port 80:
  - Add init script (`/etc/init.d/teamcity-server`) and `chmod 755` it (see sample `teamcity/init.d/teamcity-server` in my repo).
  - Register script to start at boot with `sudo update-rc.d teamcity-server defaults 98 02`.
  - Install Authbind to allow TeamCity to listen on port 80 without being root:

        sudo apt-get install authbind
        sudo touch /etc/authbind/byport/80
        sudo chown teamcity /etc/authbind/byport/80
        sudo chmod 755 /etc/authbind/byport/80

  - Modify `/opt/teamcity/conf/server.xml` and change Connector port from 8111 to 80 (see: https://confluence.jetbrains.com/display/TCD9/Installing+and+Configuring+the+TeamCity+Server#InstallingandConfiguringtheTeamCityServer-ChangingServerPort)

9. *[On Build Agent and CI Server]* Set password for teamcity user account.

10. *[On CI Server]* Install version control (git, mercurial, etc).

11. *[On Build Agent]* Install TeamCity agent (use Agent Push feature from CI Server for best results).

12. *[On Build Agent]* Set-up TeamCity sudoers -- see "teamcity/sudoers.d/teamcity".

13. *[On Build Agent]* Add Aegir version to buildAgent.properties (see: https://confluence.jetbrains.com/display/TCD9/Build+Agent+Configuration):

        agent.aegir.version=6.x-2.1

14. *[On Build Agent]* Add an init script to start the TeamCity Agent at boot:
  - Add init script (`/etc/init.d/teamcity-agent`) and `chmod 755` it (see sample `teamcity/init.d/teamcity-agent` in my repo).
  - Register script to start at boot with `sudo update-rc.d teamcity-agent defaults 98 02`.

15. *[On Build Agent]* Start agent.

16. *[On CI Server]* Add TeamCity CI profile (`drupal_ci.xml` under `/opt/teamcity/.BuildServer/config/projects/_Root/buildTypes`;  see `teamcity/build-types/drupal_ci.xml` in my repo).

17. *[On CI Server]* Set-up CI build template (change host name and e-mail address as appropriate for your environment).

18. *[On CI Server]* Set-up your first project.

19. *[On CI Server]* Set the TeamCity Branch spec to `+:*` if you want builds when there are commits on any branch.

20. *[On Build Agent]* Install Drush make local (Make sure to run the last command to clear the drush cache!):

        git clone https://github.com/helior/make_local.git /usr/share/drush/commands/make_local
        drush cc drush

21. *[On CI Server]* Run your first build!
