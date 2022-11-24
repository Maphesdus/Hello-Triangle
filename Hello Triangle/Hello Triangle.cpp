/*	Hello Triangle
	Direct3D 11 Up and Running
	https://antongerdelan.net/opengl/d3d11.html
*/



#ifndef UNICODE
#define UNICODE
#endif 

//#include <windows.h>
#include<assert.h> 

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>       // D3D interface
#include <dxgi.h>        // DirectX driver interface
#include <d3dcompiler.h> // shader compiler

#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Start Direct3D:
ID3D11Device* device_ptr = NULL;
ID3D11DeviceContext* device_context_ptr = NULL;
IDXGISwapChain* swap_chain_ptr = NULL;
ID3D11RenderTargetView* render_target_view_ptr = NULL;



// wWinMain:
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    int WINDOW_STYLE = 0;
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    const wchar_t WINDOW_NAME[] = L"Hello Triangle";
    

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        WINDOW_STYLE,           // Optional window styles.
        CLASS_NAME,             // Window class
        WINDOW_NAME,            // Window text
        WS_OVERLAPPEDWINDOW,    // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);




    //  DXGI_SWAP_CHAIN_DESC structure (dxgi.h)
    //  https://learn.microsoft.com/en-us/windows/win32/api/dxgi/ns-dxgi-dxgi_swap_chain_desc
    DXGI_SWAP_CHAIN_DESC swap_chain_descr = { 0 };
    swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
    swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1;
    //swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Default color output
    swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // Default color output with gamma correction
    swap_chain_descr.SampleDesc.Count = 1; // multisampling anti-aliasing
    swap_chain_descr.SampleDesc.Quality = 0;
    swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_descr.BufferCount = 1; // Number of back buffers to add to the swap chain
    swap_chain_descr.OutputWindow = hwnd; // Ties output buffers to the window using Win32 handle
    swap_chain_descr.Windowed = true;

    //-------------------------------
    // Direct3D startup code:

    D3D_FEATURE_LEVEL feature_level;
    UINT flags1 = D3D11_CREATE_DEVICE_SINGLETHREADED;
    UINT flags2 = D3DCOMPILE_ENABLE_STRICTNESS;




    // Here I have added extra debug output to the function "flags" when the program is built in debug mode:
#if defined( DEBUG ) || defined( _DEBUG )
    flags1 |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr1 = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags1,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &swap_chain_descr,
        &swap_chain_ptr,
        &device_ptr,
        &feature_level,
        &device_context_ptr);

    assert(S_OK == hr1 && swap_chain_ptr && device_ptr && device_context_ptr);

    /* The output images from Direct3D are called "Render Targets."
    We can get a view pointer to ours now, by fetching it from our swap chain. */

    ID3D11Texture2D* framebuffer;
    hr1 = swap_chain_ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&framebuffer);
    assert(SUCCEEDED(hr1));

    hr1 = device_ptr->CreateRenderTargetView(framebuffer, 0, &render_target_view_ptr);
    assert(SUCCEEDED(hr1));
    framebuffer->Release();
    
    // D3D Compile From File:
    // Builds both shaders (vertex shader and pixel shader)

    ID3DBlob* vs_blob_ptr = NULL, * ps_blob_ptr = NULL, * error_blob = NULL;

    // COMPILE VERTEX SHADER:
    HRESULT hr2 = D3DCompileFromFile(
        L"shaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs_main",
        "vs_5_0",
        flags2,
        0,
        &vs_blob_ptr,
        &error_blob);

    if (FAILED(hr2)) {
        if (error_blob) {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (vs_blob_ptr) { vs_blob_ptr->Release(); }
        assert(false);
    }

    // COMPILE PIXEL SHADER:
    hr2 = D3DCompileFromFile(
        L"shaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps_main",
        "ps_5_0",
        flags2,
        0,
        &ps_blob_ptr,
        &error_blob);

    if (FAILED(hr2)) {
        if (error_blob) {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (ps_blob_ptr) { ps_blob_ptr->Release(); }
        assert(false);
    }

    // Now we need to call CreateVertexShader() and CreatePixelShader() with our shader blobs
    // to get pointers that let us use the shaders for drawing.

    ID3D11VertexShader* vertex_shader_ptr = NULL;
    ID3D11PixelShader* pixel_shader_ptr = NULL;

    hr2 = device_ptr->CreateVertexShader(
        vs_blob_ptr->GetBufferPointer(),
        vs_blob_ptr->GetBufferSize(),
        NULL,
        &vertex_shader_ptr);

    assert(SUCCEEDED(hr2));

    hr2 = device_ptr->CreatePixelShader(
        ps_blob_ptr->GetBufferPointer(),
        ps_blob_ptr->GetBufferSize(),
        NULL,
        &pixel_shader_ptr);

    assert(SUCCEEDED(hr2));


    // We must also create an input layout to describe how vertex data memory
    // from a buffer should map to the input variables for the vertex shader.
    // We do this by filling out an array of D3D11_INPUT_ELEMENT_DESC structs,
    // and passing that to CreateInputLayout().

    ID3D11InputLayout* input_layout_ptr = NULL;

    // Vertex Shader input variables:
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        // SemanticName (LPCSTR), SemanticIndex (UINT), Format (DXGI_FORMAT), InputSlot (UINT), AlignedByteOffset (UINT), InputSlotClass (D3D11_INPUT_CLASSIFICATION), InstanceDataStepRate (UINT)

        // XYZ Position:
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

        // Color:
        //{ "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      
        // Normal Map:
        //{ "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      
        // Texture Coodrinates:
        //{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // Create Input Layout:
    hr2 = device_ptr->CreateInputLayout(
        inputElementDesc,
        ARRAYSIZE(inputElementDesc),
        vs_blob_ptr->GetBufferPointer(),
        vs_blob_ptr->GetBufferSize(),
        &input_layout_ptr);

    assert(SUCCEEDED(hr2));



    // Create a Vertex Buffer

    float vertex_data_array[] = {
        0.0f,  0.5f,  0.0f,  // point at top
        0.5f, -0.5f,  0.0f,  // point at bottom-right
        -0.5f, -0.1f,  1.0f, // point at bottom-left
    };

    UINT vertex_stride = 3 * sizeof(float); // the bytes between each vertex (XYZ so 3 floats)
    UINT vertex_offset = 0; // the offset into the buffer to start reading (0)
    UINT vertex_count = 3; // the number of vertices (3)

    ID3D11Buffer* vertex_buffer_ptr = NULL; // Vertex buffer pointer

    { /*** load mesh data into vertex buffer **/
        D3D11_BUFFER_DESC vertex_buff_descr = {}; // Required for CreateBuffer()
        vertex_buff_descr.ByteWidth = sizeof(vertex_data_array);
        //vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT; // Usage helps the driver optimisation, based on how often the buffer values will change.
        vertex_buff_descr.Usage = D3D11_USAGE_IMMUTABLE; // Since we don't need to update our vertex buffer (yet), usage could be set to IMMUTABLE for optimal usage for now.
        vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Binds a vertex buffer to the BindFlags. Don't set another type by accident! 
        D3D11_SUBRESOURCE_DATA sr_data = { 0 }; // Points to the actual vertex array data (array of floats).
        sr_data.pSysMem = vertex_data_array;

        // CreateBuffer():
        HRESULT hr3 = device_ptr->CreateBuffer(&vertex_buff_descr, &sr_data, &vertex_buffer_ptr);
        assert(SUCCEEDED(hr3));
    }


    //-------------------------------
    // Set up the message loop:
    MSG msg = { };
    bool should_close = false;

    // Message Loop (main loop of Direct3D program):
    while (!should_close)
    {
        /**** handle user input and other window events ****/
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) { break; }

        
        // Drawing Each Frame:
        /*** TODO: RENDER A FRAME HERE WITH DIRECT3D ***/

        // Render Frame Loop:
        { /*** RENDER A FRAME ***/
            // Clear the back buffer to "cornflower blue" for the new frame:
            float background_colour[4] = { 0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f };
            device_context_ptr->ClearRenderTargetView(render_target_view_ptr, background_colour);

            // Create the viewport for drawing within the window:
            /**** Rasteriser state - set viewport area *****/
            RECT winRect;
            GetClientRect(hwnd, &winRect);

            D3D11_VIEWPORT viewport = {
              0.0f,
              0.0f,
              (FLOAT)(winRect.right - winRect.left),
              (FLOAT)(winRect.bottom - winRect.top),
              0.0f,
              1.0f
            };

            /**** Rasteriser State (RS) *****/
            // RS Set Viewports:
            device_context_ptr->RSSetViewports(1, &viewport);


            /**** Output Merger (OM) *****/
            // OM Set Render Targets:
            // Tells the Output Merger to use our render target.
            device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, NULL);


            // Set the Input Assembler (IA):
            // Maps how the vertex shader inputs should be read from vertex buffer.


            // IA Set Primitive Topology:
            // Tell the Input Assember to expect vertices in the primitive topology to correspond to the primitive topology triangle list,
            // which means every 3 vertices should form a separate triangle.
            device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // IA Set Input Layout:
            // Set input layout for Input Assembler memory.
            device_context_ptr->IASetInputLayout(input_layout_ptr);

            // IA Set Vertex Buffers:
            // Feed vertex data from the vertex buffer to vertex shaders,
            // and also give it the vertex memory stride and vertex memory offset.
            device_context_ptr->IASetVertexBuffers(0, 1, &vertex_buffer_ptr, &vertex_stride, &vertex_offset);

            // Set the Vertex Shader (VS) and Pixel Shader (PS):
            /*** Set vertex shader to use, and pixel shader to use, and constant buffers for each ***/
            device_context_ptr->VSSetShader(vertex_shader_ptr, NULL, 0); // Vertex Shader (VS)
            device_context_ptr->PSSetShader(pixel_shader_ptr, NULL, 0); // Pixel Shader (PS)

            // Draw():
            /*** Draw the vertex buffer with the shaders ****/
            device_context_ptr->Draw(vertex_count, 0);

            // Present():
            // Present the Frame (Swap the Buffers).
            /**** Swap the back and front buffers (show the frame we just drew). ****/
            swap_chain_ptr->Present(1, 0);
        } // End of Render Frame Loop
    } // End of Message Loop (main loop)

    return 0;
}


// WindowProc:
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}