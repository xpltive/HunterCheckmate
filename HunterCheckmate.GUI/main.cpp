// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_stdlib.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include "FileHandler.h"
#include "AnimalPopulation.h"
#include "ThpPlayerProfile.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void ShowFileSelector(bool* p_open);
static void ShowMainWindow();
static void ShowGroupInfo();


using namespace HunterCheckmate_FileAnalyzer;

bool keep_open = true;
std::string str_input_file_path = "C:/Users/oleSQL/Documents/thehunter working/pop/decomp_animal_population_0";
boost::filesystem::path input_file_path = "C:/Users/oleSQL/Documents/thehunter working/pop/decomp_animal_population_0";
std::shared_ptr<FileHandler> file_handler = std::make_shared<FileHandler>(Endian::Little, input_file_path);
std::shared_ptr<ReserveData> reserve_data = std::make_shared<ReserveData>(ReserveData::ResolveFileNameToInt(input_file_path.filename().generic_string()));
std::unique_ptr<AnimalPopulation> animal_population = std::make_unique<AnimalPopulation>(file_handler, reserve_data);

std::string animal_str = "";
// idx, weight, score
std::vector<bool> sort = { true, false, false, false, false, false };

AnimalType edit_animal_type;
uint32_t edit_animal_grp_idx;
int edit_animal_idx;
std::string edit_animal_gender;
float edit_animal_weight;
float edit_animal_score;
std::string edit_animal_str_igo;
std::string edit_animal_str_fur_type;
bool expand_groups = false;
bool collapse_groups = false;

bool show_file_selector = false;
bool show_all_animals = false;
std::shared_ptr<AnimalGroup> selected_group;
AnimalType animal_type;
std::vector<AnimalGroup> groups;



// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Hunter Checkmate"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Hunter Checkmate"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;
    //io.ConfigViewportsNoDefaultParent = true;
    //io.ConfigDockingAlwaysTabBar = true;
    //io.ConfigDockingTransparentPayload = true;
    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
	style.FrameRounding = 5.0f;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

	io.Fonts->AddFontFromFileTTF("fonts/CascadiaMono.ttf", 15.0f);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

		if (show_file_selector) ShowFileSelector(&show_file_selector);

		ShowMainWindow();

        //ImGui::ShowDemoWindow();


        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

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
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

void ShowFileSelector(bool* p_open)
{
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImVec2 size = ImVec2(vp->WorkSize.x / 2.f, vp->WorkSize.y / 2.f);
	ImVec2 pos = ImVec2(vp->GetCenter().x - (size.x / 2.f), vp->GetCenter().y - (size.y / 2.f));
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus;

	if (ImGui::Begin("File Selector", p_open, window_flags))
	{
		ImGui::BeginChild("file_path", ImVec2(ImGui::GetContentRegionAvail().x, 80.f), true);
		ImGui::Text("Enter the location of your decompiled population file:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::InputText(" ", &str_input_file_path);
		input_file_path = str_input_file_path;
		ImGui::EndChild();

		ImGui::BeginChild("file_browser", ImVec2(ImGui::GetContentRegionAvail().x, 200.f), true);
		if (ImGui::Button(".."))
		{
			str_input_file_path = input_file_path.parent_path().generic_string();
			input_file_path = input_file_path.parent_path();
		}
		if (boost::filesystem::exists(input_file_path))
		{
			if (boost::filesystem::is_directory(input_file_path))
			{
				for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(input_file_path))
				{
					if (boost::filesystem::is_directory(x.path()))
					{
						if (ImGui::Button(x.path().filename().generic_string().c_str()))
						{
							str_input_file_path = x.path().generic_string();
							input_file_path = x.path();

						}
					}
					else
					{
						if (ImGui::Selectable(x.path().filename().generic_string().c_str()))
						{
							str_input_file_path = x.path().generic_string();
							input_file_path = x.path();
						}
					}
				}
			}
			else
			{
				for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(input_file_path.parent_path()))
				{
					if (boost::filesystem::is_directory(x.path()))
					{
						if (ImGui::Button(x.path().filename().generic_string().c_str()))
						{
							str_input_file_path = x.path().generic_string();
							input_file_path = x.path();

						}
					}
					else
					{
						if (ImGui::Selectable(x.path().filename().generic_string().c_str()))
						{
							str_input_file_path = x.path().generic_string();
							input_file_path = x.path();
						}
					}
				}
			}
		}
		ImGui::EndChild();

		ImGui::BeginChild("file_actions", ImGui::GetContentRegionAvail(), true);
		if (ImGui::Button("Analyze"))
		{
			animal_str = "";
			file_handler = std::make_shared<FileHandler>(Endian::Little, input_file_path);
			reserve_data = std::make_shared<ReserveData>(ReserveData::ResolveFileNameToInt(input_file_path.filename().generic_string()));
			animal_population = std::make_unique<AnimalPopulation>(file_handler, reserve_data);
			if (animal_population->Deserialize())
			{
				animal_population->MapAnimals();
				std::cout << "[DEBUG] Successfully analyzed population file!\n";
				show_file_selector = false;
			}
			else
			{
				ImGui::OpenPopup("file_failed_loading");

			}
		}
		size = ImVec2(180, 60);
		pos = ImVec2(vp->GetCenter().x - (size.x / 2.f), vp->GetCenter().y - (size.y / 2.f));
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowViewport(vp->ID);
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		if (ImGui::BeginPopupModal("file_failed_loading", (bool*)0, window_flags))
		{
			ImGui::Text("Invalid file selected!");
			if (ImGui::Button("Ok"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::EndChild();

		ImGui::End();
	}
}

void ShowMainWindow()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoFocusOnAppearing;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImVec2 window_pos = viewport->WorkPos;
	ImVec2 window_size = viewport->WorkSize;

	ImGui::SetNextWindowPos(window_pos);
	ImGui::SetNextWindowSize(window_size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::Begin("HunterCheckmate", NULL, window_flags);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("Load", NULL, &show_file_selector);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Settings"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ImGui::BeginChild("sort", ImVec2(ImGui::GetContentRegionAvail().x, 65.f), true);
	if (ImGui::BeginTabBar("animal_selection", ImGuiTabBarFlags_FittingPolicyResizeDown))
	{
		if (animal_population->m_initialized && animal_population->m_valid)
		{
			auto it_group = animal_population->m_animals.begin();
			for (; it_group != animal_population->m_animals.end(); ++it_group)
			{
				std::string show_animal = it_group->second.at(0).m_name;
				if (ImGui::BeginTabItem(show_animal.c_str()))
				{
					animal_str = show_animal;
					ImGui::EndTabItem();
				}
			}
		}
		ImGui::EndTabBar();
	}

	ImGui::Text("Sort by ");
	ImGui::SameLine();
	if (ImGui::Button("Index"))
	{
		sort = { true, false, false, false, false, false };
	}
	ImGui::SameLine();
	if (ImGui::Button("Score Down"))
	{
		sort = { false, true, false , false, false, false };
	}
	ImGui::SameLine();
	if (ImGui::Button("Score Up"))
	{
		sort = { false, false, true, false, false, false };
	}
	ImGui::SameLine();
	if (ImGui::Button("Weight Down"))
	{
		sort = { false, false, false, true, false, false };
	}
	ImGui::SameLine();
	if (ImGui::Button("Weight Up"))
	{
		sort = { false, false, false, false, true, false };
	}
	ImGui::SameLine();
	if (ImGui::Button("Fur Type"))
	{
		sort = { false, false, false, false, false, true };
	}

	ImGui::EndChild();

	ImGui::BeginChild("species_info", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 90.f), true);
	if (animal_population->m_initialized && animal_population->m_valid && animal_str != "")
	{
		std::string info_text = (boost::format("General info about %s:") % animal_str).str();
		std::string info_text_male;
		std::string info_text_female;
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(info_text.c_str()).x) * 0.5f);
		ImGui::Text(info_text.c_str());
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("MALE").x) * 0.333333f);
		ImGui::Text("MALE");
		ImGui::SameLine();
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("MALE").x - ImGui::CalcTextSize("FEMALE").x) * 0.666666f);
		ImGui::Text("FEMALE");
		ImGui::Text("Weight:");
		ImGui::SameLine();

		info_text_male = (boost::format("[%.2f - %.2f]") % weight_db.at(Animal::ResolveAnimalType(animal_str)).min_weight_male % weight_db.at(Animal::ResolveAnimalType(animal_str)).max_weight_male).str();
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(info_text_male.c_str()).x) * 0.333333f);
		ImGui::Text(info_text_male.c_str());
		ImGui::SameLine();

		info_text_female = (boost::format("[%.2f - %.2f]") % weight_db.at(Animal::ResolveAnimalType(animal_str)).min_weight_female % weight_db.at(Animal::ResolveAnimalType(animal_str)).max_weight_female).str();
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(info_text_female.c_str()).x) * 0.666666f);
		ImGui::Text(info_text_female.c_str());


		ImGui::Text("Score:");
		ImGui::SameLine();
		info_text_male = (boost::format("[%.2f - %.2f]") % score_db.at(Animal::ResolveAnimalType(animal_str)).min_score_male % score_db.at(Animal::ResolveAnimalType(animal_str)).max_score_male).str();
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(info_text_male.c_str()).x) * 0.333333f);
		ImGui::Text(info_text_male.c_str());
		ImGui::SameLine();

		info_text_female = (boost::format("[%.2f - %.2f]") % score_db.at(Animal::ResolveAnimalType(animal_str)).min_score_female % score_db.at(Animal::ResolveAnimalType(animal_str)).max_score_female).str();
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(info_text_female.c_str()).x) * 0.666666f);
		ImGui::Text(info_text_female.c_str());
	}

	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("species_stats", ImVec2(ImGui::GetContentRegionAvail().x, 90.f), true);
	if (animal_population->m_initialized && animal_population->m_valid && animal_str != "")
	{
		std::string info_text = (boost::format("Statistics about your %s population:") % animal_str).str();
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(info_text.c_str()).x) * 0.5f);
		ImGui::Text(info_text.c_str());

		animal_type = Animal::ResolveAnimalType(animal_str);
		groups = animal_population->m_animals.at(animal_type);

		uint32_t total_amount = 0;
		uint32_t male_amount = 0;
		uint32_t female_amount = 0;
		auto it_groups = groups.begin();
		for (; it_groups != groups.end(); ++it_groups)
		{
			total_amount += it_groups->m_size;
			auto it_animals = it_groups->m_animals.begin();
			for (; it_animals != it_groups->m_animals.end(); ++it_animals)
			{
				if (it_animals->m_gender == "male")
					male_amount++;
				else if (it_animals->m_gender == "female")
					female_amount++;
			}
		}

		info_text = (boost::format("Total amount: %i [%i male | %i female]") % total_amount % male_amount % female_amount).str();
		ImGui::Text(info_text.c_str());
	}
	ImGui::EndChild();

	ImGui::BeginChild("group_overview", ImVec2(300.f, ImGui::GetContentRegionAvail().y), true);
	if (animal_population->m_initialized && animal_population->m_valid && animal_str != "")
	{
		animal_type = Animal::ResolveAnimalType(animal_str);
		groups = animal_population->m_animals.at(animal_type);

		if (sort.at(0))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpIdx);
		else if (sort.at(1))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpHighestScore);
		else if (sort.at(2))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpLowestScore);
		else if (sort.at(3))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpHighestWeight);
		else if (sort.at(4))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpLowestWeight);

		if (ImGui::Selectable("All Groups"))
		{
			show_all_animals = true;
		}

		auto it_groups = groups.begin();
		for (; it_groups != groups.end(); ++it_groups)
		{
			std::string desc = "Group [#" + std::to_string(it_groups->m_size) + "]";
			ImGui::PushID(it_groups->m_index);
			if (ImGui::Selectable(desc.c_str()))
			{
				selected_group = std::make_shared<AnimalGroup>(*it_groups);
				show_all_animals = false;
			}
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::Text((boost::format("%.1f") % it_groups->m_max_weight).str().c_str());
			ImGui::SameLine();
			ImGui::Text((boost::format("%.1f") % it_groups->m_max_score).str().c_str());
			ImGui::SameLine();
			ImGui::Text(std::to_string(it_groups->m_spawn_area_id).c_str());
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("group_animal_overview", ImGui::GetContentRegionAvail(), true);
	if (animal_population->m_initialized && animal_population->m_valid && animal_str != "" && (!(selected_group == nullptr) || show_all_animals))
	{
		if (ImGui::BeginTable("animal_table", 10, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable |
							  ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter))
		{
			ImGui::TableSetupColumn("Grp Idx");
			ImGui::TableSetupColumn("Idx");
			ImGui::TableSetupColumn("Gender");
			ImGui::TableSetupColumn("Weight");
			ImGui::TableSetupColumn("Score");
			ImGui::TableSetupColumn("Is GO");
			ImGui::TableSetupColumn("Fur Type");
			ImGui::TableSetupColumn("Fur Type Id");
			ImGui::TableSetupColumn("Visual Variation Seed");
			ImGui::TableSetupColumn("Operations");
			ImGui::TableHeadersRow();
			ImGui::TableNextRow();

			if (!show_all_animals)
			{
				if (selected_group->m_animal_type != Animal::ResolveAnimalType(animal_str))
					show_all_animals = true;

				if (sort.at(0))
					std::sort(selected_group->m_animals.begin(), selected_group->m_animals.end(), AnimalGroup::cmpIdx);
				else if (sort.at(1))
					std::sort(selected_group->m_animals.begin(), selected_group->m_animals.end(), AnimalGroup::cmpHighestScore);
				else if (sort.at(2))
					std::sort(selected_group->m_animals.begin(), selected_group->m_animals.end(), AnimalGroup::cmpLowestScore);
				else if (sort.at(3))
					std::sort(selected_group->m_animals.begin(), selected_group->m_animals.end(), AnimalGroup::cmpHighestWeight);
				else if (sort.at(4))
					std::sort(selected_group->m_animals.begin(), selected_group->m_animals.end(), AnimalGroup::cmpLowestWeight);
				else if (sort.at(5))
					std::sort(selected_group->m_animals.begin(), selected_group->m_animals.end(), AnimalGroup::cmpFurTypeA);

				auto it_animals = selected_group->m_animals.begin();
				for (; it_animals != selected_group->m_animals.end(); ++it_animals)
				{
					int idx = std::distance(selected_group->m_animals.begin(), it_animals);
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(std::to_string(selected_group->m_index).c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(std::to_string(it_animals->m_idx).c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::Text(it_animals->m_gender.c_str());
					ImGui::TableSetColumnIndex(3);
					ImGui::Text(std::to_string(it_animals->m_weight).c_str());
					ImGui::TableSetColumnIndex(4);
					ImGui::Text(std::to_string(it_animals->m_score).c_str());
					ImGui::TableSetColumnIndex(5);
					ImGui::Text(std::to_string(it_animals->m_is_great_one).c_str());
					ImGui::TableSetColumnIndex(6);
					ImGui::Text(it_animals->m_fur_type.c_str());
					ImGui::TableSetColumnIndex(7);
					ImGui::Text(std::to_string(it_animals->m_fur_type_id).c_str());
					ImGui::TableSetColumnIndex(8);
					ImGui::Text(std::to_string(it_animals->m_visual_variation_seed).c_str());
					ImGui::TableSetColumnIndex(9);
					ImGui::PushID(&(it_animals->m_visual_variation_seed));
					if (ImGui::Button("Edit"))
					{
						edit_animal_type = it_animals->m_animal_type;
						edit_animal_idx = static_cast<int>(it_animals->m_idx);
						edit_animal_gender = it_animals->m_gender;
						edit_animal_weight = it_animals->m_weight;
						edit_animal_score = it_animals->m_score;
						edit_animal_str_igo = std::to_string(it_animals->m_is_great_one);
						edit_animal_str_fur_type = it_animals->m_fur_type;
						ImGui::OpenPopup("Edit Animal");
					}

					ImVec2 center = ImGui::GetMainViewport()->GetCenter();
					ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
					ImVec2 size = ImGui::GetMainViewport()->Size;
					ImGui::SetNextWindowSize(ImVec2(size.x / 2.f, size.y / 2.f));
					if (ImGui::BeginPopupModal("Edit Animal", (bool*)0))
					{
						const char* items[] = { "male", "female" };
						static int gender_combo_current_index = 0;
						if (edit_animal_gender == "male")
							gender_combo_current_index = 0;
						else
							gender_combo_current_index = 1;
						const char* gender_combo_preview = edit_animal_gender.c_str();  // Pass in the preview value visible before opening the combo (it could be anything)
						if (ImGui::BeginCombo("Gender", gender_combo_preview))
						{
							for (int n = 0; n < 2; n++)
							{
								const bool is_gender_selected = (gender_combo_current_index == n);
								if (ImGui::Selectable(items[n], is_gender_selected))
									gender_combo_current_index = n;

								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_gender_selected)
									ImGui::SetItemDefaultFocus();
							}
							edit_animal_gender = items[gender_combo_current_index];
							ImGui::EndCombo();
						}

						ImGui::InputFloat("Weight", &edit_animal_weight);
						ImGui::InputFloat("Score", &edit_animal_score);
						ImGui::InputText("IGO", &edit_animal_str_igo);
						std::vector<const char*> fur_items;
						FurEntry* fur_entry = nullptr;
						if (edit_animal_gender == "male")
							fur_entry = &fur_db.at(edit_animal_type).at(0);
						else if (edit_animal_gender == "female")
							fur_entry = &fur_db.at(edit_animal_type).at(1);

						if (fur_entry != nullptr)
						{
							auto it_fur_type = fur_entry->fur_types.begin();
							for (; it_fur_type != fur_entry->fur_types.end(); ++it_fur_type)
							{
								fur_items.push_back(it_fur_type->fur_name.c_str());
							}
							static int fur_combo_current_index = 0;
							it_fur_type = fur_entry->fur_types.begin();
							for (; it_fur_type != fur_entry->fur_types.end(); ++it_fur_type)
							{
								size_t idx = std::distance(std::begin(fur_entry->fur_types), it_fur_type);
								if (it_fur_type->fur_name == edit_animal_str_fur_type)
									fur_combo_current_index = idx;
							}
							if (ImGui::BeginCombo("Fur Type", edit_animal_str_fur_type.c_str()))
							{
								auto it_fur_type = fur_entry->fur_types.begin();
								for (; it_fur_type != fur_entry->fur_types.end(); ++it_fur_type)
								{
									size_t idx = std::distance(std::begin(fur_entry->fur_types), it_fur_type);
									const bool is_fur_selected = (fur_combo_current_index == idx);
									if (ImGui::Selectable(fur_items.at(idx), is_fur_selected))
										fur_combo_current_index = idx;
								}
								edit_animal_str_fur_type = fur_items[fur_combo_current_index];
								ImGui::EndCombo();
							}
						}
					

						//ImGui::InputText("Fur Type", &edit_animal_str_fur_type);
						if (ImGui::Button("Save"))
						{
							uint32_t visual_variation_seed = Animal::CreateVisualVariationSeed(animal_type, edit_animal_gender, edit_animal_str_fur_type);
							const std::shared_ptr<Animal> animal =
								Animal::Create(animal_type, Animal::ResolveGender(edit_animal_gender),
											   edit_animal_weight, edit_animal_score, boost::lexical_cast<bool>(edit_animal_str_igo),
											   visual_variation_seed, edit_animal_idx, selected_group->m_index);
							if (animal->IsValid())
							{
								animal_population->ReplaceAnimal(animal, selected_group->m_index, it_animals->m_idx);
							}
							animal_population = std::make_unique<AnimalPopulation>(file_handler, reserve_data);
							if (animal_population->Deserialize())
							{
								animal_population->MapAnimals();
								AnimalType tmp_at = selected_group->m_animal_type;
								uint32_t tmp_idx = selected_group->m_index;
								selected_group = std::make_shared<AnimalGroup>(animal_population->m_animals.at(tmp_at).at(tmp_idx));
								it_animals = selected_group->m_animals.begin() + idx;
							}
							ImGui::CloseCurrentPopup();
						}
						ImGui::SameLine();
						if (ImGui::Button("Exit"))
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					ImGui::PopID();



					if (idx < selected_group->m_size - 1)
						ImGui::TableNextRow();
				}
			}
			else
			{
				AnimalGroup tmp_group;
				std::vector<Animal> animals;
				auto it_groups = groups.begin();
				for (; it_groups != groups.end(); ++it_groups)
					animals.insert(animals.end(), it_groups->m_animals.begin(), it_groups->m_animals.end());

				tmp_group.m_animals = animals;
				if (sort.at(0))
					std::sort(tmp_group.m_animals.begin(), tmp_group.m_animals.end(), AnimalGroup::cmpIdx);
				else if (sort.at(1))
					std::sort(tmp_group.m_animals.begin(), tmp_group.m_animals.end(), AnimalGroup::cmpHighestScore);
				else if (sort.at(2))
					std::sort(tmp_group.m_animals.begin(), tmp_group.m_animals.end(), AnimalGroup::cmpLowestScore);
				else if (sort.at(3))
					std::sort(tmp_group.m_animals.begin(), tmp_group.m_animals.end(), AnimalGroup::cmpHighestWeight);
				else if (sort.at(4))
					std::sort(tmp_group.m_animals.begin(), tmp_group.m_animals.end(), AnimalGroup::cmpLowestWeight);
				else if (sort.at(5))
					std::sort(tmp_group.m_animals.begin(), tmp_group.m_animals.end(), AnimalGroup::cmpFurTypeA);

				auto it_animals = tmp_group.m_animals.begin();
				for (; it_animals != tmp_group.m_animals.end(); ++it_animals)
				{
					int idx = std::distance(tmp_group.m_animals.begin(), it_animals);
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(std::to_string(it_animals->m_grp_idx).c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(std::to_string(it_animals->m_idx).c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::Text(it_animals->m_gender.c_str());
					ImGui::TableSetColumnIndex(3);
					ImGui::Text(std::to_string(it_animals->m_weight).c_str());
					ImGui::TableSetColumnIndex(4);
					ImGui::Text(std::to_string(it_animals->m_score).c_str());
					ImGui::TableSetColumnIndex(5);
					ImGui::Text(std::to_string(it_animals->m_is_great_one).c_str());
					ImGui::TableSetColumnIndex(6);
					ImGui::Text(it_animals->m_fur_type.c_str());
					ImGui::TableSetColumnIndex(7);
					ImGui::Text(std::to_string(it_animals->m_fur_type_id).c_str());
					ImGui::TableSetColumnIndex(8);
					ImGui::Text(std::to_string(it_animals->m_visual_variation_seed).c_str());
					ImGui::TableSetColumnIndex(9);
					ImGui::PushID(&(it_animals->m_visual_variation_seed));
					if (ImGui::Button("Edit"))
					{
						edit_animal_grp_idx = it_animals->m_grp_idx;
						edit_animal_type = it_animals->m_animal_type;
						edit_animal_idx = static_cast<int>(it_animals->m_idx);
						edit_animal_gender = it_animals->m_gender;
						edit_animal_weight = it_animals->m_weight;
						edit_animal_score = it_animals->m_score;
						edit_animal_str_igo = std::to_string(it_animals->m_is_great_one);
						edit_animal_str_fur_type = it_animals->m_fur_type;
						ImGui::OpenPopup("Edit Animal");
					}

					ImVec2 center = ImGui::GetMainViewport()->GetCenter();
					ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
					ImVec2 size = ImGui::GetMainViewport()->Size;
					ImGui::SetNextWindowSize(ImVec2(size.x / 2.f, size.y / 2.f));
					if (ImGui::BeginPopupModal("Edit Animal", (bool*)0))
					{
						const char* items[] = { "male", "female" };
						static int gender_combo_current_index = 0;
						if (edit_animal_gender == "male")
							gender_combo_current_index = 0;
						else
							gender_combo_current_index = 1;
						const char* gender_combo_preview = edit_animal_gender.c_str();  // Pass in the preview value visible before opening the combo (it could be anything)
						if (ImGui::BeginCombo("Gender", gender_combo_preview))
						{
							for (int n = 0; n < 2; n++)
							{
								const bool is_gender_selected = (gender_combo_current_index == n);
								if (ImGui::Selectable(items[n], is_gender_selected))
									gender_combo_current_index = n;

								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_gender_selected)
									ImGui::SetItemDefaultFocus();
							}
							edit_animal_gender = items[gender_combo_current_index];
							ImGui::EndCombo();
						}

						ImGui::InputFloat("Weight", &edit_animal_weight);
						ImGui::InputFloat("Score", &edit_animal_score);
						ImGui::InputText("IGO", &edit_animal_str_igo);
						std::vector<const char*> fur_items;
						FurEntry* fur_entry = nullptr;
						if (edit_animal_gender == "male")
							fur_entry = &fur_db.at(edit_animal_type).at(0);
						else if (edit_animal_gender == "female")
							fur_entry = &fur_db.at(edit_animal_type).at(1);

						if (fur_entry != nullptr)
						{
							auto it_fur_type = fur_entry->fur_types.begin();
							for (; it_fur_type != fur_entry->fur_types.end(); ++it_fur_type)
							{
								fur_items.push_back(it_fur_type->fur_name.c_str());
							}
							static int fur_combo_current_index = 0;
							it_fur_type = fur_entry->fur_types.begin();
							for (; it_fur_type != fur_entry->fur_types.end(); ++it_fur_type)
							{
								size_t idx = std::distance(std::begin(fur_entry->fur_types), it_fur_type);
								if (it_fur_type->fur_name == edit_animal_str_fur_type)
									fur_combo_current_index = idx;
							}
							if (ImGui::BeginCombo("Fur Type", edit_animal_str_fur_type.c_str()))
							{
								auto it_fur_type = fur_entry->fur_types.begin();
								for (; it_fur_type != fur_entry->fur_types.end(); ++it_fur_type)
								{
									size_t idx = std::distance(std::begin(fur_entry->fur_types), it_fur_type);
									const bool is_fur_selected = (fur_combo_current_index == idx);
									if (ImGui::Selectable(fur_items.at(idx), is_fur_selected))
										fur_combo_current_index = idx;
								}
								edit_animal_str_fur_type = fur_items[fur_combo_current_index];
								ImGui::EndCombo();
							}
						}


						//ImGui::InputText("Fur Type", &edit_animal_str_fur_type);
						if (ImGui::Button("Save"))
						{
							uint32_t visual_variation_seed = Animal::CreateVisualVariationSeed(animal_type, edit_animal_gender, edit_animal_str_fur_type);
							const std::shared_ptr<Animal> animal =
								Animal::Create(animal_type, Animal::ResolveGender(edit_animal_gender),
											   edit_animal_weight, edit_animal_score, boost::lexical_cast<bool>(edit_animal_str_igo),
											   visual_variation_seed, edit_animal_idx, edit_animal_grp_idx);
							if (animal->IsValid())
							{
								animal_population->ReplaceAnimal(animal, edit_animal_grp_idx, it_animals->m_idx);
							}
							animal_population = std::make_unique<AnimalPopulation>(file_handler, reserve_data);
							if (animal_population->Deserialize())
							{
								animal_population->MapAnimals();
							}
							ImGui::CloseCurrentPopup();
						}
						ImGui::SameLine();
						if (ImGui::Button("Exit"))
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					ImGui::PopID();

					if (idx < animals.size() - 1)
						ImGui::TableNextRow();
				}

			}

			
			
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

void ShowGroupInfo()
{
	if (animal_str != "")
	{
		const AnimalType animal_type = Animal::ResolveAnimalType(animal_str);
		std::vector<AnimalGroup> groups = animal_population->m_animals.at(animal_type);
		uint32_t total_animals = 0;

		if (sort.at(0))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpIdx);
		else if (sort.at(1))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpHighestScore);
		else if (sort.at(2))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpLowestScore);
		else if (sort.at(3))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpHighestWeight);
		else if (sort.at(4))
			std::sort(groups.begin(), groups.end(), AnimalPopulation::cmpLowestWeight);

		auto it_beg = groups.begin();
		for (; it_beg != groups.end(); ++it_beg)
		{
			std::ostringstream group_info;
			group_info << "[ " << std::setw(3) << it_beg->m_index
				<< " | #" << std::setw(2) << it_beg->m_size
				<< " | " << std::setw(12) << it_beg->m_spawn_area_id
				<< " | " << std::setw(8) << it_beg->m_max_weight
				<< " | " << std::setw(8) << it_beg->m_max_score
				<< " ]";

			if (sort.at(0))
				std::sort(it_beg->m_animals.begin(), it_beg->m_animals.end(), AnimalGroup::cmpIdx);
			else if (sort.at(1))
				std::sort(it_beg->m_animals.begin(), it_beg->m_animals.end(), AnimalGroup::cmpHighestScore);
			else if (sort.at(2))
				std::sort(it_beg->m_animals.begin(), it_beg->m_animals.end(), AnimalGroup::cmpLowestScore);
			else if (sort.at(3))
				std::sort(it_beg->m_animals.begin(), it_beg->m_animals.end(), AnimalGroup::cmpHighestWeight);
			else if (sort.at(4))
				std::sort(it_beg->m_animals.begin(), it_beg->m_animals.end(), AnimalGroup::cmpLowestWeight);
			else if (sort.at(5))
				std::sort(it_beg->m_animals.begin(), it_beg->m_animals.end(), AnimalGroup::cmpFurTypeA);

			if (expand_groups == true)
			{
				ImGui::SetNextItemOpen(true);
			}
			else if (collapse_groups)
			{
				ImGui::SetNextItemOpen(false);
			}

			if (ImGui::CollapsingHeader(group_info.str().c_str()))
			{
				if (ImGui::BeginTable("animal_table", 9, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable |
									  ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter))
				{
					ImGui::TableSetupColumn("Idx");
					ImGui::TableSetupColumn("Gender");
					ImGui::TableSetupColumn("Weight");
					ImGui::TableSetupColumn("Score");
					ImGui::TableSetupColumn("Is GO");
					ImGui::TableSetupColumn("Fur Type");
					ImGui::TableSetupColumn("Fur Type Id");
					ImGui::TableSetupColumn("Visual Variation Seed");
					ImGui::TableSetupColumn("Operations");
					ImGui::TableHeadersRow();
					ImGui::TableNextRow();

					auto it_animal_beg = it_beg->m_animals.begin();
					for (; it_animal_beg != it_beg->m_animals.end(); ++it_animal_beg)
					{
						int idx = std::distance(it_beg->m_animals.begin(), it_animal_beg);
						ImGui::TableSetColumnIndex(0);
						ImGui::Text(std::to_string(it_animal_beg->m_idx).c_str());
						ImGui::TableSetColumnIndex(1);
						ImGui::Text(it_animal_beg->m_gender.c_str());
						ImGui::TableSetColumnIndex(2);
						ImGui::Text(std::to_string(it_animal_beg->m_weight).c_str());
						ImGui::TableSetColumnIndex(3);
						ImGui::Text(std::to_string(it_animal_beg->m_score).c_str());
						ImGui::TableSetColumnIndex(4);
						ImGui::Text(std::to_string(it_animal_beg->m_is_great_one).c_str());
						ImGui::TableSetColumnIndex(5);
						ImGui::Text(it_animal_beg->m_fur_type.c_str());
						ImGui::TableSetColumnIndex(6);
						ImGui::Text(std::to_string(it_animal_beg->m_fur_type_id).c_str());
						ImGui::TableSetColumnIndex(7);
						ImGui::Text(std::to_string(it_animal_beg->m_visual_variation_seed).c_str());
						ImGui::TableSetColumnIndex(8);

						ImGui::PushID(it_animal_beg->m_visual_variation_seed);
						if (ImGui::Button("Edit"))
						{
							edit_animal_idx = static_cast<int>(it_animal_beg->m_idx);
							edit_animal_gender = it_animal_beg->m_gender;
							edit_animal_weight = it_animal_beg->m_weight;
							edit_animal_score = it_animal_beg->m_score;
							edit_animal_str_igo = std::to_string(it_animal_beg->m_is_great_one);
							edit_animal_str_fur_type = it_animal_beg->m_fur_type;
						}

						ImVec2 center = ImGui::GetMainViewport()->GetCenter();
						ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
						if (ImGui::BeginPopupContextItem((const char*)0, ImGuiPopupFlags_MouseButtonLeft))
						{
							const char* items[] = { "male", "female" };
							static int item_current_idx = 0; // Here we store our selection data as an index.
							const char* combo_preview_value = edit_animal_gender.c_str();  // Pass in the preview value visible before opening the combo (it could be anything)
							if (ImGui::BeginCombo("Gender", combo_preview_value))
							{
								for (int n = 0; n < 2; n++)
								{
									const bool is_selected = (item_current_idx == n);
									if (ImGui::Selectable(items[n], is_selected))
										item_current_idx = n;

									// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
									if (is_selected)
										ImGui::SetItemDefaultFocus();
								}
								edit_animal_gender = items[item_current_idx];
								ImGui::EndCombo();
							}
							//ImGui::InputText("Gender", &edit_animal_gender);
							ImGui::InputFloat("Weight", &edit_animal_weight);
							ImGui::InputFloat("Score", &edit_animal_score);
							ImGui::InputText("IGO", &edit_animal_str_igo);
							ImGui::InputText("Fur Type", &edit_animal_str_fur_type);
							if (ImGui::Button("Save"))
							{
								uint32_t visual_variation_seed = Animal::CreateVisualVariationSeed(animal_type, edit_animal_gender, edit_animal_str_fur_type);
								const std::shared_ptr<Animal> animal =
									Animal::Create(animal_type, Animal::ResolveGender(edit_animal_gender),
												   edit_animal_weight, edit_animal_score, boost::lexical_cast<bool>(edit_animal_str_igo),
												   visual_variation_seed, edit_animal_idx, it_beg->m_index);
								if (animal->IsValid())
								{
									animal_population->ReplaceAnimal(animal, it_beg->m_index, it_animal_beg->m_idx);
								}
								file_handler = std::make_shared<FileHandler>(Endian::Little, input_file_path);
								reserve_data = std::make_shared<ReserveData>(ReserveData::ResolveFileNameToInt(input_file_path.filename().generic_string()));
								animal_population = std::make_unique<AnimalPopulation>(file_handler, reserve_data);
								if (animal_population->Deserialize())
								{
									animal_population->MapAnimals();
								}
								ImGui::CloseCurrentPopup();
							}
							ImGui::SameLine();
							if (ImGui::Button("Exit"))
							{
								ImGui::CloseCurrentPopup();
							}

							ImGui::EndPopup();
						}

						ImGui::PopID();

						if (idx < it_beg->m_size - 1)
							ImGui::TableNextRow();
					}

					ImGui::EndTable();
				}
			}

		}

		expand_groups = false;
		collapse_groups = false;
	}
}