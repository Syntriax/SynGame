# SynGame
Simple Shoot 'Em Up game. 

Keys:
Move Up - Up Arrow
Move Right - Right Arrow
Move Down - Down Arrow
Move Left - Left Arrow
Shoot - Space
Restart - R
Exit - Escape(ESC)

Settings.syn can be opened with a text editor.

Settings Format:
First line: Screen Mode, 1 is Fullscreen, 0 is Windowed (Default Value = 1)
Second line: Screen Width, Windowed Mode Only (Default Value = 1600)
Third line: Screen Height, Windowed Mode Only (Default Value = 900)
Fourth line: Enemy Count Limiter, there will be no more enemies than this number on the screen (Default Value = 12)

Exe without console window

gcc SynGame.c -o SynGame.exe "allegro\lib\liballegro-5.0.10-md.a" "allegro\lib\liballegro_audio-5.0.10-md.a" "allegro\lib\liballegro_acodec-5.0.10-md.a" "allegro\lib\liballegro_image-5.0.10-md.a" "allegro\lib\liballegro_dialog-5.0.10-md.a" "allegro\lib\liballegro_primitives-5.0.10-md.a" --machine-windows

Exe with console window

gcc SynGame.c -o SynGame.exe "allegro\lib\liballegro-5.0.10-md.a" "allegro\lib\liballegro_audio-5.0.10-md.a" "allegro\lib\liballegro_acodec-5.0.10-md.a" "allegro\lib\liballegro_image-5.0.10-md.a" "allegro\lib\liballegro_primitives-5.0.10-md.a"