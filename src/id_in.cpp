//
//  ID Engine
//  ID_IN.c - Input Manager
//  v1.0d1
//  By Jason Blochowiak
//

//
//  This module handles dealing with the various input devices
//
//  Depends on: Memory Mgr (for demo recording), Sound Mgr (for timing stuff),
//              User Mgr (for command line parms)
//
//  Globals:
//      LastScan - The keyboard scan code of the last key pressed
//      LastASCII - The ASCII value of the last key pressed
//  DEBUG - there are more globals
//

#include "wl_def.h"

/*
=============================================================================

                    GLOBAL VARIABLES

=============================================================================
*/

//
// configuration variables
//
boolean MousePresent;
boolean ModernMouseKeyboard = true;
boolean forcegrabmouse = ModernMouseKeyboard;

//  Global variables
volatile boolean Keyboard[SDL_NUM_SCANCODES];
volatile boolean Paused;
volatile char LastASCII;
volatile ScanCode LastScan;
bool GameControllerButtons[gcbt_Max];
int GameControllerLeftStick[2] = {0, 0};
int GameControllerRightStick[2] = {0, 0};
int JoyNumButtons;

/*
=============================================================================

                    LOCAL VARIABLES

=============================================================================
*/
byte ASCIINames[] = // Unshifted ASCII for scan codes       // TODO: keypad
    {
        //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
        0,   0,   0,   0,   0,   0,   0,   0,   8,   9,   0,   0,   0,   13,  0,   0,   // 0
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   27,  0,   0,   0,   // 1
        ' ', 0,   0,   0,   0,   0,   0,   39,  0,   0,   '*', '+', ',', '-', '.', '/', // 2
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0,   ';', 0,   '=', 0,   0,   // 3
        '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', // 4
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', 92,  ']', 0,   0,   // 5
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 6
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0    // 7
};
byte ShiftNames[] = // Shifted ASCII for scan codes
    {
        //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
        0,   0,   0,   0,   0,   0,   0,   0,   8,   9,   0,   0,   0,   13,  0,   0,   // 0
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   27,  0,   0,   0,   // 1
        ' ', 0,   0,   0,   0,   0,   0,   34,  0,   0,   '*', '+', '<', '_', '>', '?', // 2
        ')', '!', '@', '#', '$', '%', '^', '&', '*', '(', 0,   ':', 0,   '+', 0,   0,   // 3
        '~', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', // 4
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', 0,   0,   // 5
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 6
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0    // 7
};
byte SpecialNames[] = // ASCII for 0xe0 prefixed codes
    {
        //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
        0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0, // 0
        0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 13, 0, 0, 0, // 1
        0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0, // 2
        0, 0, 0, 0, 0, '/', 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, // 3
        0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0, // 4
        0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0, // 5
        0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0, // 6
        0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0  // 7
};

static KeyboardDef KbdDefs = {
    sc_Control,    // button0
    sc_Alt,        // button1
    sc_Home,       // upleft
    sc_UpArrow,    // up
    sc_PgUp,       // upright
    sc_LeftArrow,  // left
    sc_RightArrow, // right
    sc_End,        // downleft
    sc_DownArrow,  // down
    sc_PgDn        // downright
};

static SDL_Joystick *Joystick;
static int JoyNumHats;
static SDL_GameController *GameController = nullptr;
static bool GrabInput = false;

static boolean IN_Started;

static Direction DirTable[] = // Quick lookup for total direction
    {dir_NorthWest, dir_North, dir_NorthEast, dir_West, dir_None, dir_East, dir_SouthWest, dir_South, dir_SouthEast};

///////////////////////////////////////////////////////////////////////////
//
//  INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//      mouse driver
//
///////////////////////////////////////////////////////////////////////////
static int INL_GetMouseButtons(void)
{
    int buttons = SDL_GetMouseState(NULL, NULL);
    bool middlePressed = (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    bool rightPressed = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    buttons &= ~(SDL_BUTTON(SDL_BUTTON_MIDDLE) | SDL_BUTTON(SDL_BUTTON_RIGHT));
    if (middlePressed)
        buttons |= 1 << 2;
    if (rightPressed)
        buttons |= 1 << 1;

    return buttons;
}

///////////////////////////////////////////////////////////////////////////
//
//  IN_GetJoyDelta() - Returns the relative movement of the specified
//      joystick (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void IN_GetJoyDelta(int *dx, int *dy)
{
    if (!Joystick)
    {
        *dx = *dy = 0;
        return;
    }

    SDL_JoystickUpdate();

    int x = SDL_JoystickGetAxis(Joystick, 0) >> 8;
    int y = SDL_JoystickGetAxis(Joystick, 1) >> 8;

    if (param_joystickhat != -1)
    {
        uint8_t hatState = SDL_JoystickGetHat(Joystick, param_joystickhat);
        if (hatState & SDL_HAT_RIGHT)
            x += 127;
        else if (hatState & SDL_HAT_LEFT)
            x -= 127;
        if (hatState & SDL_HAT_DOWN)
            y += 127;
        else if (hatState & SDL_HAT_UP)
            y -= 127;

        if (x < -128)
            x = -128;
        else if (x > 127)
            x = 127;

        if (y < -128)
            y = -128;
        else if (y > 127)
            y = 127;
    }

    *dx = x;
    *dy = y;
}

#if 0 // Unused function
///////////////////////////////////////////////////////////////////////////
//
//  IN_GetJoyFineDelta() - Returns the relative movement of the specified
//      joystick without dividing the results by 256 (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void IN_GetJoyFineDelta(int *dx, int *dy)
{
    if (!Joystick)
    {
        *dx = 0;
        *dy = 0;
        return;
    }

    SDL_JoystickUpdate();
    int x = SDL_JoystickGetAxis(Joystick, 0);
    int y = SDL_JoystickGetAxis(Joystick, 1);

    if (x < -128)
        x = -128;
    else if (x > 127)
        x = 127;

    if (y < -128)
        y = -128;
    else if (y > 127)
        y = 127;

    *dx = x;
    *dy = y;
}
#endif

/*
===================
=
= IN_JoyButtons
=
===================
*/

int IN_JoyButtons()
{
    if (!Joystick)
        return 0;

    SDL_JoystickUpdate();

    int res = 0;
    for (int i = 0; i < JoyNumButtons && i < 32; i++)
        res |= SDL_JoystickGetButton(Joystick, i) << i;
    return res;
}

boolean IN_JoyPresent()
{
    return Joystick != NULL;
}

static void processEvent(SDL_Event *event)
{
    switch (event->type)
    {
    // exit if the window is closed
    case SDL_QUIT:
        Quit(NULL);

    // check for keypresses
    case SDL_KEYDOWN: {
        if (event->key.keysym.scancode == SDL_SCANCODE_SCROLLLOCK || event->key.keysym.sym == SDL_SCANCODE_F12)
        {
            GrabInput = !GrabInput;
            SDL_SetRelativeMouseMode(GrabInput ? SDL_TRUE : SDL_FALSE);
            return;
        }

        LastScan = event->key.keysym.scancode;
        SDL_Keymod mod = SDL_GetModState();
        if (Keyboard[sc_Alt])
        {
            if (LastScan == SDL_SCANCODE_F4)
                Quit(NULL);
        }

        if (LastScan == SDL_SCANCODE_KP_ENTER)
            LastScan = SDL_SCANCODE_RETURN;
        else if (LastScan == SDL_SCANCODE_RSHIFT)
            LastScan = SDL_SCANCODE_LSHIFT;
        else if (LastScan == SDL_SCANCODE_RALT)
            LastScan = SDL_SCANCODE_LALT;
        else if (LastScan == SDL_SCANCODE_RCTRL)
            LastScan = SDL_SCANCODE_LCTRL;
        else
        {
            if ((mod & KMOD_NUM) == 0)
            {
                switch (LastScan)
                {
                case SDL_SCANCODE_KP_2:
                    LastScan = SDL_SCANCODE_DOWN;
                    break;
                case SDL_SCANCODE_KP_4:
                    LastScan = SDL_SCANCODE_LEFT;
                    break;
                case SDL_SCANCODE_KP_6:
                    LastScan = SDL_SCANCODE_RIGHT;
                    break;
                case SDL_SCANCODE_KP_8:
                    LastScan = SDL_SCANCODE_UP;
                    break;
                }
            }
        }

        int sym = event->key.keysym.sym;
        if (sym >= 'a' && sym <= 'z')
            sym -= 32; // convert to uppercase

        if (mod & (KMOD_SHIFT | KMOD_CAPS))
        {
            if (sym < lengthof(ShiftNames) && ShiftNames[sym])
                LastASCII = ShiftNames[sym];
        }
        else
        {
            if (sym < lengthof(ASCIINames) && ASCIINames[sym])
                LastASCII = ASCIINames[sym];
        }

        if (LastScan < SDL_NUM_SCANCODES)
        {
            Keyboard[LastScan] = 1;
        }
        if (LastScan == SDL_SCANCODE_PAUSE)
            Paused = true;
        break;
    }

    case SDL_KEYUP: {
        int key = event->key.keysym.scancode;
        if (key == SDL_SCANCODE_KP_ENTER)
            key = SDL_SCANCODE_RETURN;
        else if (key == SDL_SCANCODE_RSHIFT)
            key = SDL_SCANCODE_LSHIFT;
        else if (key == SDL_SCANCODE_RALT)
            key = SDL_SCANCODE_LALT;
        else if (key == SDL_SCANCODE_RCTRL)
            key = SDL_SCANCODE_LCTRL;
        else
        {
            if ((SDL_GetModState() & KMOD_NUM) == 0)
            {
                switch (key)
                {
                case SDL_SCANCODE_KP_2:
                    key = SDL_SCANCODE_DOWN;
                    break;
                case SDL_SCANCODE_KP_4:
                    key = SDL_SCANCODE_LEFT;
                    break;
                case SDL_SCANCODE_KP_6:
                    key = SDL_SCANCODE_RIGHT;
                    break;
                case SDL_SCANCODE_KP_8:
                    key = SDL_SCANCODE_UP;
                    break;
                }
            }
        }

        if (key < SDL_NUM_SCANCODES)
        {
            Keyboard[key] = 0;
        }
        break;
    }

    // check for game controller events
    case SDL_CONTROLLERDEVICEADDED: {
        if (!GameController)
        {
            int id = event->cdevice.which;
            if (SDL_IsGameController(id))
            {
                GameController = SDL_GameControllerOpen(id);
                LOG_Infof("SDL GameController '%s' connected", SDL_GameControllerName(GameController));
            }
        }
        break;
    }
    case SDL_CONTROLLERDEVICEREMOVED: {
        if (GameController)
        {
            LOG_Infof("SDL GameController '%s' disconnected", SDL_GameControllerName(GameController));
            SDL_GameControllerClose(GameController);
            GameController = nullptr;
        }
        break;
    }
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        if (GameController)
        {
            GameControllerButtons[event->cbutton.button] = (bool)event->cbutton.state == SDL_PRESSED;
        }
        break;
    case SDL_CONTROLLERAXISMOTION:
        if (GameController)
        {
            if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
                GameControllerLeftStick[0] = event->caxis.value >> 8;
            if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
                GameControllerLeftStick[1] = event->caxis.value >> 8;
            if (event->caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
                GameControllerRightStick[0] = event->caxis.value >> 8;
            if (event->caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
                GameControllerRightStick[1] = event->caxis.value >> 8;

            if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
                GameControllerButtons[gcbt_LeftShoulder] = event->caxis.value == 32767;
            if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
                GameControllerButtons[gcbt_RightShoulder] = event->caxis.value == 32767;
        }
        break;
    }
}

void IN_WaitAndProcessEvents()
{
    SDL_Event event;
    if (!SDL_WaitEvent(&event))
        return;
    do
    {
        processEvent(&event);
    } while (SDL_PollEvent(&event));
}

void IN_ProcessEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        processEvent(&event);
    }
}

///////////////////////////////////////////////////////////////////////////
//
//  IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void IN_Startup(void)
{
    if (IN_Started)
        return;

    IN_ClearKeysDown();

    if (param_joystickindex >= 0 && param_joystickindex < SDL_NumJoysticks())
    {
        if (!SDL_IsGameController(param_joystickindex))
        {
            Joystick = SDL_JoystickOpen(param_joystickindex);
            if (Joystick)
            {
                JoyNumButtons = SDL_JoystickNumButtons(Joystick);
                if (JoyNumButtons > 32)
                    JoyNumButtons = 32; // only up to 32 buttons are supported
                JoyNumHats = SDL_JoystickNumHats(Joystick);
                if (param_joystickhat < -1 || param_joystickhat >= JoyNumHats)
                    Quit("The joystickhat param must be between 0 and %i!", JoyNumHats - 1);
            }
        }
    }

    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

    if (fullscreen || forcegrabmouse)
    {
        GrabInput = true;
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }

    // I didn't find a way to ask libSDL whether a mouse is present, yet...
    MousePresent = true;

    IN_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//  IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void IN_Shutdown(void)
{
    if (!IN_Started)
        return;

    if (Joystick)
        SDL_JoystickClose(Joystick);

    if (GameController)
        SDL_GameControllerClose(GameController);

    LOG_Infof("SDL Joystick and GameController resources destroyed");

    IN_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//  IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void IN_ClearKeysDown(void)
{
    LastScan = sc_None;
    LastASCII = key_None;
    memset((void *)Keyboard, 0, sizeof(Keyboard));
}

///////////////////////////////////////////////////////////////////////////
//
//  IN_ReadControl() - Reads the device associated with the specified
//      player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void IN_ReadControl(int player, ControlInfo *info)
{
    word buttons;
    int dx, dy;
    Motion mx, my;

    dx = dy = 0;
    mx = my = motion_None;
    buttons = 0;

    IN_ProcessEvents();

    // read input from the game controller
    if (GameControllerButtons[gcbt_DpadUp])
        my = motion_Up;
    else if (GameControllerButtons[gcbt_DpadDown])
        my = motion_Down;

    if (GameControllerButtons[gcbt_DpadLeft])
        mx = motion_Left;
    else if (GameControllerButtons[gcbt_DpadRight])
        my = motion_Right;

    if (GameControllerLeftStick[1] < -SENSITIVE)
        my = motion_Up;
    else if (GameControllerLeftStick[1] > SENSITIVE)
        my = motion_Down;

    if (GameControllerLeftStick[0] < -SENSITIVE)
        mx = motion_Left;
    else if (GameControllerLeftStick[0] > SENSITIVE)
        mx = motion_Right;

    if (GameControllerButtons[gcbt_Start] || GameControllerButtons[gcbt_A])
        buttons += 1 << 0;
    if (GameControllerButtons[gcbt_Back] || GameControllerButtons[gcbt_B])
        buttons += 1 << 1;

    // read input from the keyboard
    if (Keyboard[KbdDefs.upleft])
        mx = motion_Left, my = motion_Up;
    else if (Keyboard[KbdDefs.upright])
        mx = motion_Right, my = motion_Up;
    else if (Keyboard[KbdDefs.downleft])
        mx = motion_Left, my = motion_Down;
    else if (Keyboard[KbdDefs.downright])
        mx = motion_Right, my = motion_Down;

    if (Keyboard[KbdDefs.up])
        my = motion_Up;
    else if (Keyboard[KbdDefs.down])
        my = motion_Down;

    if (Keyboard[KbdDefs.left])
        mx = motion_Left;
    else if (Keyboard[KbdDefs.right])
        mx = motion_Right;

    if (Keyboard[KbdDefs.button0])
        buttons += 1 << 0;
    if (Keyboard[KbdDefs.button1])
        buttons += 1 << 1;

    dx = mx * 127;
    dy = my * 127;

    info->x = dx;
    info->xaxis = mx;
    info->y = dy;
    info->yaxis = my;
    info->button0 = (buttons & (1 << 0)) != 0;
    info->button1 = (buttons & (1 << 1)) != 0;
    info->button2 = (buttons & (1 << 2)) != 0;
    info->button3 = (buttons & (1 << 3)) != 0;
    info->dir = DirTable[((my + 1) * 3) + (mx + 1)];
}

///////////////////////////////////////////////////////////////////////////
//
//  IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

boolean btnstate[NUMBUTTONS];

void IN_StartAck(void)
{
    IN_ProcessEvents();
    //
    // get initial state of everything
    //
    IN_ClearKeysDown();
    memset(GameControllerButtons, 0, sizeof(bool));
    memset(btnstate, 0, sizeof(btnstate));

    int buttons = IN_JoyButtons() << 4;

    if (MousePresent)
        buttons |= IN_MouseButtons();

    for (int i = 0; i < NUMBUTTONS; i++, buttons >>= 1)
        if (buttons & 1)
            btnstate[i] = true;
}

boolean IN_CheckAck(void)
{
    IN_ProcessEvents();
    //
    // see if something has been pressed
    //
    if (LastScan)
        return true;

    for (int i = 0; i < gcbt_Max; i++)
    {
        if (GameControllerButtons[i])
            return true;
    }

    int buttons = IN_JoyButtons() << 4;

    if (MousePresent)
        buttons |= IN_MouseButtons();

    for (int i = 0; i < NUMBUTTONS; i++, buttons >>= 1)
    {
        if (buttons & 1)
        {
            if (!btnstate[i])
            {
                // Wait until button has been released
                do
                {
                    IN_WaitAndProcessEvents();
                    buttons = IN_JoyButtons() << 4;

                    if (MousePresent)
                        buttons |= IN_MouseButtons();
                } while (buttons & (1 << i));

                return true;
            }
        }
        else
            btnstate[i] = false;
    }

    return false;
}

void IN_Ack(void)
{
    IN_StartAck();

    do
    {
        IN_WaitAndProcessEvents();
    } while (!IN_CheckAck());
}

///////////////////////////////////////////////////////////////////////////
//
//  IN_UserInput() - Waits for the specified delay time (in ticks) or the
//      user pressing a key or a mouse button. If the clear flag is set, it
//      then either clears the key or waits for the user to let the mouse
//      button up.
//
///////////////////////////////////////////////////////////////////////////
boolean IN_UserInput(longword delay)
{
    longword lasttime;

    lasttime = GetTimeCount();
    IN_StartAck();
    do
    {
        IN_ProcessEvents();
        if (IN_CheckAck())
            return true;
        SDL_Delay(5);
    } while (GetTimeCount() - lasttime < delay);
    return (false);
}

//===========================================================================

/*
===================
=
= IN_MouseButtons
=
===================
*/
int IN_MouseButtons(void)
{
    if (MousePresent)
        return INL_GetMouseButtons();
    else
        return 0;
}

bool IN_IsInputGrabbed()
{
    return GrabInput;
}
