#include <adios2.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

std::vector<float> generate_3d_wave_data(int t, int ny, int nx, float n = -1.0f, float m = 1.0f)
{
    if (m < n) std::swap(n, m);

    const size_t total = static_cast<size_t>(t) * ny * nx;
    std::vector<float> data(total);

    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> noise_dist(-0.5f, 0.5f);

    const float cx = nx / 2.0f;
    const float cy = ny / 2.0f;
    const float omega = 0.2f;

    for (int time = 0; time < t; ++time)
    {
        float time_scale = 0.5f + ((rand() % 1000) / 1000.0f) + std::sin((rand() % 1000) / 1000.0f);
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

                // normalize to [n, m]
                val = ((val + 1.0f) / 2.0f) * (m - n) + n;
                data[idx] = val;
            }
        }
    }
    return data;
}

// simple dummy field (scaled version + noise)
std::vector<float> generate_dummy_field(const std::vector<float>& base, float scale)
{
    static thread_local std::mt19937 rng(std::random_device{}());
    std::normal_distribution<float> noise(0.0f, 0.05f);

    std::vector<float> out(base.size());
    for (size_t i = 0; i < base.size(); ++i)
        out[i] = scale * base[i] + noise(rng);
    return out;
}

int main()
{
    try
    {
        adios2::ADIOS adios;
        adios2::IO io = adios.DeclareIO("TestIO");
        io.SetEngine("BP5");

        adios2::Engine writer = io.Open("test_caesar_data.bp", adios2::Mode::Write);

        const int t_3d = 20;
        const int ny_3d = 256;
        const int nx_3d = 256;

        std::cout << "Writing 3 variables of size [" << t_3d << ", " << ny_3d << ", " << nx_3d << "]" << std::endl;

        // define three variables (same shape)
        auto var_wave = io.DefineVariable<float>(
            "wave_3d",
            { (size_t)t_3d, (size_t)ny_3d, (size_t)nx_3d },
            { 0, 0, 0 },
            { (size_t)t_3d, (size_t)ny_3d, (size_t)nx_3d });

        auto var_dummy1 = io.DefineVariable<float>(
            "dummy_3d_1",
            { (size_t)t_3d, (size_t)ny_3d, (size_t)nx_3d },
            { 0, 0, 0 },
            { (size_t)t_3d, (size_t)ny_3d, (size_t)nx_3d });

        auto var_dummy2 = io.DefineVariable<float>(
            "dummy_3d_2",
            { (size_t)t_3d, (size_t)ny_3d, (size_t)nx_3d },
            { 0, 0, 0 },
            { (size_t)t_3d, (size_t)ny_3d, (size_t)nx_3d });

        for (int step = 0; step < 3; ++step)
        {
            auto wave_data = generate_3d_wave_data(t_3d, ny_3d, nx_3d);
            auto dummy1 = generate_dummy_field(wave_data, 0.8f);
            auto dummy2 = generate_dummy_field(wave_data, 1.2f);

            writer.BeginStep();
            writer.Put(var_wave, wave_data.data());
            writer.Put(var_dummy1, dummy1.data());
            writer.Put(var_dummy2, dummy2.data());
            writer.EndStep();

            std::cout << "  Written timestep " << step + 1 << "/20" << std::endl;
        }

        writer.Close();
        std::cout << "Done. File: test_caesar_data.bp" << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

