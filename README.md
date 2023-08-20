Video Explanation of Install, Configuration, and Explanation
Table of Contents:
1. Install
2. Configuration
3. Explanation
<br />

**INSTALL**

The install instruction is going to be the same as myapex (https://github.com/arturzxc/myapex). This should work for any distro but if you are downloading Linux just to use this cheat or any other I would recommend Ubuntu, Manjaro, or Arch with use the archinstall script.
1. git clone this repository 
2. >cd myapex
3. compile with
>g++ Main.cpp -lX11
4. run apex
5.> sudo ./a.out
<br />
Here is where the install differs slightly, once in the game you will notice that all of the text that was printed out to the terminal is now gone and replaced by a menu. I would recommend setting this window to the top because there are a couple of values that are easier to keep track of like that
I use Manjaro personally but setting to the top should be similar and if not then Google/Chat-GPT it. 
left click on the terminal
6. a set option that says something like "always on top" 
7. if this does not work then don't be in fullscreen

**CONFIGURATION**

So I redesigned some source code and made it so that the anti-recoil and aimbot have much more customization (the GLOW also has some but you will have to edit the real source code which is super simple with all of the comments I added but it can be intimidating for people without brains)

So in the aimbot we have our anti-recoil setting, the code

>double norecoilPitchStrength = 0.5f; //vertical anti-recoil strength

>double norecoilYawStrength = 0.5f;   //Horizontal anti-recoil strength

defines the default value that this will be at. (the higher the number the less recoil). but there is also some buttons that can be used to adjust the anti-recoil strength mid-game:

>#define MoreVirt_button XK_KP_1 //More vertical recoil button

>#define LessVirt_button XK_KP_4 //Less vertical recoil button

>#define MoreHorz_button XK_KP_2 //More Horizontal recoil button

>#define LessHorz_button XK_KP_5 //Less Horizontal recoil button

The last value that starts with XK is the button that is attributed to each button. If you want you can change this but by default it's set to Keypad 1, 4, 2 & 5. It used dx11 keybinds so if you need to change it to something else just google it.


Then we move onto the aimbot. We have 3 aimbot types and a button for Aimbot types.

Aimbot types: Range or FOV

FOV: FOV is the distance from the player to the center of your screen. This for works similarly to how in-game FOV works but instead sight its assertive range of Aimbot.

Target: In this context it means who we are targeting (witch player we are locked onto)

The FOV range Aimbot will get the closest person to the screen and if they are in the selected FOV range that's specified in the config section it will target them. 

They will target the closest person to you who is also in your specified FOV (how the old myApex cheat worked).

The button to toggle between these two modes is specified in this line

>#define distance_or_fov_button_dx11 XK_9 //distance_or_fov_button_button

Close Range: After some configuring I made it so that the aimbot targets the closest player even if the FOV mode is selected. Trust me this is what you want. The aimbot has the basic settings 

>#define close_range_button XK_5 //Close range button

>float close_range_FOV = 50.0f; //Close range FOV

>int close_range_smoothing = 100; //Close range smoothing (bigger number, less powerful aimbot)


the FOV is a good setting at 50 FOV. but the problem is that it targets too high vertically so you need to use 

float FOV_divide_close = 0.25;

this will decrease the fov by 0.25 so 12.5.

Note that in the code calculates a dropoff the further you are so that 50 FOV is only when your ~ 10m from the enemy and if your over ~35 its 15FOV

<br />
Medium Range:

This is much simpler when the button is pressed (look at the "close" section as it used the same config structure of buttons, FOV, and smoothing. This button is meant for medium-range combat and has nothing special like the one above. 

<br />
Long Range: 

The long-range is meant for just that, long-range, it's the same as medium but way faster. You should also use it if you're using energy weapons.

<br />
Advanced Config:


how much the virtual FOV is divided
float FOV_divide = 0.5f; 
how much the closeup virtual FOV is divided
float FOV_divide_close = 0.25;

If your have a vertical aimbot
bool aimbot_vertical_aim = false;
The base smoothing for vertical FOV
float vertical_smoothing = 0.33f; 
An option I found useful, only use the vertical aimbot if you want vertical when hip firing:
bool vertical_aimbot_hipfire_only = false;
I guess this might help with any AI-anti-cheat but prevents you from flicking to the true center of someone and might do something for the anti-cheat
float deadZone = 0.001;

**Explanation of the Code:**

Link to video were its done: 
