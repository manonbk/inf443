
#include "models_textures.hpp"

using namespace cgp;

mesh torus_with_texture()
{
    float a = 1.7f;
    float b = 0.6f;

    // Number of samples of the terrain is N x N
    int N = 50;

    mesh torus; // temporary terrain storage (CPU only)
    torus.position.resize(N*N);
    torus.uv.resize(N*N);

    // Fill terrain geometry
    for(int ku=0; ku<N; ++ku)
    {
        for(int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            float u = ku/(N-1.0f);
            float v = kv/(N-1.0f);

            // Compute the local surface function

            vec3 p = {
					(a + b*std::cos(2* Pi *u))*std::cos(2* Pi *v),
					(a + b*std::cos(2* Pi *u))*std::sin(2* Pi *v),
					b*std::sin(2* Pi *u)};
            

            // Store vertex coordinates
            torus.position[kv+N*ku] = p;
            
            torus.uv[kv+N*ku] = {20*v, 5*u};
        }
    }


    // Generate triangle organization
    for(int ku=0; ku<N-1; ++ku)
    {
        for(int kv=0; kv<N-1; ++kv)
        {
            unsigned int idx = kv + N*ku;

            uint3 triangle_1 = {idx, idx+1, idx+1+N };
            uint3 triangle_2 = {idx, idx+1+N, idx+N };

            torus.connectivity.push_back(triangle_1);
            torus.connectivity.push_back(triangle_2);
        }
    }

    torus.fill_empty_field();
    return torus;
}
