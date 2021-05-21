# bless
Allow or disallow root capabilities for a normal user

This program allows you to add root abilties to normal users on login or during a shell session.

To use this you need to also add the pam_cap.so module to your pam stack.

1. Add the file /etc/security/capabilty.conf
2. Inside the file add capabilities you want:
   "cap_net_bind_service,cap_net_admin     matthew"
3. Login with your user and call "exec bless". Alternatively add "exec bless" to the end of your "~/.bash_profile".

You will get output indicating the ambient capabilities you have been raised with. After which you can perform whatever privileged action you want to apply.

IE in python:
$ python
Python 2.7.5 (default, Nov 16 2020, 22:23:17) 
[GCC 4.8.5 20150623 (Red Hat 4.8.5-44)] on linux2
Type "help", "copyright", "credits" or "license" for more information.
> > > from socket import *
> > > s = socket(AF_INET, SOCK_STREAM)
> > > s.bind(("0.0.0.0", 400))
> > > s.listen(5)
