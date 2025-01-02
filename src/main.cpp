#include "main.h"

#include <imgui.h>

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include "camera.h"

// Required
JAPIModMeta __stdcall GetModMeta() {
    static JAPIModMeta meta = {
        "Free Camera", // Name
        "FreeCam", // GUID
        "Kapilarny & Kojo Bailey", // Author
        "1.0.0", // Version
        "Frees the camera!" // Description
    };

    return meta;
}

struct mat4x4 {
    float m[4][4];
};

struct vec3f {
    float x, y, z;
};

struct {
    bool left = false;
    bool right = false;
    bool forward = false;
    bool backward = false;
    bool up = false;
    bool down = false;
} KeyPressed;

// vec3f free_cam_pos = {69, 69, 69};
// vec3f free_cam_rot = {0, 0, 0};

static Camera camera({69, 69, 69});

static int res_width = 0;
static int res_height = 0;
static bool disable_ui = false;

// this shouldnt be heap allocated, but oh well
static char* LEFT_KEY = new char[2];
static char* RIGHT_KEY = new char[2];
static char* FORWARD_KEY = new char[2];
static char* BACKWARD_KEY = new char[2];
static char* UP_KEY = new char[2];
static char* DOWN_KEY = new char[2];
static char* BLOCK_CAMERA_KEY = new char[2];

static const char* LEFT_KEY_DESC = "KeyLeft";
static const char* RIGHT_KEY_DESC = "KeyRight";
static const char* FORWARD_KEY_DESC = "KeyForward";
static const char* BACKWARD_KEY_DESC = "KeyBackward";
static const char* UP_KEY_DESC = "KeyUp";
static const char* DOWN_KEY_DESC = "KeyDown";
static const char* BLOCK_CAMERA_KEY_DESC = "BlockCameraKey";

bool free_cam_enabled = false;
bool movement_blocked = false;

typedef LRESULT(__fastcall* ASBR_HWNDProcCallback)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ASBR_HWNDProcCallback ASBR_HWNDProcCallback_Original;

LRESULT __fastcall ASBR_HWNDProcCallback_Hook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // if key is pressed, toggle free cam
    if (msg == WM_KEYUP && wParam == VK_F5) {
        free_cam_enabled = !free_cam_enabled;
        // JAPI_LogInfo("Free cam: " + std::to_string(free_cam_enabled));
    
        if (free_cam_enabled) {
            // Make the cursor invisible
            ShowCursor(FALSE);

            // Get the window size
            RECT rect;
            GetWindowRect(hWnd, &rect);

            // Calculate the center of the window
            int centerX = rect.left + (rect.right - rect.left) / 2;
            int centerY = rect.top + (rect.bottom - rect.top) / 2;

            // Set the mouse position to the center
            SetCursorPos(centerX, centerY);
        }
    }

    if (!free_cam_enabled) {
        ShowCursor(TRUE);
        return ASBR_HWNDProcCallback_Original(hWnd, msg, wParam, lParam);
    }

    if(msg == WM_KEYDOWN && wParam == BLOCK_CAMERA_KEY[0]) {
        movement_blocked = !movement_blocked;
    }

    if(movement_blocked) {
        ShowCursor(TRUE);
        return ASBR_HWNDProcCallback_Original(hWnd, msg, wParam, lParam);
    }

    // Record camera movement inputs.
    if (msg == WM_KEYDOWN) {
        if (wParam == RIGHT_KEY[0])    KeyPressed.right = true;
        if (wParam == LEFT_KEY[0])     KeyPressed.left = true;
        if (wParam == FORWARD_KEY[0])  KeyPressed.forward = true;
        if (wParam == BACKWARD_KEY[0]) KeyPressed.backward = true;
        if (wParam == UP_KEY[0])    KeyPressed.right = true;
        if (wParam == DOWN_KEY[0])     KeyPressed.left = true;
    }

    if (msg == WM_KEYUP) {
        if (wParam == RIGHT_KEY[0])    KeyPressed.right = false;
        if (wParam == LEFT_KEY[0])     KeyPressed.left = false;
        if (wParam == FORWARD_KEY[0])  KeyPressed.forward = false;
        if (wParam == BACKWARD_KEY[0]) KeyPressed.backward = false;
        if (wParam == UP_KEY[0])    KeyPressed.right = false;
        if (wParam == DOWN_KEY[0])     KeyPressed.left = false;
    }

    if (msg == WM_MOUSEMOVE) {
        // Get the mouse position
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        // Get the window size
        RECT rect;
        GetWindowRect(hWnd, &rect);

        // Calculate the center of the window
        int centerX = rect.left + (rect.right - rect.left) / 2;
        int centerY = rect.top + (rect.bottom - rect.top) / 2;

        // Set the mouse position to the center
        SetCursorPos(centerX, centerY);

        // Get Cursor position in screen coordinates
        POINT p;
        GetCursorPos(&p);

        // Convert screen coordinates to client coordinates
        ScreenToClient(hWnd, &p);

        // Calculate the difference
        int diffX = x - p.x;
        int diffY = y - p.y;

        // Rotate the camera
        camera.ProcessMouseMovement(-diffX, diffY);
    }

    auto result = ASBR_HWNDProcCallback_Original(hWnd, msg, wParam, lParam);
    ShowCursor(FALSE); // Make the cursor invisible

    return result;
}

void copy_mat4x4(mat4x4* dest, glm::mat4* src) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            dest->m[i][j] = (*src)[i][j];
        }
    }
}

// 7727E0
typedef mat4x4*(__fastcall* ASBR_PrepareLookAtMatrix)(__int64 a1);
ASBR_PrepareLookAtMatrix ASBR_PrepareLookAtMatrix_Original;

mat4x4* __fastcall ASBR_PrepareLookAtMatrix_Hook(__int64 a1) {
    vec3f* pos = (vec3f*)(a1 + 112);
    mat4x4* matrix = (mat4x4*)(a1 + 0x30);

    // If free cam isn't currently active...
    if (!free_cam_enabled) return ASBR_PrepareLookAtMatrix_Original(a1);

    // __int64 __fastcall ASBR_CameraDirector_GetCamera(__int64 a1, unsigned int a2)
    //__int64 __fastcall sub_76B4B0(__int64 a1, unsigned int a2)
    auto GetCameraDirector = ((__int64(__fastcall*)(__int64, unsigned int)) (JAPI_GetModuleBase() + 0x76B4B0));
    auto CameraDirector_GetCamera = ((__int64(__fastcall*)(__int64, unsigned int)) (JAPI_GetModuleBase() + 0x771150));

    auto qword_12A8A40 = *(__int64*)(JAPI_GetModuleBase() + 0x12A8A40);
    __int64 camera_director = 0;

    if(qword_12A8A40) {
        camera_director = GetCameraDirector(qword_12A8A40, 0);
    }

    // Ignore all cameras except the game cameras
    if(a1 != CameraDirector_GetCamera(camera_director, 0) && a1 != CameraDirector_GetCamera(camera_director, 1) && a1 != CameraDirector_GetCamera(camera_director, 2)) {
        return ASBR_PrepareLookAtMatrix_Original(a1);
    }

    camera.ChangeX(KeyPressed.right - KeyPressed.left);
    camera.ChangeY(KeyPressed.up - KeyPressed.down);
    camera.ChangeZ(KeyPressed.forward - KeyPressed.backward);

    ASBR_PrepareLookAtMatrix_Original(a1);

    glm::mat4 view = camera.GetViewMatrix();

    void* matrix_ptr = (void*)matrix;
    void* view_ptr = (void*)&view[0][0];

    auto CopyMatrix4x4Inversed = ((__int64(__fastcall*)(void*, void*)) (JAPI_GetModuleBase() + 0x6C9300));
    CopyMatrix4x4Inversed(matrix_ptr, view_ptr);

    return matrix;
}

void __stdcall DrawImGUI() {
    if (free_cam_enabled) {
        ImGui::Text("Free Cam position: (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
    }

    ImGui::Checkbox("Enabled", &free_cam_enabled);

    if(ImGui::CollapsingHeader("Camera Properties")) {
        if(ImGui::SliderFloat("Movement Speed", &SPEED, 1.0f, 100.0f)) JAPI_ConfigSetFloat("MovementSpeed", SPEED);
        if(ImGui::SliderFloat("Mouse Sensitivity", &SENSITIVITY, 0.1f, 2.0f)) JAPI_ConfigSetFloat("MouseSensitivity", SENSITIVITY);
    }

    if(ImGui::CollapsingHeader("Controls")) {
        if(ImGui::InputText(LEFT_KEY_DESC, LEFT_KEY, 2)) {
            JAPI_ConfigSetString(LEFT_KEY_DESC, LEFT_KEY);
        }
        if(ImGui::InputText(RIGHT_KEY_DESC, RIGHT_KEY, 2)) JAPI_ConfigSetString(RIGHT_KEY_DESC, RIGHT_KEY);
        if(ImGui::InputText(FORWARD_KEY_DESC, FORWARD_KEY, 2)) JAPI_ConfigSetString(FORWARD_KEY_DESC, FORWARD_KEY);
        if(ImGui::InputText(BACKWARD_KEY_DESC, BACKWARD_KEY, 2)) JAPI_ConfigSetString(BACKWARD_KEY_DESC, BACKWARD_KEY);
        if(ImGui::InputText(UP_KEY_DESC, UP_KEY, 2)) JAPI_ConfigSetString(UP_KEY_DESC, UP_KEY);
        if(ImGui::InputText(DOWN_KEY_DESC, DOWN_KEY, 2)) JAPI_ConfigSetString(DOWN_KEY_DESC, DOWN_KEY);
        if(ImGui::InputText(BLOCK_CAMERA_KEY_DESC, BLOCK_CAMERA_KEY, 2)) JAPI_ConfigSetString(BLOCK_CAMERA_KEY_DESC, BLOCK_CAMERA_KEY);
    }
}

// Not required but useful
void __stdcall ModInit() {

    // Bind all the values
    LEFT_KEY = JAPI_ConfigBindString("KeyLeft", "J");
    RIGHT_KEY = JAPI_ConfigBindString("KeyRight", "L");
    FORWARD_KEY = JAPI_ConfigBindString("KeyForward", "I");
    BACKWARD_KEY = JAPI_ConfigBindString("KeyBackward", "K");
    UP_KEY = JAPI_ConfigBindString("KeyUp", "U");
    DOWN_KEY = JAPI_ConfigBindString("KeyDown", "O");
    BLOCK_CAMERA_KEY = JAPI_ConfigBindString("BlockCameraKey", "B");

    SPEED = JAPI_ConfigBindFloat("MovementSpeed", 0.05f);
    SENSITIVITY = JAPI_ConfigBindFloat("MouseSensitivity", 0.1f);

    JAPIHook h_ASBR_PrepareLookAtMatrix = {
        0x7727E0,
        (void*)ASBR_PrepareLookAtMatrix_Hook,
        (void**)&ASBR_PrepareLookAtMatrix_Original,
        "ASBR_PrepareLookAtMatrix"
    };

    JAPIHook h_ASBR_HWNDProcCallback = {
        0x66185C,
        (void*)ASBR_HWNDProcCallback_Hook,
        (void**)&ASBR_HWNDProcCallback_Original,
        "ASBR_HWNDProcCallback"
    };

    if(!JAPI_HookGameFunction(h_ASBR_PrepareLookAtMatrix) || !JAPI_HookGameFunction(h_ASBR_HWNDProcCallback)) {
        JERROR("Hooking failed!");
        return;
    }

    JINFO("Initialized!");
}