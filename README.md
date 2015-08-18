This is a workaround for a hardware bug in:

    Logitech T630 Ultrathin Bluetooth Mouse


Description of the Bug:

  This mouse supports gestures.  Under certain conditions (high load?) it
sends random keystrokes (eg. the key '7') repeatedly instead of mouse
events.


Description of the Fix:

  We use input libs (written by Gerd Knorr) to configure the mouse
to send KEY_UNKNOWN instead of any and all keys it can or cannot send.


INSTALL:

1. Install input-libs (from this repository)

2. Install 'fixMouse/fixMouse.sudo' in /etc/sudoers.d/ and change
username to match your username

3. Run 'fixMouse.sh' when you login


FAQ:

1. Does the bug happen in Windows ?

Apparently yes, according to other people.  I haven't tried.

2. Have you tried contacting Logitech?

Logitech disavows any Linux questions.
https://forums.logitech.com/t5/Mice-and-Pointing-Devices/T630-Mouse-on-Ubuntu-Linux/td-p/1094111

3. Do you have a fix for Windows ?

No.

4. What keys does it send ?

It emulates a full keyboard, so in theory it could send any key.  However,
I've only seen it ever send numeric keys.

It seems to get stuck under high load.

5. Is there more information on the bug ?

https://askubuntu.com/questions/568228/controlling-bluetooth-mouse-gestures-that-register-keyboard-events

6. Why do you include 'input'/'input-libs'/'input-utils' ?

The workaround needs input-kbd.  However input-libs 1.0 has a bug
that this workaround needs fixed, so it cannot depend on input-libs 1.0

input-kbd assumes that the key numbers listed are both contiguous and
start at 0.  For this mouse that's not true.

7. Is it OK to include input-libs?

It's GPL-ed.

I've tried to contact the author (Gerd Knorr) and the Ubuntu
and SuSE maintainer names I've found.
