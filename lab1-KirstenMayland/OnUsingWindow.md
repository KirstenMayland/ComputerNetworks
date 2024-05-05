# On Generating ICMP Captures with Virtual Machines on a Windows
### (As seen in lab 1 of Dartmouth 24S Computer Networks course)

## Disclosure:
This is my current understanding of the topic; however, I am very much a beginner so if you as a reader spot any errors or inconsistencies, please just let me know, I don't want to spread misinformation and I would welcome the opportunity to learn more.

## Overview
The goal of exercise 2 in the lab was to create as many different ICMP type/code combinations as possible. Some of that could be done by pinging and tracerouting various IP addresses/websites, but often IP addresses wouldn't respond. In order to create an environment where you could send out requests and get replies, one needed to create that environment with a virtual machine that they controlled. For MacOS users, they were able to just set up an instance in Multipass and talk to it. For Windows users, we ran into some issues as detailed below.

#### Representation of the setup (to the best of my current knowledge):
[WSL Ubuntu (VM)] <---->  [ Base Computer (Windows Powershell) ]  <---some_number_of---> [Multipass (VMs)]

## Initial Incorrect Approaches
### Talking from Base Computer to Multipass VM
This was the approach that MacOS users used, and was my initial approach. It does work to some degree, you can ping and traceroute the multipass VM from Windows Powershell; however, it got really obnoxious because you needed to use Windows Powershell's networking tools, which aren't as well-documented or simple as UNIX networking tools. For example, Windows Powershell doesn't support hping3. If someone was knowledgable in Windows Powershell, this is probably a viable option, it was just a headache and a half for me, so I never fully explored this option.

### Talking from WSL Ubuntu to Multipass VM
can't, not connected, on 2 different ethernet networks (I think, need to research more)
172 vs 129/198

### Talking from Multipass VM to Multipass VM
can talk between them, just can't pick up on the traffic

## Method that Worked
### Talking from WSL Ubuntu to Base Computer
still able to use Linux, doesn't work initially, like most of the IP addresses/websites out there you hit a firewall, but since it's your computer, you can override that firewall

* Windows Defender Firewall > Advanced Settings (left side panel) > Inbound rules (left side panel) > New rule (right side panel)
* In the dialog, choose "custom" > then, Protocol type = ICMPv4 > ICMP settings, customize = All ICMP types > Save



## Methods to Investigate
### Talking from Multipass VM to Base Computer
if works, essentially same thing as talking from WSL Ubuntu, just with another thing downloaded for no reason

### SSH'ing into a VM
I'm honestly confused on this topic, would need more insight
