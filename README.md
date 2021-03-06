# bless
Allow or disallow root capabilities for a normal user

This program allows you to add root abilties to normal users on login or during a shell session.

To use this you need to also add the pam_cap.so module to your pam stack.

1. Add the file /etc/security/capabilty.conf
2. Inside the file add capabilities you want:
   "cap_net_bind_service,cap_net_admin     matthew"
3. Login with your user and call "exec bless". Alternatively add "exec bless" to the end of your "~/.bash_profile".

You will get output indicating the ambient capabilities you have been raised with. After which you can perform whatever privileged action you want to apply.

IE in python (needs cap_net_bind):
```
$ python
>>> from socket import *
>>> s = socket(AF_INET, SOCK_STREAM)
>>> s.bind(("0.0.0.0", 400))
>>> s.listen(5)
```
# curse
The inverse of bless. Will never allow a capability, even if you raise yourself to root via sudo or some other privilege raising event.

# Purpose
I wrote this program for my personal projects, often which are bound on low ports but I didn't want to be running a compiler as root.
It uses the ambient capabilities feature on Linux kernels to work.
