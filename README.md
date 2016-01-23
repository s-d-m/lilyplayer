PIANOGUI
========

Pianogui is a simple application that let's you play a midi file, or a midi
input, and shows how it should be played on a piano.
Pianogui is the "graphical" equivalent of [Pianoterm][pianoterm site]

[pianoterm site]: https://github.com/s-d-m/pianoterm

Below is a screenshots of the main window.


![screenshot of pianoguy] (./misc/mainwindow.png?raw=true)


License
------

Todo: choose a license

Build dependencies
----------------

`Pianogui` requires a C++14 compiler to build. (g++ 5.3 and clang++ 3.6.2 work both fine).
It also depends on the following libraries:

- [`libRtMidi`][rtmidi]
- [`Qt`][qt5]

[rtmidi]: http://www.music.mcgill.ca/~gary/rtmidi/
[qt5]: http://www.qt.io/

Also note that `pianogui` does not play music itself. Instead it
relies on a system-wide midi sequencer.  On `GNU/Linux` you might
consider installing `timidity`

On `debian`, one can install them the following way:

	sudo apt-get install timidity librtmidi-dev librtmidi2 libqt5widgets5 libqt5gui5 libqt5core5 qt5-default qt5-qmake g++-5,3


Compiling instructions
-------------------

Once all the dependencies have been installed, you can simply compile `pianogui` by entering:

	make

This will generate the `pianogui` binary in `./bin`

How to use
----------

Pianoterm needs a midi sequencer. If you decided to use timidity, you will need to run it first using:

	timity -iA &

Then you can run the application by typing

	./bin/pianogui

This will open an a window showing a piano keyboard.
On the "output port" menu, you can select the midi sequencer like `TiMidity xxx:0`.
On the "input" menu, you can either select an input keyboard (e.g. a [virtual midi keyboard player][vmpk])

[vmpk]: http://sourceforge.net/projects/vmpk/

or you can play a midi file by choosing `select file` in the input menu, or using the `Ctrl + O` shortcut.

When playing a midi file, one can play/pause it using the `space` key, or the `Ctrl + P` shortcut.


Other files you may want to read
--------------------------------

todo.txt contains a list of things that I still need to do.

Bugs & questions
--------------

Report bugs and questions to da.mota.sam@gmail.com (I trust the anti spam filter)
