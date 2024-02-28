
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <initguid.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>

#if !SHIP_MODE
#include <dxgidebug.h>
IDXGIDebug1* dxgi_debug;
#endif

#include "4ed_directx_render.cpp"

internal LRESULT CALL_CONVENTION
win32_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

internal b32
win32_gl_create_window(HWND *wnd_out, DWORD style, RECT rect){
    
    local_persist b32 srgb_support = false;
    local_persist b32 first_call = true;
    
    b32 result = false;
    
    *wnd_out = 0;
    
    Assert( g_dx11.initialized == 0 );
    Assert( g_dx11.device == 0 );
    Assert( g_dx11.context == 0 );
    Assert( g_dx11.swap_chain == 0 );
    Assert( g_dx11.sampler == 0 );
    Assert( g_dx11.render_target_view == 0 );
    Assert( g_dx11.gpu_program.vertex == 0 );
    Assert( g_dx11.gpu_program.layout == 0 );
    Assert( g_dx11.gpu_program.pixel == 0 );
    Assert( g_dx11.gpu_program.valid == 0 );
    Assert( g_dx11.vertex_buffer == 0 );
    Assert( g_dx11.constants_buffer == 0 );
    Assert( g_dx11.texture_count == 0 );
    
    g_dx11 = { };
    
    HINSTANCE this_instance = GetModuleHandle(0);
    
    HWND wnd = 0;
    
    ID3D11Device* base_device = 0;
    ID3D11DeviceContext* base_device_context = 0;
    IDXGIFactory2* dxgi_factory = 0;
    
    ID3D11BlendState* blend_state = 0;
    ID3D11RasterizerState1* rasterizer_state = 0;
    
    do {
        
        // NOTE(simon, 28/02/24): There is nothing in the code suggesting that this function could
        // be called several time. If it is called several time, we would need to make sure that
        // we cleaned up previous DirectX resources. The reason this function could be called twice
        // would be if it failed previously, and in that case we clean up everything before
        // exiting the function so we should be good. Still we assume it's only ever call once.
        Assert( first_call );
        
        if (first_call){
            
            first_call = false;
            
            log_os( " Registering graphics class...\n" );
            
            WNDCLASSW wndclass = {};
            wndclass.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
            wndclass.lpfnWndProc = win32_proc;
            wndclass.hIcon = LoadIconW(GetModuleHandle(0), L"main");
            wndclass.hInstance = this_instance;
            wndclass.lpszClassName = L"GRAPHICS-WINDOW-NAME";
            if (RegisterClassW(&wndclass) == 0){
                log_os("  Failed.\n");
                break;
            }
        }
        
        log_os( " Creating graphics window...\n" );
        
        wnd = CreateWindowExW(0, L"GRAPHICS-WINDOW-NAME", L"GRAPHICS", style,
                              CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                              0, 0, this_instance, 0);
        
        if (wnd == 0) {
            log_os( "  Failed.\n" );
            break;
        }
        
        log_os( " Creating a d3d11 hardware device and context...\n" );
        // NOTE(simon, 28/02/24): We are creating a directx 11.1 device and context (supported
        // since windows 8).
        D3D_FEATURE_LEVEL feature_levels[ ] = { D3D_FEATURE_LEVEL_11_1 };
        
        u32 device_flags = 0;
        // device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        
#if !SHIP_MODE
        device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        
        HRESULT hr = D3D11CreateDevice( 0, D3D_DRIVER_TYPE_HARDWARE, 0, device_flags, feature_levels, ArrayCount( feature_levels ), D3D11_SDK_VERSION, &base_device, 0, &base_device_context );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            log_os( " Creating a d3d11 software (WARP) device and context...\n" );
            // NOTE(simon, 28/02/24): Try creating a high performance software device as a fallback.
            hr = D3D11CreateDevice( 0, D3D_DRIVER_TYPE_WARP, 0, device_flags, feature_levels, ArrayCount( feature_levels ), D3D11_SDK_VERSION, &base_device, 0, &base_device_context );
        }
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        log_os( " Creating a ID3D11Device1...\n" );
        hr = base_device->QueryInterface( __uuidof( ID3D11Device1 ), ( void** ) &g_dx11.device );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        log_os( " Creating a ID3D11DeviceContext1...\n" );
        hr = base_device_context->QueryInterface( __uuidof( ID3D11DeviceContext1 ), ( void** ) &g_dx11.context );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        ID3D11Device1* device = g_dx11.device;
        ID3D11DeviceContext1* context = g_dx11.context;
        
#if !SHIP_MODE
        log_os( " Getting ID3D11InfoQueue. This is not important if you're not debugging graphics...\n" );
        ID3D11InfoQueue* info;
        hr = device->QueryInterface( __uuidof( ID3D11InfoQueue ), ( void** ) &info );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
        } else {
            info->SetBreakOnSeverity( D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE );
            info->SetBreakOnSeverity( D3D11_MESSAGE_SEVERITY_ERROR, TRUE );
            info->Release( );
        }
        
        log_os( " Getting IDXGIDebug1. This is not important if you're not debugging graphics...\n" );
        hr = DXGIGetDebugInterface1( 0, __uuidof( IDXGIDebug1 ), ( void** ) &dxgi_debug );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
        }
#endif
        
        // NOTE(simon, 28/02/24): sRGB should be supported by any hardware now, but there was a
        // check so I added one. The OpenGL version never enables sRGB.
        DXGI_FORMAT back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        UINT format_support = 0;
        
        if ( SUCCEEDED( device->CheckFormatSupport( back_buffer_format, &format_support ) ) ) {
            u32 required = D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DISPLAY;
            srgb_support = ( ( format_support & required ) == required );
        }
        
        if ( !srgb_support ) {
            log_os( " sRBG back buffer not supported.\n" );
            back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;
        } else {
            log_os( " sRBG back buffer supported.\n" );
        }
        
        log_os( " Creating a IDXGIFactory2...\n" );
        hr = CreateDXGIFactory( __uuidof( IDXGIFactory2 ), ( void** ) &dxgi_factory );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = { };
        // NOTE(simon, 28/02/24): Can't request sRGB format here when using FLIP_* swap chain. It's
        // requested when creating the render target view.
        swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = 2;
        swap_chain_desc.Scaling = DXGI_SCALING_NONE;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        
        log_os( " Creating a IDXGISwapChain1...\n" );
        hr = dxgi_factory->CreateSwapChainForHwnd( device, wnd, &swap_chain_desc, 0, 0, &g_dx11.swap_chain );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        // NOTE(simon, 28/02/24): We setup alpha blending here as it's always on in 4coder.
        D3D11_BLEND_DESC blend_state_desc = { };
        blend_state_desc.RenderTarget[ 0 ].BlendEnable = TRUE;
        blend_state_desc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend_state_desc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_state_desc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
        blend_state_desc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        blend_state_desc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blend_state_desc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_state_desc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        
        log_os( " Creating a blend state...\n" );
        hr = device->CreateBlendState( &blend_state_desc, &blend_state );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        context->OMSetBlendState( blend_state, 0, 0xffffffff );
        
        // NOTE(simon, 28/02/24): Enable scissor and disable backface culling.
        D3D11_RASTERIZER_DESC1 rasterizer_desc = { };
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.DepthClipEnable = TRUE;
        rasterizer_desc.ScissorEnable = TRUE;
        
        log_os( " Creating a rasterizer state...\n" );
        hr = device->CreateRasterizerState1( &rasterizer_desc, &rasterizer_state );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        context->RSSetState( rasterizer_state );
        
        // NOTE(simon, 28/02/24): Not setting depth stencil as 4coder doesn't use it.
        // NOTE(simon, 28/02/24): Swap interval is a parameter of swap_chain->present.
        
        D3D11_SAMPLER_DESC linear_desc = { };
        linear_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        linear_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        linear_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        linear_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        linear_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        linear_desc.MinLOD = 0;
        linear_desc.MaxLOD = D3D11_FLOAT32_MAX;
        
        log_os( " Creating a sampler state...\n" );
        hr = device->CreateSamplerState( &linear_desc, &g_dx11.sampler );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        // NOTE(simon, 28/02/24): We create the vertex buffer, constants buffers and shader here
        // because if we can't create them we won't be able to render anything and so we should
        // just exit the program.
        
        D3D11_BUFFER_DESC vertex_buffer_desc = { };
        // NOTE(simon, 28/02/24): Reserving 400K vertices which is about 11 megabytes and would
        // allow 100K characters. On a 1080p monitor, with 4 by 10 pixels characters we would need
        // (1920/4)*(1080/10) = 51840 characters to fill the screen.
        vertex_buffer_desc.ByteWidth = 400000 * sizeof( Render_Vertex );
        vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        
        log_os( " Creating a vertex buffer...\n" );
        hr = device->CreateBuffer( &vertex_buffer_desc, 0, &g_dx11.vertex_buffer );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        D3D11_BUFFER_DESC constants_buffer_desc = { };
        // NOTE(simon, 28/02/24): constants buffer size needs to be a multiple of 16.
        // NOTE(simon, 28/02/24): The layout is explained where we set the values in the buffer in
        // gl_render.
        constants_buffer_desc.ByteWidth = 32;
        constants_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        constants_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constants_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        
        log_os( " Creating a constants buffer...\n" );
        hr = device->CreateBuffer( &constants_buffer_desc, 0, &g_dx11.constants_buffer );
        
        if ( FAILED( hr ) ) {
            log_os( "  Failed.\n" );
            break;
        }
        
        g_dx11.gpu_program = gl__make_program( gl__vertex, gl__fragment );
        
        if ( !g_dx11.gpu_program.valid ) {
            break;
        }
        
        *wnd_out = wnd;
        // NOTE(simon, 28/02/24): Reserve the first texture slot as a invalid/unbind texture.
        g_dx11.texture_count = 1;
        g_dx11.initialized = true;
        result = true;
        
    } while ( 0 );
    
    if ( !result ) {
        
        if ( wnd ) {
            DestroyWindow( wnd );
            ( *wnd_out ) = 0;
        }
        
        if ( g_dx11.context ) {
            g_dx11.context->OMSetBlendState( 0, 0, 0xffffffff );
            g_dx11.context->RSSetState( 0 );
        }
        
        if ( g_dx11.constants_buffer ) {
            g_dx11.constants_buffer->Release( );
            g_dx11.constants_buffer = 0;
        }
        
        if ( g_dx11.vertex_buffer ) {
            g_dx11.vertex_buffer->Release( );
            g_dx11.vertex_buffer = 0;
        }
        
        if ( g_dx11.gpu_program.valid ) {
            
            if ( g_dx11.gpu_program.vertex ) {
                g_dx11.gpu_program.vertex->Release( );
                g_dx11.gpu_program.vertex = 0;
            }
            
            if ( g_dx11.gpu_program.layout ) {
                g_dx11.gpu_program.layout->Release( );
                g_dx11.gpu_program.layout = 0;
            }
            
            if ( g_dx11.gpu_program.pixel ) {
                g_dx11.gpu_program.pixel->Release( );
                g_dx11.gpu_program.pixel = 0;
            }
        }
        
        // NOTE(simon, 28/02/24): No render target view at this point as it's created in the
        // WM_SIZE message.
        
        if ( g_dx11.sampler ) {
            g_dx11.sampler->Release( );
            g_dx11.sampler = 0;
        }
        
        if ( g_dx11.swap_chain ) {
            g_dx11.swap_chain->Release( );
            g_dx11.swap_chain = 0;
        }
        
        if ( g_dx11.context ) {
            g_dx11.context->Release( );
            g_dx11.context = 0;
        }
        
        if ( g_dx11.device ) {
            g_dx11.device->Release( );
            g_dx11.device = 0;
        }
        
        g_dx11.initialized = false;
        
#if SHIP_MODE
        os_popup_error( "Error", "Window creation failed.");
#endif
    }
    
    if( base_device ) {
        base_device->Release( );
        base_device = 0;
    }
    
    if ( base_device_context ) {
        base_device_context->Release( );
        base_device_context = 0;
    }
    
    if ( dxgi_factory ) {
        dxgi_factory->Release( );
        dxgi_factory = 0;
    }
    
    if ( blend_state ) {
        blend_state->Release( );
        blend_state = 0;
    }
    
    if ( rasterizer_state ) {
        rasterizer_state->Release( );
        rasterizer_state = 0;
    }
    
    return(result);
}

#if !SHIP_MODE

// NOTE(simon, 28/02/24): Only call this when working on 4coder, to make sure we don't do something
// stupid. In SHIP_MODE we let the os clean up resources.
internal void
win32_gl_cleanup( void ) {
    
    if ( dxgi_debug && g_dx11.initialized ) {
        
        OutputDebugString( L"win32_gl_cleanup start report...\n" );
        dxgi_debug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL );
        
        g_dx11.initialized = false;
        
        if ( g_dx11.context ) {
            g_dx11.context->OMSetBlendState( 0, 0, 0xffffffff );
            g_dx11.context->RSSetState( 0 );
        }
        
        for ( u32 i = 1; i < g_dx11.texture_count; i++ ) {
            
            DX11Texture* texture = g_dx11.textures + i;
            
            if ( texture->view ) {
                texture->view->Release( );
                texture->view = 0;
            }
            
            if ( texture->pointer ) {
                texture->pointer->Release( );
                texture->pointer = 0;
            }
        }
        
        g_dx11.texture_count = 0;
        
        if ( g_dx11.constants_buffer ) {
            g_dx11.constants_buffer->Release( );
            g_dx11.constants_buffer = 0;
        }
        
        if ( g_dx11.vertex_buffer ) {
            g_dx11.vertex_buffer->Release( );
            g_dx11.vertex_buffer = 0;
        }
        
        if ( g_dx11.gpu_program.valid ) {
            
            if ( g_dx11.gpu_program.vertex ) {
                g_dx11.gpu_program.vertex->Release( );
                g_dx11.gpu_program.vertex = 0;
            }
            
            if ( g_dx11.gpu_program.layout ) {
                g_dx11.gpu_program.layout->Release( );
                g_dx11.gpu_program.layout = 0;
            }
            
            if ( g_dx11.gpu_program.pixel ) {
                g_dx11.gpu_program.pixel->Release( );
                g_dx11.gpu_program.pixel = 0;
            }
        }
        
        if ( g_dx11.render_target_view ) {
            g_dx11.render_target_view->Release( );
            g_dx11.render_target_view = 0;
        }
        
        if ( g_dx11.sampler ) {
            g_dx11.sampler->Release( );
            g_dx11.sampler = 0;
        }
        
        if ( g_dx11.swap_chain ) {
            g_dx11.swap_chain->Release( );
            g_dx11.swap_chain = 0;
        }
        
        if ( g_dx11.context ) {
            g_dx11.context->Release( );
            g_dx11.context = 0;
        }
        
        if ( g_dx11.device ) {
            g_dx11.device->Release( );
            g_dx11.device = 0;
        }
        
        OutputDebugString( L"win32_gl_cleanup end report (nothing printed after this line means everything is OK)...\n" );
        dxgi_debug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL );
    }
}
#endif