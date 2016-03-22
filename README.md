# POE-whisper-dispatcher

#### What is POE-whisper-dispatcher?
POE-whisper-dispatcher forwards whisper messages from __Path Of Exile__ to __Pushbullet__.

![Preview](https://www.dropbox.com/s/movwfc0bu9zmyqs/Poe-whisper-dispatcher.png?dl=1)

#### Compatibility
This software is only compatible with OS X at the moment.

#### How does it work?
PoE logs various informations inside a specific file within PoE installation directory (Client.txt). This program parses that file to extract whisper messages before sending them to __Pushbullet__'s v2 API through __curl__.

#### How to use?
Two parameters are required:
- The path to the folder enclosing __Path Of Exile.app__
- A Pushbullet token

Aditionnaly, the refresh frequency of the client file can be set with the slider. Upon pressing __Start__, a config file containing the provided token and the path to the folder enclosing your PoE installation is created inside __~/.config/poe-whisper-dispatcher/config.cfg__.


#### Compiling from source
This software requires QT 5. 
To compile the project, open _POE-whisper-dispatcher.pro_ and press __Build__.
