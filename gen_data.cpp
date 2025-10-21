#include <adios2.h>
#include <iostream>
#include <vector>
#include <cmath>

// Generate 3D data: A wave propagating through a 3D medium over time
// Dimensions: [time, y, x] = [20, 256, 256]
std::vector<float> generate_3d_wave_data(int t , int ny , int nx) {
    std::vector<float> data(t * ny * nx);

    const float cx = nx / 2.0f;
    const float cy = ny / 2.0f;
    const float omega = 0.3f; // Angular frequency

    for (int time = 0; time < t; time++) {
        for (int y = 0; y < ny; y++) {
            for (int x = 0; x < nx; x++) {
                int idx = time * ny * nx + y * nx + x;

                // Distance from center
                float dx = x - cx;
                float dy = y - cy;
                float r = std::sqrt(dx * dx + dy * dy);

                // Propagating wave equation: A * sin(k*r - omega*t)
                float phase = 0.1f * r - omega * time;
                float amplitude = 1.0f / (1.0f + 0.01f * r); // Decay with distance

                data[idx] = amplitude * std::sin(phase);
            }
        }
    }

    return data;
}

// Generate 4D data: Vortex flow field in 3D space at a single timestep
// Dimensions: [vx, vy, vz, spatial_dim] = [3, 256, 256, 64]
// This represents velocity components in a 3D volume
std::vector<float> generate_4d_vortex_data(int ncomp , int nz , int ny , int nx) {
    std::vector<float> data(ncomp * nz * ny * nx);

    const float cx = nx / 2.0f;
    const float cy = ny / 2.0f;
    const float cz = nz / 2.0f;
    const float vortex_strength = 0.5f;

    for (int comp = 0; comp < ncomp; comp++) {
        for (int z = 0; z < nz; z++) {
            for (int y = 0; y < ny; y++) {
                for (int x = 0; x < nx; x++) {
                    int idx = comp * nz * ny * nx + z * ny * nx + y * nx + x;

                    float dx = (x - cx) / nx;
                    float dy = (y - cy) / ny;
                    float dz = (z - cz) / nz;
                    float r = std::sqrt(dx * dx + dy * dy + dz * dz);

                    // Rotating vortex field
                    if (comp == 0) { // vx component
                        data[idx] = -vortex_strength * dy * std::exp(-r * r);
                    }
                    else if (comp == 1) { // vy component
                        data[idx] = vortex_strength * dx * std::exp(-r * r);
                    }
                    else { // vz component
                        data[idx] = vortex_strength * dz * std::exp(-r * r) * 0.3f;
                    }
                }
            }
        }
    }

    return data;
}

int main() {
    try {
        adios2::ADIOS adios;
        adios2::IO io = adios.DeclareIO("TestIO");
        io.SetEngine("BP4");

        // Open file for writing
        adios2::Engine writer = io.Open("test_caesar_data.bp" , adios2::Mode::Write);

        std::cout << "Generating test data..." << std::endl;

        // ========== 3D Data: Wave Propagation ==========
        std::cout << "\n=== 3D Data: Wave Propagation ===" << std::endl;
        const int t_3d = 20;   // Time steps (>= 8 for CAESAR)
        const int ny_3d = 256;
        const int nx_3d = 256;

        std::cout << "Dimensions: [" << t_3d << ", " << ny_3d << ", " << nx_3d << "]" << std::endl;
        std::cout << "Generating wave data..." << std::endl;

        auto var_wave = io.DefineVariable<float>(
            "wave_3d" ,
            { static_cast<size_t>(t_3d), static_cast<size_t>(ny_3d), static_cast<size_t>(nx_3d) } ,
            { 0, 0, 0 } ,
            { static_cast<size_t>(t_3d), static_cast<size_t>(ny_3d), static_cast<size_t>(nx_3d) }
        );

        // Write across timesteps
        for (int step = 0; step < 20; step++) {
            auto wave_data = generate_3d_wave_data(t_3d , ny_3d , nx_3d);

            writer.BeginStep();
            writer.Put(var_wave , wave_data.data());
            writer.EndStep();

            std::cout << "  ✓ Written timestep " << (step + 1) << "/20" << std::endl;
        }

        std::cout << "✓ Written 3D wave data" << std::endl;

        // ========== 4D Data: Vortex Flow Field ==========
        std::cout << "\n=== 4D Data: Vortex Flow Field ===" << std::endl;
        const int ncomp = 8;   // Components (>= 8 for CAESAR)
        const int nz_4d = 64;
        const int ny_4d = 256;
        const int nx_4d = 256;

        std::cout << "Dimensions: [" << ncomp << ", " << nz_4d << ", " << ny_4d << ", " << nx_4d << "]" << std::endl;
        std::cout << "Generating vortex field data..." << std::endl;

        auto var_vortex = io.DefineVariable<float>(
            "vortex_4d" ,
            { static_cast<size_t>(ncomp), static_cast<size_t>(nz_4d),
             static_cast<size_t>(ny_4d), static_cast<size_t>(nx_4d) } ,
            { 0, 0, 0, 0 } ,
            { static_cast<size_t>(ncomp), static_cast<size_t>(nz_4d),
             static_cast<size_t>(ny_4d), static_cast<size_t>(nx_4d) }
        );

        // Write across timesteps
        for (int step = 0; step < 20; step++) {
            auto vortex_data = generate_4d_vortex_data(ncomp , nz_4d , ny_4d , nx_4d);

            writer.BeginStep();
            writer.Put(var_vortex , vortex_data.data());
            writer.EndStep();

            std::cout << "  ✓ Written timestep " << (step + 1) << "/20" << std::endl;
        }

        std::cout << "✓ Written 4D vortex data" << std::endl;

        writer.Close();

        std::cout << "\n========================================" << std::endl;
        std::cout << "Successfully created test_caesar_data.bp" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "\nData description:" << std::endl;
        std::cout << "  - wave_3d: 3D wave propagation [20, 256, 256] across 20 timesteps" << std::endl;
        std::cout << "  - vortex_4d: 4D vortex flow field [8, 64, 256, 256] across 20 timesteps" << std::endl;

    }
    catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}