# Running P4 Example Tutorial on New England Research Center (NERC)

## Introduction
The P4 tutorial has already been setup in a virtualbox. In this guideline, we will show how to install and boot the VM on NERC. <br>
This tutorial is setup and test on an Ubuntu distribution. This setup is only used for first time. After the first time setup, you could always jump to Step: Launch virtual machine.

## Prerequisite 

For this tutorial, a VNC server and client should be  set up on the NERC instance and your host machine. 
You can follow the instructions from the [link](https://docs.google.com/document/d/1_JZ1K0lDdCTKP6TePhMbEBIyySO4jYZbF9-yBIQO07A/edit).

The commands below should be run from a terminal inside the VNC server instance connected to your NERC instance.  

## Steps
### Install Vagrant
After launching to your NERC instance through the VNC client, you need to install Vagrant by the following command:\
`curl -fsSL https://apt.releases.hashicorp.com/gpg | sudo apt-key add -`\
\
`sudo apt-add-repository "deb [arch=amd64] https://apt.releases.hashicorp.com $(lsb\_release -cs) main"`\
\
`sudo apt-get update && sudo apt-get install vagrant`


These commands is used for install vagrant on Ubuntu. You could find alternative install commands for other system from [here](https://www.vagrantup.com/downloads).
### Install Virtualbox
Run `sudo apt-get install virtualbox`.
### Clone P4 tutorial repository
Run `git clone https://github.com/p4lang/tutorials.git`.
### Launch virtual machine
`cd tutorials/vm-ubuntu-20.04`\
Run `vagrant up`. This will start the boot process for virtual machine. The first time boot will take around 20 minutes.  \
After the system boots up, you can login to the `p4` account using password `p4`.
The P4 tutorial is located at `p4@p4:~/tutorials`

Now you can follow up the instructions from the [P4 tutorial](https://github.com/p4lang/tutorials).

