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

8. *[On Build Agent and CI Server]* Set password for teamcity user account.

9. *[On CI Server]* Install version control (git, mercurial, etc).

10. *[On Build Agent]* Install TeamCity agent (use Agent Push feature from CI Server for best results).

11. *[On Build Agent]* Set-up TeamCity sudoers -- see "teamcity/sudoers.d/teamcity".

12. *[On Build Agent]* Add Aegir version to buildAgent.properties (see: https://confluence.jetbrains.com/display/TCD9/Build+Agent+Configuration):

        agent.aegir.version=6.x-2.1

13. *[On Build Agent]* Add init script (`/etc/init.d/teamcity-agent`) and `chmod 755` it (see sample `teamcity/init.d/teamcity-agent` in my repo).

14. *[On Build Agent]* Start agent.

15. *[On CI Server]* Add TeamCity CI profile (`drupal_ci.xml` under `/opt/teamcity/.BuildServer/config/projects/_Root/buildTypes`;  see `teamcity/build-types/drupal_ci.xml` in my repo).

16. *[On CI Server]* Set-up CI build template (change host name and e-mail address as appropriate for your environment).

17. *[On CI Server]* Set-up your first project.

18. *[On CI Server]* Set the TeamCity Branch spec to `+:*` if you want builds when there are commits on any branch.

19. *[On Build Agent]* Install Drush make local (Make sure to run the last command to clear the drush cache!):

        git clone https://github.com/helior/make_local.git /usr/share/drush/commands/make_local
        drush cc drush

20. *[On CI Server]* Run your first build!