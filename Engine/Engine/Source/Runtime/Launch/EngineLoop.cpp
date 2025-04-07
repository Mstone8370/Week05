#include "EngineLoop.h"
#include "ImGuiManager.h"
#include "Level.h"
#include "PropertyEditor/ViewportTypePanel.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/UnrealEd.h"
#include "UnrealClient.h"
#include "slate/Widgets/Layout/SSplitter.h"
#include "LevelEditor/SLevelEditor.h"
#include "World.h"
#include "GameFramework/Actor.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }
    int zDelta = 0;
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            //UGraphicsDevice 객체의 OnResize 함수 호출
            if (FEngineLoop::GraphicDevice.SwapChain)
            {
                FEngineLoop::GraphicDevice.OnResize(hWnd);
            }
            for (int i = 0; i < 4; i++)
            {
                if (GEngineLoop.GetLevelEditor())
                {
                    if (GEngineLoop.GetLevelEditor()->GetViewports()[i])
                    {
                        GEngineLoop.GetLevelEditor()->GetViewports()[i]->ResizeViewport(FEngineLoop::GraphicDevice.SwapchainDesc);
                    }
                }
            }
        }
     Console::GetInstance().OnResize(hWnd);
    // ControlPanel::GetInstance().OnResize(hWnd);
    // PropertyPanel::GetInstance().OnResize(hWnd);
    // Outliner::GetInstance().OnResize(hWnd);
    // ViewModeDropdown::GetInstance().OnResize(hWnd);
    // ShowFlags::GetInstance().OnResize(hWnd);
        if (GEngineLoop.GetUnrealEditor())
        {
            GEngineLoop.GetUnrealEditor()->OnResize(hWnd);
        }
        ViewportTypePanel::GetInstance().OnResize(hWnd);
        break;
    case WM_MOUSEWHEEL:
        if (ImGui::GetIO().WantCaptureMouse)
            return 0;
        zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // 휠 회전 값 (+120 / -120)
        if (GEngineLoop.GetLevelEditor())
        {
            if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->IsPerspective())
            {
                if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetIsOnRBMouseClick())
                {
                    GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetCameraSpeedScalar(
                        static_cast<float>(GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar() + zDelta * 0.01)
                    );
                }
                else
                {
                    GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->CameraMoveForward(zDelta * 0.1f);
                }
            }
            else
            {
                FEditorViewportClient::SetOthoSize(-zDelta * 0.01f);
            }
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

FGraphicsDevice FEngineLoop::GraphicDevice;
FRenderer FEngineLoop::Renderer;
FResourceMgr FEngineLoop::ResourceManager;
uint32 FEngineLoop::TotalAllocationBytes = 0;
uint32 FEngineLoop::TotalAllocationCount = 0;

FEngineLoop::FEngineLoop()
    : hWnd(nullptr)
    , UIMgr(nullptr)
    , GLevel(nullptr)
    , LevelEditor(nullptr)
    , UnrealEditor(nullptr)
{
}

int32 FEngineLoop::PreInit()
{
    return 0;
}

int32 FEngineLoop::Init(HINSTANCE hInstance)
{
    /* must be initialized before window. */
    UnrealEditor = new UnrealEd();
    UnrealEditor->Initialize();

    WindowInit(hInstance);
    GraphicDevice.Initialize(hWnd);
    Renderer.Initialize(&GraphicDevice);

    UIMgr = new UImGuiManager;
    UIMgr->Initialize(hWnd, GraphicDevice.Device, GraphicDevice.DeviceContext);

    ResourceManager.Initialize(&Renderer, &GraphicDevice);
    LevelEditor = new SLevelEditor();
    LevelEditor->Initialize();

    GWorld = FObjectFactory::ConstructObject<UWorld>();
    WorldContexts.Add({GWorld, EWorldType::Editor});
    GLevel = FObjectFactory::ConstructObject<ULevel>();
    GWorld->Level =GLevel;
    GLevel->Initialize(EWorldType::Editor);

    WorldContexts.Add({});

    return 0;
}


void FEngineLoop::Render()
{
    GraphicDevice.Prepare();
    if (LevelEditor->IsMultiViewport())
    {
        std::shared_ptr<FEditorViewportClient> ViewportClient = GetLevelEditor()->GetActiveViewportClient();
        for (int i = 0; i < 4; ++i)
        {
            LevelEditor->SetViewportClient(i);
            Renderer.PrepareRender(GLevel);
            Renderer.RenderScene(GetLevel(),LevelEditor->GetActiveViewportClient());

            LevelEditor->GetActiveViewportClient()->UpdatePrevMatrix();
            Renderer.OnEndRender();
        }
        GetLevelEditor()->SetViewportClient(ViewportClient);
    }
    else
    {
        Renderer.PrepareRender(GLevel);
        Renderer.RenderScene(GetLevel(),LevelEditor->GetActiveViewportClient());

        Renderer.SampleAndProcessSRV(LevelEditor->GetActiveViewportClient());

        Renderer.PostProcess(LevelEditor->GetActiveViewportClient());

        Renderer.RenderFullScreenQuad(LevelEditor->GetActiveViewportClient());

        LevelEditor->GetActiveViewportClient()->UpdatePrevMatrix();
        Renderer.OnEndRender();
    }
}

void FEngineLoop::Tick()
{
    LARGE_INTEGER Frequency;
    const double TargetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

    QueryPerformanceFrequency(&Frequency);

    LARGE_INTEGER StartTime, EndTime;
    double ElapsedTime = 1.0;

    while (bIsExit == false)
    {
        QueryPerformanceCounter(&StartTime);

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); // 키보드 입력 메시지를 문자메시지로 변경
            DispatchMessage(&msg);  // 메시지를 WndProc에 전달

            if (msg.message == WM_QUIT)
            {
                bIsExit = true;
                break;
            }
        }

        switch (WorldContexts[curWorldContextIndex].worldType)
        {
        case EWorldType::Editor:
            EditorTick(ElapsedTime);
            break;
        case EWorldType::PIE:
            PIETick(ElapsedTime);
            break;
        }

        if (bShouldChangeWorldType == true)
        {
            TogglePIE();
            bShouldChangeWorldType = false;
        }

        do
        {
            Sleep(0);
            QueryPerformanceCounter(&EndTime);
            ElapsedTime = (EndTime.QuadPart - StartTime.QuadPart) * 1000.0 / Frequency.QuadPart;
        }
        while (ElapsedTime < TargetFrameTime);
    }
}

void FEngineLoop::EditorTick(double DeltaTime)
{
    Input();
    GLevel->Tick(DeltaTime);
    LevelEditor->Tick(DeltaTime);

    Render();
    UIMgr->BeginFrame();
    UnrealEditor->Render();

    Console::GetInstance().Draw();

    UIMgr->EndFrame();

    // Pending 처리된 오브젝트 제거
    GUObjectArray.ProcessPendingDestroyObjects();

    GraphicDevice.SwapBuffer();
}

void FEngineLoop::PIETick(double DeltaTime)
{
    Input();
    GLevel->Tick(DeltaTime);
    for (auto& actor : GLevel->GetActors())
    {
        actor->SetActorRotation(actor->GetActorRotation() + FVector(0.1,0.1,0.1));
    }
    Render();

    UIMgr->BeginFrame();
    UnrealEditor->RenderPIE();

    UIMgr->EndFrame();

    // Pending 처리된 오브젝트 제거
    GUObjectArray.ProcessPendingDestroyObjects();

    GraphicDevice.SwapBuffer();
}

void FEngineLoop::TogglePIE()
{
    curWorldContextIndex == 0 ? curWorldContextIndex = 1 : curWorldContextIndex = 0;
    if (curWorldContextIndex == 1)
    {
        WorldContexts[1] = {Cast<UWorld>(WorldContexts[0].World->Duplicate()), EWorldType::PIE};
        WorldContexts[1].World->Level->Initialize(EWorldType::PIE);
        uint32 NewFlag = LevelEditor->GetActiveViewportClient()->GetShowFlag() & 14;
        LevelEditor->GetActiveViewportClient()->SetShowFlag(NewFlag);
    }
    else
    {
        uint32 NewFlag = LevelEditor->GetActiveViewportClient()->GetShowFlag() | 1;
        LevelEditor->GetActiveViewportClient()->SetShowFlag(NewFlag);
    }
    GWorld = WorldContexts[curWorldContextIndex].World;
    GLevel = GWorld->Level;
}

float FEngineLoop::GetAspectRatio(IDXGISwapChain* swapChain) const
{
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    return static_cast<float>(desc.BufferDesc.Width) / static_cast<float>(desc.BufferDesc.Height);
}

void FEngineLoop::Input()
{
    if (GetAsyncKeyState('M') & 0x8000)
    {
        if (!bTestInput)
        {
            bTestInput = true;
            if (LevelEditor->IsMultiViewport())
            {
                LevelEditor->DisableMultiViewport();
            }
            else
                LevelEditor->EnableMultiViewport();
        }
    }
    else
    {
        bTestInput = false;
    }
}

void FEngineLoop::Exit()
{
    LevelEditor->Release();

    GLevel->Release();
    delete GLevel;

    UIMgr->Shutdown();
    delete UIMgr;

    ResourceManager.Release(&Renderer);
    Renderer.Release();
    GraphicDevice.Release();
}


void FEngineLoop::WindowInit(HINSTANCE hInstance)
{
    WCHAR WindowClass[] = L"JungleWindowClass";

    WCHAR Title[] = L"Game Tech Lab";

    WNDCLASSW wndclass = {0};
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = WindowClass;

    RegisterClassW(&wndclass);

    hWnd = CreateWindowExW(
        0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 1000,
        nullptr, nullptr, hInstance, nullptr
    );
}
