# Adam's Adventure 2D game
A mini space runner 2D game.

## Instructions to compile and run

### Windows
- Assuming that you have already installed the GNU C++ compiler with version 9.3.0 installed as this program requires a GNU C++ compiler to work

1. Unzip the folder and extract the folder to your current directory.
2. Go to your File Explorer > Computer > Right click > Properties > Advanced system settings > Environment Variables.
3. In the System Variables, scroll down and find the PATH environment variable and select it then click on Edit > New > copy the file path of the “lib” folder and paste it into the dialog box > Click OK.
4. Go to Command Prompt, change to the directory of the game by using the ```cd``` command.
5. Next, type in ```g++ AdamAdventure.cpp -I "<path-to-include/box2d-folder>" -I "<path-to-include-folder>" -L "<path-to-lib-folder>" -std=c++17 -lbox2d -lsfml-graphics -lsfml-window -lsfml-system -o AdamAdventure.exe```
6. Inside the folder, open the AdamAdventure.exe file.
