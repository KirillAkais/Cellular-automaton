#include "SFML/Graphics.hpp"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include "Automaton.h"
#include "misc/cpp/imgui_stdlib.h"

#include <iostream>

using namespace std;

//Automata window size
#define WINDOW_W 800
#define WINDOW_H 800

#define BIG_AUTOMATON_W 256.
#define BIG_AUTOMATON_H 256.

#define AMOUNT_PER_SIDE 4
#define AMOUNT AMOUNT_PER_SIDE*AMOUNT_PER_SIDE
//Control panel size
#define CONTROL_W 800
#define CONTROL_H 800

// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace sf;

int main()
{
	srand(time(0));

    string name = "unnamed";
    bool overwrite = false;
    int view_id = 0;
    int current_id = 0;
    int big_id = 0;
    static int f = 30;
    static bool paused = false;
    static bool show_id = true;
    static bool draw_big = false;
    vector<int> patterns;

	RenderWindow window(VideoMode(WINDOW_W, WINDOW_H), "Cellular automata"); //Automata window

	//Automaton declaration and creation of sprite for drawing it
    Population p;
    Automaton big(BIG_AUTOMATON_W, BIG_AUTOMATON_H);

	Image **image = new Image*[AMOUNT + 1];
	Texture **texture = new Texture*[AMOUNT + 1];
	
	Sprite* sprite = new Sprite[AMOUNT + 1];
    for (int i = 0; i <= AMOUNT; i++)
    {
        image[i] = new Image;
        if (i != AMOUNT)
            image[i]->create(DEFAULT_FIELD_W, DEFAULT_FIELD_H);
        else
            image[i]->create(BIG_AUTOMATON_W, BIG_AUTOMATON_H);
        texture[i] = new Texture;
        texture[i]->loadFromImage(*image[i]);
        sprite[i].setTexture(*texture[i]);
        if (i != AMOUNT)
            sprite[i].setScale(WINDOW_W / DEFAULT_FIELD_W / AMOUNT_PER_SIDE, WINDOW_H / DEFAULT_FIELD_H / AMOUNT_PER_SIDE);
        else
            sprite[i].setScale(WINDOW_W / BIG_AUTOMATON_W, WINDOW_H / BIG_AUTOMATON_H);
    }
    Font font;
    font.loadFromFile("c:\\Windows\\Fonts\\verdana.ttf");
    Text id_display;
    id_display.setFont(font);
    id_display.setFillColor(Color::White);
    id_display.setOutlineColor(Color::Black);
    id_display.setOutlineThickness(3);

    // Create control panel window
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Cellular automata", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Automata control", WS_OVERLAPPEDWINDOW, 100, 100, CONTROL_W, CONTROL_H, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the control window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Font
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\verdana.ttf", 24.0f);

    ImVec4 clear_color = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);

    //Main loop
	while (window.isOpen())
	{

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_CLOSE)
                window.close();
        }

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        
        // Create control menu
        {
            static int mutation_chance = 10;
            static int mutation_amount = 2;
            static int gene_ratio = 50;
            static int field_ratio = 50;
            static int steps = 500;
            static int evo_steps = 500;
            static int pattern = 33080895;
            static int mistakes = 2;


            const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(CONTROL_W, CONTROL_H), ImGuiCond_Always);


            ImGui::Begin("Control panel", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::SeparatorText("Render settings");

            ImGui::SliderInt("FPS", &f, 1, 60);

            ImGui::Text("Showing automata %d-%d/%d", view_id, view_id + AMOUNT - 1, POPULATION_SIZE - 1);
            ImGui::SameLine();
            if (ImGui::Button("< Prev"))
                view_id -= view_id < AMOUNT ? view_id : AMOUNT;
            ImGui::SameLine();
            if (ImGui::Button("Next >"))
                view_id += view_id < POPULATION_SIZE - AMOUNT ? AMOUNT : 0;

            ImGui::Checkbox("Show IDs", &show_id);
            ImGui::Checkbox("Show big automaton", &draw_big);
            ImGui::InputInt("Copy from", &big_id);
            big_id = big_id < 0 ? 0 : big_id < POPULATION_SIZE ? big_id : POPULATION_SIZE - 1;
            ImGui::SameLine();
            if (ImGui::Button("Copy"))
                big.clone(p.automata[big_id]);
            ImGui::Checkbox("Paused", &paused);

            if (ImGui::Button("Single step"))
                p.step();

            ImGui::InputInt("steps at once", &steps, 1, 100);
            ImGui::SameLine();
            if (ImGui::Button("Go"))
                p.step(steps);


            ImGui::SeparatorText("Rule settings");
            if (ImGui::Button("Set to Game of life"))
                p.set_conway();

            ImGui::SliderInt("% of 1 in rule", &gene_ratio, 0, 100);
            ImGui::SameLine();
            if (ImGui::Button("Set"))
                p.set_gene_ratio(gene_ratio);

            ImGui::SeparatorText("Reset field");

            if (ImGui::Button("Fill random"))
            {
                p.fill_ratio(50);
                big.fill_ratio(50);
            }

            ImGui::SliderInt("% of 1 on field", &field_ratio, 0, 100);
            ImGui::SameLine();
            if (ImGui::Button("Fill"))
            {
                p.fill_ratio(field_ratio);
                big.fill_ratio(field_ratio);
            }

            if (ImGui::Button("Set 0ne"))
            {
                p.fill_one();
                big.fill_one();
            }

            ImGui::SeparatorText("Evolution settings");

            const char* items[] = { "Least change", "Pattern", "Pattern fast", "Pattern static", "By hand" };
            static int type = 0;
            ImGui::Combo("Selection type", &type, items, IM_ARRAYSIZE(items));

            ImGui::InputInt("Pattern", &pattern);
            ImGui::SameLine();
            if (ImGui::Button("Add"))
                patterns.push_back(pattern);
            ImGui::Text("Patterns: ");
            for (int i = 0; i < patterns.size(); i++)
            {
                //ImGui::SameLine();
                ImGui::Text("%d", patterns[i]);
            }
            if (ImGui::Button("Remove") && patterns.size())
                patterns.pop_back();

            ImGui::InputInt("Allowed mistakes", &mistakes);

            ImGui::SliderInt("% of mutations", &mutation_chance, 0, 100);
            ImGui::SliderInt("amount of mutations", &mutation_amount, 0, 32);

            ImGui::InputInt("Evolution steps", &evo_steps, 1, 1000);
            ImGui::SameLine();
            if (ImGui::Button("Evolve"))
                p.evolute(evo_steps, type, mutation_chance, mutation_amount, &patterns, &mistakes);

            ImGui::Text("Evolution process normally takes several minutes");

            ImGui::SeparatorText("Save/load automaton");
            ImGui::InputInt("Automaton ID", &current_id);
            current_id = current_id < 0 ? 0 : current_id < POPULATION_SIZE ? current_id : POPULATION_SIZE - 1;
            ImGui::InputText("Enter name", &name);

            if (ImGui::Button("Save"))
                p.automata[current_id]->write(name, overwrite);
            ImGui::SameLine();
            if (ImGui::Button("Load"))
                p.automata[current_id]->read(name);
            ImGui::SameLine();
            ImGui::Checkbox("Overwrite", &overwrite);

            ImGui::Separator();

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // Rendering gui
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();

        // Handle SFML window event
		Event e;
		while (window.pollEvent(e))
		{
			switch (e.type)
			{
			case Event::Closed:
				window.close();
                break;
			default:
				break;
			}
		}
        // Iterate and draw automaton
        if (draw_big)
        {
            if (!paused)
                big.draw(image[AMOUNT], f);
            else
                big.draw(image[AMOUNT]);
            texture[AMOUNT]->loadFromImage(*image[AMOUNT]);
            sprite[AMOUNT].setTexture(*texture[AMOUNT]);
            sprite[AMOUNT].setScale(WINDOW_W / BIG_AUTOMATON_W, WINDOW_H / BIG_AUTOMATON_H);
        }
        else
        {
            if (!paused)
                p.draw(view_id, view_id + AMOUNT, image, f);
            else
                p.draw(view_id, view_id + AMOUNT, image);
            for (int i = 0; i < AMOUNT; i++)
            {
                texture[i]->loadFromImage(*image[i]);
                sprite[i].setTexture(*texture[i]);
                sprite[i].setScale(WINDOW_W / DEFAULT_FIELD_W / AMOUNT_PER_SIDE, WINDOW_H / DEFAULT_FIELD_H / AMOUNT_PER_SIDE);
            }
        }
		window.clear();
        if (draw_big)
            window.draw(sprite[AMOUNT]);
        else
        {
            for (int i = 0; i < AMOUNT; i++)
            {
                sprite[i].setPosition(5 + (sprite[i].getScale().x * DEFAULT_FIELD_W + 5) * (i % AMOUNT_PER_SIDE),
                    5 + (sprite[i].getScale().x * DEFAULT_FIELD_H + 5) * (i / AMOUNT_PER_SIDE));
                id_display.setPosition(sprite[i].getPosition());
                id_display.setString(to_string(view_id + i));
                window.draw(sprite[i]);
                if (show_id)
                    window.draw(id_display);
            }
        }
		window.display();
	}

    // Close gui window
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}