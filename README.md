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

Exe without console window

gcc SynGame.c -o SynGame.exe "allegro\lib\liballegro-5.0.10-md.a" "allegro\lib\liballegro_audio-5.0.10-md.a" "allegro\lib\liballegro_acodec-5.0.10-md.a" "allegro\lib\liballegro_image-5.0.10-md.a" "allegro\lib\liballegro_dialog-5.0.10-md.a" "allegro\lib\liballegro_primitives-5.0.10-md.a" --machine-windows

Exe with console window

gcc SynGame.c -o SynGame.exe "allegro\lib\liballegro-5.0.10-md.a" "allegro\lib\liballegro_audio-5.0.10-md.a" "allegro\lib\liballegro_acodec-5.0.10-md.a" "allegro\lib\liballegro_image-5.0.10-md.a" "allegro\lib\liballegro_primitives-5.0.10-md.a"