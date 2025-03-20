
#define DX11_MAX_TEXTURE_COUNT 32

struct DX11Texture {
    ID3D11Texture2D* pointer;
    ID3D11ShaderResourceView* view;
};

struct GL_Program {
    ID3D11VertexShader* vertex;
    ID3D11InputLayout* layout;
    ID3D11PixelShader* pixel;
    b8 valid;
};

struct DX11 {
    b8 initialized;
    ID3D11Device1* device;
    ID3D11DeviceContext1* context;
    IDXGISwapChain1* swap_chain;
    ID3D11SamplerState* sampler;
    ID3D11RenderTargetView* render_target_view;
    GL_Program gpu_program;
    ID3D11Buffer* vertex_buffer;
    ID3D11Buffer* constants_buffer;
    
    // NOTE(simon, 28/02/24): To keep the API the same since the OpenGL texture handle are store in
    // other places than the graphics parts (e.g. in the font Face struct), we create an array of
    // textures, and use the indices as texture handles.
    DX11Texture textures[ DX11_MAX_TEXTURE_COUNT + 1 ];
    // NOTE(simon, 28/02/24): The first slot in the array should not be used so we can consider an
    // index of 0 to be invalid. OpenGL should not return 0 for texture handle, so we sort of do
    // the same.
    u32 texture_count;
};

global DX11 g_dx11 = { };

// NOTE(simon, 28/02/24): Passing 0 for texid use the reserved texture in the array, and passing a
// resource view of zero unbinds the resource.
internal void
gl__bind_texture(Render_Target *t, i32 texid){
    if (t->bound_texture != texid){
        DX11Texture* texture = g_dx11.textures + texid;
        g_dx11.context->PSSetShaderResources( 0, 1, &texture->view );
        t->bound_texture = texid;
    }
}

internal void
gl__bind_any_texture(Render_Target *t){
    if (t->bound_texture == 0){
        Assert(t->fallback_texture_id != 0);
        DX11Texture* texture = g_dx11.textures + t->fallback_texture_id;
        g_dx11.context->PSSetShaderResources( 0, 1, &texture->view );
        t->bound_texture = t->fallback_texture_id;
    }
}

internal u32
gl__get_texture(Vec3_i32 dim, Texture_Kind texture_kind){
    
    u32 texid = 0;
    
    if ( g_dx11.texture_count < ArrayCount( g_dx11.textures ) ) {
        
        texid = g_dx11.texture_count;
        g_dx11.texture_count++;
        
    } else {
        
        for ( u32 i = 1; i < g_dx11.texture_count; i++ ) {
            
            DX11Texture* texture = g_dx11.textures + i;
            
            if ( !texture->pointer && !texture->view ) {
                texid = i;
                break;
            }
        }
    }
    
    if ( texid ) {
        
        DX11Texture* texture = g_dx11.textures + texid;
        Assert( texture->pointer == 0 );
        Assert( texture->view == 0 );
        
        D3D11_TEXTURE2D_DESC texture_desc = { 0 };
        texture_desc.Width = dim.x;
        texture_desc.Height = dim.y;
        texture_desc.MipLevels = 1;
        texture_desc.ArraySize = dim.z;
        texture_desc.Format = DXGI_FORMAT_A8_UNORM;
        texture_desc.SampleDesc.Count = 1;
        texture_desc.Usage = D3D11_USAGE_DEFAULT;
        texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texture_desc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_WRITE;
        
        // NOTE(simon, 28/02/24): I initialize the texture with zeros. In practice it doesn't seem
        // to matter, but since the shader use a bilinear filter, the unitialized data in the
        // texture could change the result of the filtering for texel at the edge of a character.
        // I did some tests with the rectangle packer to have a border around character but got the
        // exact same render, so It doesn't matter much.
        D3D11_SUBRESOURCE_DATA* texture_data = push_array_zero( &win32vars.frame_arena, D3D11_SUBRESOURCE_DATA, dim.z );
        u8* initial_data = push_array_zero( &win32vars.frame_arena, u8, dim.x * dim.y );
        
        for ( i32 i = 0; i < dim.z; i++ ) {
            texture_data[ i ].pSysMem = initial_data;
            texture_data[ i ].SysMemPitch = dim.x;
        }
        
        HRESULT hr = g_dx11.device->CreateTexture2D( &texture_desc, texture_data, &texture->pointer );
        
        pop_array( &win32vars.frame_arena, u8, dim.x * dim.y );
        pop_array( &win32vars.frame_arena, D3D11_SUBRESOURCE_DATA, dim.z );
        
        if ( SUCCEEDED( hr ) ) {
            hr = g_dx11.device->CreateShaderResourceView( ( ID3D11Resource* ) texture->pointer, 0, &texture->view );
        }
        
        if ( FAILED( hr ) ) {
            
            // NOTE(simon, 28/02/24): When we fail, we donc decrement the texture count, but the
            // loop at the beginning of the function will reuse texture when
            // texture_count == DX11_MAX_TEXTURE_COUNT.
            texid = 0;
            
            if ( texture->pointer ) {
                texture->pointer->Release( );
                texture->pointer = 0;
            }
            
            if ( texture->view ) {
                texture->view->Release( );
                texture->view = 0;
            }
        }
    }
    
    return(texid);
}

internal b32
gl__fill_texture(Texture_Kind texture_kind, u32 texid, Vec3_i32 p, Vec3_i32 dim, void *data){
    
    // NOTE(simon, 28/02/24): The OpenGL version always returns false.
    b32 result = false;
    
    // NOTE(simon, 28/02/24): In the OpenGL version, if we pass zero as texture handle, the
    // function works on the currently bound texture. In directx we need to get the texture pointer.
    // We could retrieve that from Render_Target->bound_texture, but we don't have that as a
    // parameter to this function and don't want to change the signature since it's used by the
    // font rendering code and other platforms. Fortunately the only call that specified 0 for the
    // texture handle was for the creation of the fallback texture in gl_render, and we can modify
    // that call to pass the fallback texture handle.
    Assert( texid != 0 );
    
    if (dim.x > 0 && dim.y > 0 && dim.z > 0){
        
        DX11Texture* texture = g_dx11.textures + texid;
        
        D3D11_BOX box = { };
        box.left = p.x;
        box.right = p.x + dim.x;
        box.top = p.y;
        box.bottom = p.y + dim.y;
        box.front = 0;
        box.back = 1;
        
        u32 sub_resource_index = D3D11CalcSubresource( 0 /* MipSlice */, p.z /* ArraySlice */, 1 /* MipLevels */ );
        g_dx11.context->UpdateSubresource( texture->pointer, sub_resource_index, &box, data, dim.x, dim.x * dim.y );
    }
    
    return(result);
}

internal void gl__free_texture( u32 texid ) {
    
    if ( texid ) {
        
        DX11Texture* texture = g_dx11.textures + texid;
        
        if ( texture->view ) {
            texture->view->Release( );
            texture->view = 0;
        }
        
        if ( texture->pointer ) {
            texture->pointer->Release( );
            texture->pointer = 0;
        }
    }
}

char *gl__vertex = R"foo(

// NOTE(simon, 28/02/24): The layout of this is (constants are store in 16 bytes vectors (4 floats))
// vector1: view_m._11, view_m._12, 0, 0
// vector2: view_m._21, view_m._22, view_t.x, view_t.y
cbuffer constants : register( b0 ) {
    row_major float2x2 view_m;
    float2 view_t;
}

struct input_t {
    float2 vertex_p : POSITION;
    float3 vertex_t : UVW;
    float4 vertex_c : COLOR;
    float vertex_ht : THICKNESS;
};

struct output_t {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 uvw : UVW;
    float2 xy : XY;
    float2 adjusted_half_dim: HALF_DIM;
    float half_thickness : THICKNESS;
};

output_t main(input_t input) {

    output_t output;

    output.position = float4( mul( view_m, ( input.vertex_p - view_t ) ), 0.0, 1.0 );
    // NOTE(simon, 28/02/24): The input colors are BGRA, we need them as RGBA.
    output.color = input.vertex_c.zyxw;
    output.uvw = input.vertex_t;
    output.xy = input.vertex_p;
    output.half_thickness = input.vertex_ht;

    float2 center = input.vertex_t.xy;
    float2 half_dim = abs( input.vertex_p - center );
    output.adjusted_half_dim = half_dim - input.vertex_t.zz + float2( 0.5, 0.5 );

    return output;
}
)foo";

char *gl__fragment = R"foo(

struct input_t {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 uvw : UVW;
    float2 xy : XY;
    float2 adjusted_half_dim: HALF_DIM;
    float half_thickness : THICKNESS;
};

Texture2DArray alpha : register( t0 );
SamplerState alpha_sampler : register( s0 );

float rectangle_sd( float2 p, float2 b ) {

    float2 d = abs( p ) - b;
    return( length( max( d, float2( 0.0, 0.0 ) ) ) + min( max( d.x, d.y ), 0.0 ) );
}

float4 main( input_t input ) : SV_TARGET {

    float has_thickness = step( 0.49, input.half_thickness );
    float does_not_have_thickness = 1.0 - has_thickness;

    float sample_value = alpha.Sample( alpha_sampler, input.uvw ).a;
    sample_value *= does_not_have_thickness;

    float2 center = input.uvw.xy;
    float roundness = input.uvw.z;
    float sd = rectangle_sd( input.xy - center, input.adjusted_half_dim );
    sd = sd - roundness;
    sd = abs( sd + input.half_thickness ) - input.half_thickness;
    float shape_value = 1.0 - smoothstep(-1.0, 0.0, sd);
    shape_value *= has_thickness;

    float4 result = float4( input.color.xyz, input.color.a * ( sample_value + shape_value ) );
    return result;
}
)foo";

// NOTE(simon, 28/02/24): This function is not generic. It can compile any shader, but the vertex
// input layout is fixed. 4coder only has one vertex format and shader, so we could remove this
// function and move its content in the win32_gl_create_window. I removed the header parameter as
// it's not useful in directx.
internal GL_Program
gl__make_program( char* vertex, char* pixel ) {
    
    GL_Program result = { };
    
    u32 vertex_length = 0;
    
    while ( vertex && vertex[ vertex_length ] != 0 ) {
        vertex_length++;
    }
    
    u32 pixel_length = 0;
    
    while ( pixel && pixel[ pixel_length ] != 0 ) {
        pixel_length++;
    }
    
    ID3DBlob* vs_blob = 0;
    ID3DBlob* vs_error_blob = 0;
    ID3D11VertexShader* vertex_shader = 0;
    ID3D11InputLayout* input_layout = 0;
    
    ID3DBlob* ps_blob = 0;
    ID3DBlob* ps_error_blob = 0;
    ID3D11PixelShader* pixel_shader = 0;
    
    do {
        
        HRESULT hr = D3DCompile( vertex, vertex_length, 0, 0, 0, "main", "vs_5_0", 0, 0, &vs_blob, &vs_error_blob );
        
        if ( FAILED( hr ) ) {
            log_os( "Failed to compile vertex shader.\n" );
            
            if ( vs_error_blob ) {
                u8* error_message = ( u8* ) vs_error_blob->GetBufferPointer( );
                u32 length = ( u32 ) vs_error_blob->GetBufferSize( );
                log_os( "vertex shader error:\n%.*s\n", length, error_message );
            }
            
            break;
        }
        
        hr = g_dx11.device->CreateVertexShader( vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), 0, &vertex_shader );
        
        if ( FAILED( hr ) ) {
            log_os( "Failed to create a vertex shader.\n" );
            break;
        }
        
        
        D3D11_INPUT_ELEMENT_DESC layout_desc[ 4 ] = { };
        
        layout_desc[ 0 ].SemanticName = "POSITION";
        layout_desc[ 0 ].Format = DXGI_FORMAT_R32G32_FLOAT;
        layout_desc[ 0 ].AlignedByteOffset = 0;
        layout_desc[ 0 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        
        layout_desc[ 1 ].SemanticName = "UVW";
        layout_desc[ 1 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        layout_desc[ 1 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layout_desc[ 1 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        
        layout_desc[ 2 ].SemanticName = "COLOR";
        layout_desc[ 2 ].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        layout_desc[ 2 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layout_desc[ 2 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        
        layout_desc[ 3 ].SemanticName = "THICKNESS";
        layout_desc[ 3 ].Format = DXGI_FORMAT_R32_FLOAT;
        layout_desc[ 3 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        layout_desc[ 3 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        
        hr = g_dx11.device->CreateInputLayout( layout_desc, ArrayCount( layout_desc ), vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), &input_layout );
        
        if ( FAILED( hr ) ) {
            log_os( "Failed to create input layout.\n" );
            break;
        }
        
        
        hr = D3DCompile( pixel, pixel_length, 0, 0, 0, "main", "ps_5_0", 0, 0, &ps_blob, &ps_error_blob );
        
        if ( FAILED( hr ) ) {
            log_os( "Failed to compile pixel shader.\n" );
            
            if ( ps_error_blob ) {
                u8* error_message = ( u8* ) ps_error_blob->GetBufferPointer( );
                u32 length = ( u32 ) ps_error_blob->GetBufferSize( );
                log_os( "pixel shader error:\n%.*s\n", length, error_message );
            }
            
            break;
        }
        
        hr = g_dx11.device->CreatePixelShader( ps_blob->GetBufferPointer( ), ps_blob->GetBufferSize( ), 0, &pixel_shader );
        
        if ( FAILED( hr ) ) {
            log_os( "Failed to create a pixel shader.\n" );
            break;
        }
        
        result.vertex = vertex_shader;
        result.layout = input_layout;
        result.pixel = pixel_shader;
        result.valid = true;
        
    } while ( 0 );
    
    if ( vs_blob ) {
        vs_blob->Release( );
        vs_blob = 0;
    }
    
    if ( vs_error_blob ) {
        vs_error_blob->Release( );
        vs_error_blob = 0;
    }
    
    if ( ps_blob ) {
        ps_blob->Release( );
        ps_blob = 0;
    }
    
    if ( ps_error_blob ) {
        ps_error_blob->Release( );
        ps_error_blob = 0;
    }
    
    if ( !result.valid ) {
        
        if ( vertex_shader ) {
            vertex_shader->Release( );
            vertex_shader = 0;
        }
        
        if ( input_layout ) {
            input_layout->Release( );
            input_layout = 0;
        }
        
        if ( pixel_shader ) {
            pixel_shader->Release( );
            pixel_shader = 0;
        }
        
        os_popup_error( "Error", "Shader compilation failed." );
    }
    
    return result;
}

internal void
gl_render(Render_Target *t){
    Font_Set *font_set = (Font_Set*)t->font_set;
    
    local_persist b32 first_call = true;
    
    if (first_call){
        
        // NOTE(simon, 28/02/24): Most of the code here has been moved in win32_gl_create_window
        // because if that code fails we should exit the application directly.
        first_call = false;
        
        u32 stride = sizeof( Render_Vertex );
        u32 offset = 0;
        
        g_dx11.context->IASetVertexBuffers( 0, 1, &g_dx11.vertex_buffer, &stride, &offset );
        g_dx11.context->IASetInputLayout( g_dx11.gpu_program.layout );
        g_dx11.context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        
        g_dx11.context->VSSetShader( g_dx11.gpu_program.vertex, 0, 0 );
        g_dx11.context->VSSetConstantBuffers( 0, 1, &g_dx11.constants_buffer );
        
        g_dx11.context->PSSetShader( g_dx11.gpu_program.pixel, 0, 0 );
        g_dx11.context->PSSetSamplers( 0, 1, &g_dx11.sampler );
        
        {
            t->fallback_texture_id = gl__get_texture(V3i32(2, 2, 1), TextureKind_Mono);
            u8 white_block[] = { 0xFF, 0xFF, 0xFF, 0xFF, };
            // NOTE(simon, 28/02/24): Passing the fallback texture, because we can't rely on the
            // fact that gl__get_texture has bound the fallback texture.
            gl__fill_texture(TextureKind_Mono, t->fallback_texture_id, V3i32(0, 0, 0), V3i32(2, 2, 1), white_block);
        }
    }
    
    // NOTE(simon, 28/02/24): OMSetRenderTargets needs to be set each frame when using a FLIP swap
    // chain.
    g_dx11.context->OMSetRenderTargets( 1, &g_dx11.render_target_view, 0 );
    
    i32 width = t->width;
    i32 height = t->height;
    
    // NOTE(simon, 28/02/24): Viewport (0, 0) is top left in directx. Important for viewport and
    // scissor calls.
    
    D3D11_VIEWPORT viewport = {
        0, // TopLeftX
        0, // TopLeftY
        ( float ) width, // Width
        ( float ) height, // Height
        0, // MinDepth
        1// MaxDepth
    };
    
    g_dx11.context->RSSetViewports( 1, &viewport );
    
    D3D11_RECT scissor = {
        0, // left
        0, // top
        width, // right
        height // bottom
    };
    
    g_dx11.context->RSSetScissorRects( 1, &scissor );
    
    float magenta[ 4 ] = { 1.0f, 0.0f, 1.0f, 1.0f };
    g_dx11.context->ClearRenderTargetView( g_dx11.render_target_view, magenta );
    
    // NOTE(simon, 28/02/24): The constants (uniforms) were set in the render loop in the OpenGL
    // version. But since they don't vary between draw calls I moved the code before the render
    // loop.
    D3D11_MAPPED_SUBRESOURCE constants_map = { };
    HRESULT hr = g_dx11.context->Map( ( ID3D11Resource* ) g_dx11.constants_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constants_map );
    
    // NOTE(simon, 28/02/24): The layout of the constants buffer was a bit confusing. This link
    // explains a little about how data is laid out:
    // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
    // The article doesn't explain anything about matrices. What I found out while making this work
    // is that each row or column (depending on if we use column or row major matrices) of a matrix
    // needs to start on a new 16 bytes vector. For a 2 by 2 matrix, this means that there are two
    // register elements at the end of the first vector that aren't used.
    // Another thing is that the second vector only needs the first two elements for the matrix,
    // so the two elements we want to put next can be in the same vector.
    
    // NOTE(simon, 28/02/24): The code here could be shorter, but I prefer to make it clear what's
    // happening.
    f32 view_m[ 4 ] = {
        2.0f / width, 0,
        0, -2.0f / height
    };
    f32 view_t[ 2 ] = { width / 2.0f, height / 2.0f };
    
    f32* vector_1 = ( f32* ) constants_map.pData;
    f32* vector_2 = vector_1 + 4;
    
    vector_1[ 0 ] = view_m[ 0 ];
    vector_1[ 1 ] = view_m[ 1 ];
    vector_1[ 2 ] = 0; // Padding
    vector_1[ 3 ] = 0; // Padding
    
    vector_2[ 0 ] = view_m[ 2 ];
    vector_2[ 1 ] = view_m[ 3 ];
    vector_2[ 2 ] = view_t[ 0 ];
    vector_2[ 3 ] = view_t[ 1 ];
    
    g_dx11.context->Unmap( ( ID3D11Resource* ) g_dx11.constants_buffer, 0 );
    
    gl__bind_texture( t, 0 );
    
    for (Render_Free_Texture *free_texture = t->free_texture_first;
         free_texture != 0;
         free_texture = free_texture->next){
        
        gl__free_texture( free_texture->tex_id );
    }
    
    t->free_texture_first = 0;
    t->free_texture_last = 0;
    
    D3D11_BUFFER_DESC vertex_buffer_desc = { };
    g_dx11.vertex_buffer->GetDesc( &vertex_buffer_desc );
    
    for (Render_Group *group = t->group_first;
         group != 0;
         group = group->next){
        Rect_i32 box = Ri32(group->clip_box);
        
        D3D11_RECT group_scissor = { };
        group_scissor.left = box.x0;
        group_scissor.right = box.x1;
        group_scissor.top = box.y0;
        group_scissor.bottom = box.y1;
        
        g_dx11.context->RSSetScissorRects( 1, &group_scissor );
        
        i32 vertex_count = group->vertex_list.vertex_count;
        if (vertex_count > 0){
            Face *face = font_set_face_from_id(font_set, group->face_id);
            if (face != 0){
                gl__bind_texture(t, face->texture);
            }
            else{
                gl__bind_any_texture(t);
            }
            
            // NOTE(simon, 29/03/24): 4coder doesn't appear to clip character outside the screen
            // horizontally. Even with line wrapping enabled, you can have cases where the line
            // won't wrap, for example "{0,0,0,0,...}" with a lot of zero and no space will not
            // wrap. The consequence of that is that we might send a lot of vertex data that's
            // offscreen and the assumption about the vertex buffer size I made, can be wrong.
            // So in this loop we release the previous vertex and create a new one when necessary.
            u32 size_required = vertex_count * sizeof( Render_Vertex );
            
            if ( size_required > vertex_buffer_desc.ByteWidth ) {
                
                u32 new_size = vertex_buffer_desc.ByteWidth * 2;
                
                while ( new_size < size_required ) {
                    new_size *= 2;
                }
                
                // NOTE(simon, 29/03/24): Create a new buffer and only release the previous one if
                // the creation succeeded. If the creation fails, we skip this vertex group, which
                // means the user will see an empty panel, but at least we won't stop rendering.
                D3D11_BUFFER_DESC new_vertex_buffer_desc = vertex_buffer_desc;
                new_vertex_buffer_desc.ByteWidth = new_size;
                ID3D11Buffer* new_vertex_buffer = 0;
                hr = g_dx11.device->CreateBuffer( &new_vertex_buffer_desc, 0, &new_vertex_buffer );
                
                if ( FAILED( hr ) ) {
                    continue;
                }
                
                g_dx11.vertex_buffer->Release( );
                g_dx11.vertex_buffer = new_vertex_buffer;
                vertex_buffer_desc.ByteWidth = new_size;
                
                u32 stride = sizeof( Render_Vertex );
                u32 offset = 0;
                g_dx11.context->IASetVertexBuffers( 0, 1, &g_dx11.vertex_buffer, &stride, &offset );
            }
            
            // NOTE(simon, 28/02/24): We fill the buffer, draw what we filled and then do the next
            // group, which allows to always start drawing from vertex 0. Alternatively we could
            // do a pass to fill the vertex buffer completly so we only map the vertex buffer once,
            // and then a second pass that just execute the draw calls. It doesn't seems necessary
            // since we have less than 10 draw call.
            
            D3D11_MAPPED_SUBRESOURCE vertex_map = { };
            hr = g_dx11.context->Map( ( ID3D11Resource* ) g_dx11.vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertex_map );
            
            if ( FAILED( hr ) ) {
                // NOTE(simon, 28/02/24): It's improbable that Map will fail, but if it does we
                // just stop rendering, and we'll try on the next frame. We could just skip the
                // group and try with the next (using 'continue' instead of 'break'), but Map would
                // probably fail again. Waiting for the next frame "might" work. I don't really
                // know. We could also just exit the application assuming we won't be able to
                // render anything.
                break;
            }
            
            u8* bytes = ( u8* ) vertex_map.pData;
            
            for (Render_Vertex_Array_Node *node = group->vertex_list.first;
                 node != 0;
                 node = node->next){
                
                i32 size = node->vertex_count*sizeof(*node->vertices);
                memcpy( bytes, node->vertices, size );
                bytes += size;
            }
            
            g_dx11.context->Unmap( ( ID3D11Resource* ) g_dx11.vertex_buffer, 0 );
            
            g_dx11.context->Draw( vertex_count, 0 );
        }
    }
}