#include <adios2.h>
#include <iostream>
#include <vector>
#include <cmath>

#include <vector>
#include <cmath>
#include <random>
#include <algorithm> // for std::swap


std::vector<float> generate_3d_wave_data(int t , int ny , int nx , float n = -1.0f , float m = 1.0f)
{
    if (m < n) std::swap(n , m);

    const size_t total = static_cast<size_t>(t) * ny * nx;
    std::vector<float> data(total);

    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> noise_dist(-0.5f , 0.5f);

    const float cx = nx / 2.0f;
    const float cy = ny / 2.0f;
    const float omega = 0.2f;

    for (int time = 0; time < t; ++time)
    {

        float time_scale = 0.5f + ((rand() % 1000) / 1000.0f) + std::sin(rand() % 1000 / 1000.0f);
        for (int y = 0; y < ny; ++y)
        {
            for (int x = 0; x < nx; ++x)
            {
                size_t idx = static_cast<size_t>(time) * ny * nx + y * nx + x;

                float dx = x - cx;
                float dy = y - cy;
                float r = std::sqrt(dx * dx + dy * dy);

                float phase1 = 0.1f * r - omega * time;
                float phase2 = 0.15f * r + 0.3f * time;
                float amplitude = 1.0f / (1.0f + 0.01f * r);

                float wave_val = amplitude * (std::sin(phase1) + 0.5f * std::sin(phase2));

                float val = wave_val * time_scale + noise_dist(rng);

                val = ((val + 1.0f) / 2.0f) * (m - n) + n;

                data[idx] = val;
            }
        }
    }

    return data;
}

int main()
{
    try
    {
        adios2::ADIOS adios;
        adios2::IO io = adios.DeclareIO("TestIO");
        io.SetEngine("BP5");

        adios2::Engine writer = io.Open("test_caesar_data.bp" , adios2::Mode::Write);

        std::cout << "Generating test data..." << std::endl;


        std::cout << "\n=== 3D Data: Wave Propagation ===" << std::endl;
        const int t_3d = 20;
        const int ny_3d = 256;
        const int nx_3d = 256;

        std::cout << "Dimensions: [" << t_3d << ", " << ny_3d << ", " << nx_3d << "]" << std::endl;
        std::cout << "Generating wave data..." << std::endl;

        auto var_wave = io.DefineVariable<float>(
            "wave_3d" ,
            { static_cast<size_t>(t_3d), static_cast<size_t>(ny_3d), static_cast<size_t>(nx_3d) } ,
            { 0, 0, 0 } ,
            { static_cast<size_t>(t_3d), static_cast<size_t>(ny_3d), static_cast<size_t>(nx_3d) });


        for (int step = 0; step < 20; step++)
        {
            auto wave_data = generate_3d_wave_data(t_3d , ny_3d , nx_3d);

            writer.BeginStep();
            writer.Put(var_wave , wave_data.data());
            writer.EndStep();

            std::cout << " Written timestep " << (step + 1) << "/20" << std::endl;
        }

        std::cout << " Written 3D wave data" << std::endl;

        writer.Close();

        std::cout << "\n========================================" << std::endl;
        std::cout << "Successfully created test_caesar_data.bp" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "\nData description:" << std::endl;
        std::cout << "  - wave_3d: 3D wave propagation [20, 256, 256] across 20 timesteps" << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
