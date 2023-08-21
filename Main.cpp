/*
Assuming your on the original github branch by DanJeEpicMan you should see that 
there is a "Dev" folder or somthing like that, if your trying to understand the code I would
recomend going there if your not already to see how each individual function(Glow, Aimbot, Recoil) works.
*/

/////////User Settings/////////
//I am waaaaay to lazy to make a setting file so do this

//////////////--Recoil--//////////////
//This can be changed on the fly useing hotkeys but this is the default value
//(should be between 0 (none) 1 (full))
double norecoilPitchStrength = 0.5f; //vertical anti-recoil strength
double norecoilYawStrength = 0.5f;   //Horizontal anti-recoil strength

//Less recoil. More Virt(ical) recoil, less Horz(ontal) recoil, Less Virt, More Virt
#define MoreVirt_button XK_KP_1 //More vertical recoil button
#define LessVirt_button XK_KP_4 //Less vertical recoil button
#define MoreHorz_button XK_KP_2 //More Horizontal recoil button
#define LessHorz_button XK_KP_5 //Less Horizontal recoil button

//////////////--Aimbot--//////////////
//Has 3 settings, close range, mid range, long range
//also has 1 button to toggle between distance or FOV targeting method
#define close_range_button XK_5 //Close range button
float close_range_FOV = 50.0f; //Close range FOV
int close_range_smoothing = 100; //Close range smoothing

#define mid_range_button XK_3 //Mid range button
float mid_range_FOV = 10.0f; //Mid range FOV
int mid_range_smoothing = 50; //Mid range smoothing

#define long_range_button XK_6 //Long range button
float long_range_FOV = 10.0f; //Long range FOV
int long_range_smoothing = 25; //Long range smoothing

#define distance_or_fov_button_dx11 XK_9 //distance_or_fov_button_button

//Have no Idea what your doing? 
//The following settings a probably fine but if you do want to edit it then read carfully

/*
I find that its better to have the virtical FOV as lower to not target the wrong person. 
The following will be multiplied against the FOV check to make it less
*/
float FOV_divide = 0.5f; // 1 * 0.5 = 1/2 the FOV
/*
When hipfireing you want a high FOV, but your virtical FOV is going to target people above you. So, what to do?
Well you do what we did above but EVEN MORE. The smaller the amount, the less virtical FOV
*/
float FOV_divide_close = 0.25;
//1000 - distance = x, x * close_distance_divide_falloff = y, y * FOV_divide_close
float close_distance_divide_falloff = 255.0f; 

// AIMBOT vertical AIM OPTIONS (disabled by default) (its not very usefull apart from hipfire)
bool aimbot_vertical_aim = false; //if true then it will aim vertically and horizontally
//vertical aimbot is much less strong so it needs less smoothing, this will be miltiplied against it therefor recucing smoothing
float vertical_smoothing = 0.33f; 
//vertical aimbot only for hipfire (IMO the only time its usefull)
bool vertical_aimbot_hipfire_only = false; //to use this aimbot_vertical_aim should be true

//deadzone of the aimbot
float deadZone = 0.001;


#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <thread>
#include "Memory.cpp"
#include "Offsets.cpp"
#include "functions.cpp"
#include <cmath>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

//NOTE: when the term FOV is used I am talking about the distance form the crosshair to the enemy player

int main() {

    std::cout << "Starting..." << std::endl;
    std::cout << "Close Range FOV: " << close_range_FOV << std::endl;
    std::cout << "Close Range Soothing: " << close_range_smoothing << std::endl;
    std::cout << "Mid Range FOV: " << mid_range_FOV << std::endl;
    std::cout << "Mid Range Soothing: " << mid_range_smoothing << std::endl;
    std::cout << "Long Range FOV: " << long_range_FOV << std::endl;
    std::cout << "Long Range Soothing: " << long_range_smoothing << std::endl;
        
    //Checks if we are root (sudo)
    if (getuid())
    {
        printf("Run program using as root (sudo)\n");
        return -1;
    }
    //gets the PID of the game using the output of the command "pidof -s r5apex.exe"
    if (mem::GetPID() == 0)
    {
        printf("Game not found (using name r5apex.exe)\n");
        return -1;
    }
    /*
    If at some point this does not work there is a simple solution. Go into task manager and look for a process
    that seems like Apex Legends, get its PID and then type "sudo nano /proc/(the pid number)/stat" and look for
    the name of the process, it should be in brackets. Then in Memory.cpp replace "r5apex.exe" with the new name
    */

    /*
    In the following code you will see some variables declaired, these variables will not be reset wen the loop
    restarts so we can use them to save information about the game and about setting and buttom presses
    */

    //used to kep track of our previous recoil (used for anti-recoil)
    float m_previousPunchPitch;
    float m_previousPunchYaw;

    //used for a button to toggle between distance or FOV aimbot targeting method
    bool distance_or_fov_button = false; //false = distance, true = fov
    bool was_distance_or_fov_button_pressed = false;

    //used for the button to make less pitch recoil
    bool was_norecoilPitchStrength_buttonMore = false;

    //used for the button to make more pitch recoil
    bool was_norecoilPitchStrength_buttonLess = false;

    //used for the button to make less yaw recoil
    bool was_norecoilYawStrength_buttonMore = false;

    //used for the button to make more yaw recoil
    bool was_norecoilYawStrength_buttonLess = false;

    //will store the entity we are locked onto in the aimbot
    int LockedEntity = 71;

    //last time we saw the entity used to find if the entity is visible
    float m_lastVisibleTime[70];
        
    //usedto help with the buffer for the lastVisibleTime
    int loopsSinceLastVisible = 0;

    //used for menu
    int loopAmount = 0;

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    while(1) {
        loopAmount++; //used for menu
        /*
        In the following code we do a coupple of things, we get basic info about the player but we also test if we are in a game.
        */

        //Get local player (a pointer to our player)
        long LocalPlayer = mem::ReadLong(offsets::REGION + offsets::LOCAL_PLAYER);

        try
        {
            std::string levelName = mem::ReadString(offsets::REGION + offsets::LEVEL);
            if (levelName.empty() || levelName.compare("mp_lobby") == 0) {
                std::cout << "error: In lobby \nSleeping 10 seconds" << std::endl;

                std::this_thread::sleep_for(std::chrono::milliseconds(10000));
                continue;
            }

            //if we can get this than we are in a game, if we cant it will goto catch
            float LocalPlayer_x = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN);
        } catch(...) {
            //if error
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            std::cout <<     "error: Not in game \nSleeping 10 seconds" << std::endl;
            continue;
        }



        //Getting the X, Y, Z of our player
        float LocalPlayer_x = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN);
        float LocalPlayer_y = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN + sizeof(float));
        float LocalPlayer_z = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN + sizeof(float) + sizeof(float));
        //Get level name and debug

        //Get our team number
        int LocalPlayer_Team = mem::ReadInt(LocalPlayer + offsets::TEAM_NUMBER);


        //Get our pitch and yaw (were we are looking)
        const double LocalPlayer_pitch = mem::ReadFloat(LocalPlayer + offsets::VIEW_ANGLE);
        const double LocalPlayer_yaw = mem::ReadFloat(LocalPlayer + offsets::VIEW_ANGLE + sizeof(float));

        /*
        Using the information about the localPlayer we got above, we not get information about the entitys in the game. In apex
        there are a max of 70 players so we loop 70 times. We will store information about for close we are to each entity and 
        how close the enemys are to our crosshair (FOV). 

        This loop is also used for Glow.
        */

        //were we store the sorted & unsorted distance 
        float distance[70];
        float SortedDistance[70];

        //were we store the sorted & unsorted FOV
        float FOV[70];
        float SortedFOV[70];


        //This is a for loop that will loop 70 times and every time that it loop i will increase by 1
        for (int i = 0; i < 70; i++)  {
            try {
                //This acts like the localPlayer but for each enemy
                long basePointer = mem::ReadLong(offsets::REGION + offsets::ENTITY_LIST + ((i + 1) << 5));

                //Gets the x, y, z of the specific entity that we are curretly on 
                float entity_x = mem::ReadFloat(basePointer + offsets::LOCAL_ORIGIN);
                float entity_y = mem::ReadFloat(basePointer + offsets::LOCAL_ORIGIN + sizeof(float));
                float entity_z = mem::ReadFloat(basePointer + offsets::LOCAL_ORIGIN + sizeof(float) + sizeof(float));
            
                //Gets the team of the current entity
                int entity_team = mem::ReadInt(basePointer + offsets::TEAM_NUMBER);

                //saves the distance (applys quadratirc formula to the X & Y differance) to position i in the array. This 
                distance[i] = calculateDistance2D(LocalPlayer_x, LocalPlayer_y, entity_x, entity_y);

                //Going to be used to calculate FOV (distance from crosshair to enemy)
                double desiredViewAngleYaw = calculateDesiredYaw(LocalPlayer_x, LocalPlayer_y, entity_x, entity_y);
                double desiredViewAnglePitch = calculateDesiredPitch(LocalPlayer_x, LocalPlayer_y, LocalPlayer_z, entity_x, entity_y, entity_z);

                //Will be used to get FOV
                double pitchDelta = calculateAngleDelta(LocalPlayer_pitch, desiredViewAnglePitch);
                double angleDelta = calculateAngleDelta(LocalPlayer_yaw, desiredViewAngleYaw);

                //saves the FOV to position i in the array
                FOV[i] = (abs(pitchDelta))/2 + abs(angleDelta);

                //if its less than 10 than its probobly us, we dont want to target ourselfs to set to 654321
                if (distance[i] < 10) {
                    distance[i] = 654321;
                    FOV[i] = 654321;
                }

                /////////////////Glow Part///////////////////// - the following code does the Glow part of the cheat
                int R, G, B; //Shells were we will store the values of what color people should be

                // Enable glow
                mem::WriteInt(basePointer + offsets::GLOW_ENABLE, 1);    

                // Glow through walls
                mem::WriteInt(basePointer + offsets::GLOW_THROUGH_WALL, 1);

                //Get the shield value to change the color of the glow 
                int shield = mem::ReadInt(basePointer + offsets::CURRENT_SHIELDS);    

                //if they are on our team then make them glow black
                if (entity_team == LocalPlayer_Team) {
                    int type = 75; //non-teamates are type 101
                    mem::Write(basePointer + (0x2c4 + 0x30), &type, sizeof(int));
                }

                //makes it change color to match shield
                if (shield > 75) {                      //if above 75% shield
                    R = 1; G = 0; B = 2;                //make purple
                } else if (shield < 76 && shield > 50) {//if between 75% and 50% shield
                    R = 0; G = 1; B = 2;                //make blue
                } else if (shield < 51 && shield > 0) { //if between 50% and 0% shield
                    R = 0; G = 3; B = 0;                //make green
                } else {                                //if 0% shield
                    R = 0; G = 2; B = 1;                //make teal
                }

                //if within the distance, make them glow white
                if (distance[i] > 10000) { //the distance 10000 is in game ~300m
                    R = 1; G = 1; B = 1;
                }

                //this code will make the enemy red if we are locked onto them in the aimbot
                if (i == LockedEntity) { 
                    R = 3; G = 0; B = 0;
                }

                //Defines the color of the glow using the values determine above
                mem::WriteFloat(basePointer + offsets::GLOW_COLOR, R);
                mem::WriteFloat(basePointer + offsets::GLOW_COLOR + sizeof(float), G); 
                mem::WriteFloat(basePointer + offsets::GLOW_COLOR + sizeof(float) + sizeof(float), B);

            } catch(...) {
                //if there is an error then set the position & FOV to 654321, when we get to the aimbot a value of 654321 will be skipped
                distance[i] = 654321;
                FOV[i] = 654321;
            }
        }

        /*
        Just to recap what we have done so far. We got info about our player and save the distance and FOV of each entity. Then we set the glow for each enemy with
        variations for things like team, distance, shield level and if we are tagrteting them with our aimbot (this is not yet setup but will be)

        So next we are going to have to sort the distance and FOV. The reason for this is because this is how we know who to sort through first when making our aimbot.
        */

        //prints out distance
        for (int i = 0; i < 70; i++) {
            SortedDistance[i] = distance[i];
        }

        //Saveing info about angles
        for (int i = 0; i < 70; i++) {
            SortedFOV[i] = FOV[i];
        }

        //the distance variable will be sorted, the 70 is how long it is
        selectionSort(SortedDistance, 70);

        //the sorts the FOV variable, the 70 is how long it is
        selectionSort(SortedFOV, 70);


        /*
        Next we need to detect if buttons were pressed. We will do this by using the X11 library.
        There will be 3 buttons you need to hold down to use witch will be the actuall aimbot button.
        There are 3 buttons that have different reasons

        Close range aimbot - has higher smoothing (slower) and has more FOV, should be used for hipfire
        Mid Range aimbot - is decently strong, has a small FOV and kindof smooth aimbot, less obvious than the long range one
        Long range aimbot - is very strong, has a small FOV and at medium range is obvious, should be used for long raneg
        
        There will also be a button to toggle between a distance based aimbot and a FOV based aimbot
        A distance based aimbot gets the enemy thats closest to you in the targeted FOV
        A FOV based aimbot will target an ememy that is closest to your crosshair
        */

        //This will be used to detect if any one of the keys is down
        bool someKeyDown = false;

        //Is close ranged key down
        bool CloseRangeDown = false;
        if(keyDown(XK_5)) {
            CloseRangeDown = true;
            someKeyDown = true;
        }

        //is mid ranged key down
        bool MidRangeDown = false;
        if(keyDown(XK_3)) {
            MidRangeDown = true;
            someKeyDown = true;
        }

        //is long ranged key down
        bool LongRangeDown = false;
        if(keyDown(XK_6)) {
            LongRangeDown = true;
            someKeyDown = true;
        }

        /*
        The buttons above were simple to make as they just needed to detect if the button was down THIS loop. The next button is a
        toggle type of button. If we just tryied to do what we did above and detect it every loop it would flip back and forth every 
        loop (aka 1000s of times a second). 

        We need a different apoch. This is why in our buttons we detect if the key is down and then we use a seoond variable. If this "was"
        variable is false, it means that the button is being pressed for the first time (first time since it was last let go) and we flip the value
        but if its true then its not the first time the we simpelly skip it. The way the button is reset and the "was" goes back to false is if 
        the button is let go of and the else part of the if statement is invoked.
        */
        if(keyDown(XK_9)) { //if pressed
            if(was_distance_or_fov_button_pressed == false) { //if not pressed before
                distance_or_fov_button = !distance_or_fov_button; //flip values
                was_distance_or_fov_button_pressed = true; //say that this has now been pressed before
            }
        } else { //if is not pressed
            was_distance_or_fov_button_pressed = false; //say has not been pressed before
        }

        /*
        We now need to do the same thing for the recoil buttons.
        */

        if(keyDown(MoreVirt_button)) { //if Key pressed, have less vertical recoil
            if(was_norecoilPitchStrength_buttonMore == false) { 
                norecoilPitchStrength = norecoilPitchStrength - 0.1f;
                was_norecoilPitchStrength_buttonMore = true;
            }
        } else {
            was_norecoilPitchStrength_buttonMore = false;
        }

        if(keyDown(LessVirt_button)) { //if Key pressed, have more vertical recoil
            if(was_norecoilPitchStrength_buttonLess == false) { 
                norecoilPitchStrength += 0.1f;
                was_norecoilPitchStrength_buttonLess = true;
            }
        } else {
            was_norecoilPitchStrength_buttonLess = false;
        }

        if(keyDown(MoreHorz_button)) { //if Key pressed, have less horizontal recoil
            if(was_norecoilYawStrength_buttonMore == false) { 
                norecoilYawStrength = norecoilYawStrength - 0.1f;
                was_norecoilYawStrength_buttonMore = true;
            }
        } else {
            was_norecoilYawStrength_buttonMore = false;
        }

        if(keyDown(LessHorz_button)) { //if Key pressed, have more horizontal recoil
            if(was_norecoilYawStrength_buttonLess == false) { 
                norecoilYawStrength += 0.1f;
                was_norecoilYawStrength_buttonLess = true;
            }
        } else {
            was_norecoilYawStrength_buttonLess = false;
        }

        /*
        This is the end of the button section and now the start of the Menu section. In linux we can set a window to "always on top".
        This makes it very nice to make a menu. The reason we need this menu is because of the fact that we have some togglable values
        that are hard to keep track of. This is very simple code that prints our values of the toggalable buttons.

        The if statement at the beging is just so the menu does not update too often to avoid lag. It will only update every 200 loops.
        */

        
        if (loopAmount % 200 == 0) {
            loopAmount = 0;
            system("clear");
            std::cout << "+-----------------------------+" << std::endl;
            std::cout << "| Virt Recoil Strength: "<< norecoilPitchStrength << "   |"  << std::endl;
            std::cout << "| Horz Recoil Strength: "<< norecoilYawStrength   << "   |"  << std::endl;
            if (distance_or_fov_button == false) {
                std::cout << "| Aimbot Targeting: Distance  |"  << std::endl;
            } else {
                std::cout << "| Aimbot Targeting: FOV       |"  << std::endl;
            }
            std::cout << "+-----------------------------+\n";
        }
        

        /*
        So this is the Anti-recoil section. The code is simple and self contained.

        To anyone who is trying to make this any more efficient might see that we have gotton some of these values
        before, but I also had this idea but its not worth it. The pitch and yaw change every so slightly at these 
        milisecond times and can cause your pitch and yaw to shake and other unwanted behavior. Just get them again.
        */
        
        try 
        {   

            float punchPitch = mem::ReadFloat(LocalPlayer + offsets::VEC_PUNCH_WEAPON_ANGLE);
            if (punchPitch != 0)
            {
                const double pitch = mem::ReadFloat(LocalPlayer + offsets::VIEW_ANGLE);
                const double punchPitchDelta = (punchPitch - m_previousPunchPitch) * norecoilPitchStrength;
                if (pitch - punchPitchDelta > 90 || pitch - punchPitchDelta < -90) {
                    continue;
                }

                mem::WriteFloat(LocalPlayer + offsets::VIEW_ANGLE, pitch - punchPitchDelta);
                m_previousPunchPitch = punchPitch;
            }
            
            
            float punchYaw = mem::ReadFloat(LocalPlayer + offsets::VEC_PUNCH_WEAPON_ANGLE + sizeof(float));
            if (punchYaw != 0)
            {
                const double yaw = mem::ReadFloat(LocalPlayer + offsets::VIEW_ANGLE + sizeof(float));
                const double punchYawDelta = (punchYaw - m_previousPunchYaw) * norecoilYawStrength;
                if (yaw - punchYawDelta > 180 || yaw - punchYawDelta < -180) {
                    continue;
                }

                mem::WriteFloat(LocalPlayer + offsets::VIEW_ANGLE + sizeof(float), yaw - punchYawDelta);
                m_previousPunchYaw = punchYaw;
            }
                
        } catch (...)
        {
            //if error (like that entity is dead) then this runs
        }

        /*
        Next is the aimbot. For the sake of keeping the code relitivly clean, I am a going to write out the explenation here.
        The first thing we do is setup a "timesLooped" this acts as an index for our entityies similer to how "i" is used in for loops
        We also have a "LockedEntity" witch is the index of the player we are locked onto, as you can see if no key is down we reset the lock as that means the player let go and the lock should be reset
        Then we have some conditions in our while loop, that the LockedEntity is 71 (a locked entity of 71 means that no entity should be targeted) that some key aimbot key is down and that variable loop is true
        The first thing inside the loop is a if statment breaking the loop if we are above 70 entitys (no players exist over 70 entitys)
        Then we look at weather we have toggled on a sort method of distance of FOV (for close range aimbot we auto use close range as its wayyy better (trust))
        Depending on the method we find the distance/FOV of the current entity and match it with the unsorted list to get the original entity index
        If we reach a point were the entity we are on has a value of 654321 (the designated value we established was for entities that did not exist) we set loop to false witch will soon break the loop
        If the loop is false (it could be like that from the for loops described above) we exit the loop
        We use the LocalPlayer to get new positions for us
        we get the BasePointer again and the position of the current entity were on
        we then determin were we would need to look to look at the player
        we do some math to figure our how much we would need to move the camara to achive looking at the player
        we get the team of the player
        then based on what key we are pressing and what settings we have configured we decide weather or not to lock onto the entity or skip it
        */

        int timesLooped = 0;

        //this if loop will run if no key is currently down, if no key is down then we unlock the aimbot
        if (someKeyDown == false) {
            LockedEntity = 71;
        }

        bool loop = true; //used to break the loop

        //if LockedEntity is 71 then it means that we are not locked onto anyone so we need to find someone
        //witch is why we would need to loop through all of the entitys and ectually execute this loop
        while(LockedEntity == 71 && someKeyDown == true && loop == true) { //aimbot selection loop
            //players will not exist over 70 so break the loop
            if (timesLooped >= 70) { 
                break;
            }

            //This will be the entity we will target this loop
            int target = 0; //the entity number of the target

            //depending on the state of the distance/FOV button target different entitys (or of were using close range)
            if (distance_or_fov_button == false || CloseRangeDown == true) { //sort by distance 
                for (int i = 0; i < 70; i++) { //loop though all 70 enemys
                    if (SortedDistance[timesLooped] == distance[i]) { //if the current distance or the sorted distance matches the distance of the unsorted distance 
                        if (SortedDistance[timesLooped] == 654321) { //(if its this we dont want to target it as its probobly us or dead)
                            loop = false;
                        }
                        target = i; //set matches to the index of the unsorted distance
                    }
                }
            } else { //sort by FOV
                for (int i = 0; i < 70; i++) { //loop though all 70 enemys
                    if (SortedFOV[timesLooped] == FOV[i]) { //if the current angle or the sorted angle matches the angle of the unsorted angle
                        if (SortedFOV[timesLooped] == 654321) { //(if its this we dont want to target it as its probobly us or dead)
                            loop = false;
                        }
                        target = i; //set matches to the index of the unsorted angle
                    }
                }
            }

            if (loop == false) {
                break;
            }

            //Gets our position
            float My_x = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN);
            float My_y = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN + sizeof(float));
            float My_z = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN + sizeof(float) * 2);

            //Gets the basePointer of the entity we are targeting
            long basePointer = mem::ReadLong(offsets::REGION + offsets::ENTITY_LIST + ((target + 1) << 5));

            //Gets enemy position
            const float Entity_x = mem::ReadFloat(basePointer + offsets::LOCAL_ORIGIN);
            const float Entity_y = mem::ReadFloat(basePointer + offsets::LOCAL_ORIGIN + sizeof(float));
            float Entity_z = mem::ReadFloat(basePointer + offsets::LOCAL_ORIGIN + sizeof(float) * 2);

            if (aimbot_vertical_aim == true) {
                if (!(vertical_aimbot_hipfire_only == true && CloseRangeDown == false)) { //elinates when we have virt recoil only on hipfire and no hipfire is enabled
                    Entity_z = Entity_z - (distance[target]/10);
                } 
            }

            //the pitch and yaw that would be needed to look at the enemy
            double desiredViewAngleYaw = calculateDesiredYaw(My_x, My_y, Entity_x, Entity_y);
            double desiredViewAnglePitch = calculateDesiredPitch(My_x, My_y, My_z, Entity_x, Entity_y, Entity_z);

            //gets our current angles (side to side)
            const double pitch = mem::ReadFloat(LocalPlayer + offsets::VIEW_ANGLE);
            const double pitchAngleDelta = calculatePitchAngleDelta(pitch, desiredViewAnglePitch);
            const double pitchAngleDeltaAbs = abs(pitchAngleDelta);

            //gets our current angles (up and down)
            const double yaw = mem::ReadFloat(LocalPlayer + offsets::VIEW_ANGLE + sizeof(float));
            const double angleDelta = calculateAngleDelta(yaw, desiredViewAngleYaw);
            const double angleDeltaAbs = abs(angleDelta);
 
            //Gets the team that the enemy is on
            int Entity_team = mem::ReadInt(basePointer + offsets::TEAM_NUMBER);

            //if were on the same team move onto next player
            if (LocalPlayer_Team == Entity_team) {
                timesLooped++;
                continue;
            }

            //checks if the player is downed
            short result = mem::ReadShort(basePointer + offsets::BLEEDOUT_STATE);
            if (result > 0) {
                timesLooped++;
                continue;
            }

            //If you know what your doing, this is for debugging: std::cout << "pitchAngleDeltaAbs: " << pitchAngleDeltaAbs <<  " angleDeltaAbs: " << angleDeltaAbs << std::endl;
            //this will be used to determin if we should skip this entity based on the FOV
            bool shouldContinue = false;

            //if the requirements are met for any button then shouldContinue is makred true

            float distdropoff = FOV_divide_close*((1000 - distance[target])/close_distance_divide_falloff); //FOV fallout calculations for close range
            if ((1000 - distance[target]) > 300) { // if over 700m away
                float distdropoff = 0.29; //FOV will remain the same (~14) if your farther than 700m
            }

            if(CloseRangeDown == true) { //if close range aimbot button is down
                if (pitchAngleDeltaAbs > close_range_FOV*(distdropoff)) {  //just a ratio for better FOV rartgeting
                    //bad
                } else if (angleDeltaAbs > close_range_FOV) {
                    //bad
                } else {
                    //good
                    shouldContinue = true;
                }
            } else if(MidRangeDown == true) { //if mid range aimbot button is down
                if (pitchAngleDeltaAbs > mid_range_FOV*FOV_divide) {
                    //bad
                } else if (angleDeltaAbs > mid_range_FOV) {
                    //bad
                } else {
                    //good
                    shouldContinue = true;
                }
            } else if(LongRangeDown == true) { //if long range aimbot button is down
                if (pitchAngleDeltaAbs > long_range_FOV*FOV_divide) {
                    //bad
                } else if (angleDeltaAbs > long_range_FOV) {
                    //bad
                } else {
                    //good
                    shouldContinue = true;
                }
            }

            if (shouldContinue == false) { //if the requirements are not met then skip and move on
                timesLooped++;
                continue;
            }

            //If we reach here then it means that the enemy is alive, in range, in FOV and is 
            //sorted the way we want to so we lock onto the entity
            //If you know what your doing, this is for debugging: std::cout << "pitch: " << pitchAngleDeltaAbs <<  " angle: " << angleDeltaAbs << std::endl;
            LockedEntity = target;
            break;
        }

        /*
        Now its time to do it all over again, it may seem counterproductive to get the same values we just got in the avove while loop again but that code only runs
        once a button is pressed for the first time. Whenever we are locked onto a target we need to get some new values but a lot of the logic is still the same
        */

        //if a key is not down then skip
        if (someKeyDown == false) {
            continue;
        }

        //if not target selected then skip
        if (LockedEntity == 71) { //if were not targeting skip
            continue;
        }

        int sense = 100; //default value

        //changes default value depending on what button is down
        if (CloseRangeDown == true) {
            sense = close_range_smoothing;
        } else if (MidRangeDown == true) {
            sense = mid_range_smoothing;
        } else if (LongRangeDown == true) {
            sense = long_range_smoothing;
        }

        /////////////////Were we actually write/////////////////

        //Get our position
        float mx_write = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN);
        float my_write = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN + sizeof(float));
        float mz_write = mem::ReadFloat(LocalPlayer + offsets::LOCAL_ORIGIN + sizeof(float) * 2);

        //Get the base pointer of the entity we are targeting
        long basePointer_write = mem::ReadLong(offsets::REGION + offsets::ENTITY_LIST + ((LockedEntity + 1) << 5));

        //Get the position of the entity we are targeting
        const float Entity_x = mem::ReadFloat(basePointer_write + offsets::LOCAL_ORIGIN);
        const float Entity_y = mem::ReadFloat(basePointer_write + offsets::LOCAL_ORIGIN + sizeof(float));
        const float Entity_z = mem::ReadFloat(basePointer_write + offsets::LOCAL_ORIGIN + sizeof(float) * 2);

        //were to look to look at the entity
        double desiredViewAngleYaw = calculateDesiredYaw(mx_write, my_write, Entity_x, Entity_y);
        double desiredViewAnglePitch = calculateDesiredPitch(mx_write, my_write, mz_write, Entity_x, Entity_y, Entity_z);

        //get our current angles (up and down)
        const double pitch = mem::ReadFloat(LocalPlayer + offsets::VIEW_ANGLE);
        const double pitchAngleDelta = calculatePitchAngleDelta(pitch, desiredViewAnglePitch);
        const double pitchAngleDeltaAbs = abs(pitchAngleDelta);

        //get our current angles (side to side)
        const double yaw = mem::ReadFloat(LocalPlayer + offsets::VIEW_ANGLE + sizeof(float));
        const double angleDelta = calculateAngleDelta(yaw, desiredViewAngleYaw);
        const double angleDeltaAbs = abs(angleDelta);
        
        //get the last time the entity was visible
        float LastVisTime_write = mem::ReadFloat(basePointer_write + offsets::LAST_VISIBLE_TIME);
        const bool isVisible_write = LastVisTime_write > m_lastVisibleTime[LockedEntity];
        m_lastVisibleTime[LockedEntity] = LastVisTime_write;
        
        //the variable LastVisTime_write updates too slowly and we read the same value multiple times so we need to give it some buffer time 
        //the consequenses of this if an enemy is actually not visible would be ~20 miliseconds aka not preceptible to the human brain.
        if (!isVisible_write) { //if not visible  
            if (loopsSinceLastVisible < 20) { //if they have been visible 20 loops ago
                loopsSinceLastVisible++; 
            } else {
                continue;
            }
        } else if (isVisible_write) {
            loopsSinceLastVisible = 0; //only if truely visible
        }
        
        //calculates how much to move after smoothing out the aimbot
        double angleDeltaAfterSense = angleDelta / sense; //Dev Note: 10 FOV = 0.65, on tagrte = 0.001
            
        //Calculates deadzone of when not to write because its not really needed
        if (angleDeltaAfterSense < deadZone && angleDeltaAfterSense > -1*(deadZone)) {
            continue;
        }

        //Finalises the angle to write
        double newYaw = flipYawIfNeeded(yaw + angleDeltaAfterSense);
        //Makes sure that we are not writing a value that is not possible
        if (newYaw > 180 || newYaw < -180) {
            continue;
        }

        //Writes the angle (finally)
        mem::WriteFloat(LocalPlayer + offsets::VIEW_ANGLE + sizeof(float), newYaw);

        //if vertical aimbot is enabled in the settings
        if (aimbot_vertical_aim == true) {
            double newPitch = flipPitchIfNeeded(pitch + (pitchAngleDelta / (sense*vertical_smoothing))); //flipPitchIfNeeded, no idea if it works 100%, GPT made it
            if (newPitch > 90 || newPitch < -90) {
                continue;
            }

            if (vertical_aimbot_hipfire_only == true && CloseRangeDown == false) { //checks if the option for having it only work for close range is enabled
                continue;
            }

            mem::WriteFloat(LocalPlayer + offsets::VIEW_ANGLE, newPitch); //writes virtical aimbot
        }//If statement
    }//While loop   
}//Main loop
