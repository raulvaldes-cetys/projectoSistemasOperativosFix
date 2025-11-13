#include <iostream>
#include <chrono>
#include <cstdint>

using namespace std;

int randomizeNumberInAssembly(int min, int max) {
    // current seed
    static uint32_t state = 0;
    // initialized flag
    static bool initialized = false;
    
    // print info only once
    if (!initialized) {
        cout << "Assembly language used: ARM64\n";
        cout << "Randomization algorithm: XORshift with time + CPU cycle seed\n";
    }

    cout << "Random number: ";

    uint32_t time_low, cycles_low;
    uint64_t time_ns, cycles;

    // get current time in nanoseconds
    time_ns = chrono::duration_cast<chrono::nanoseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
    time_low = (uint32_t)time_ns;

    // read cpu cycle counter
    __asm__ volatile (
        "mrs %0, cntvct_el0"
        : "=r" (cycles)
    );
    cycles_low = (uint32_t)cycles;

    // create seed by XORing time and cycles
    uint32_t seed;
    __asm__ volatile (
        "eor %0, %1, %2"    // seed = time XOR cycles
        : "=r" (seed)
        : "r" (time_low), "r" (cycles_low)
    );

    // initialize state with seed on first call
    if (!initialized) {
        state = seed;
        if (state == 0) state = 1;
        initialized = true;
    }

     // xorshift
    __asm__ volatile (
        "eor %0, %0, %0, lsl #13\n\t"    // state = state XOR (state << 13)
        "eor %0, %0, %0, lsr #17\n\t"    // state = state XOR (state >> 17)
        "eor %0, %0, %0, lsl #5"         // state = state XOR (state << 5)
        : "+r" (state)
    );

    // limit to range [min, max] using modulo
    uint32_t range = (uint32_t)(max - min + 1);
    uint32_t remainder;
    uint32_t state_val = state;  // Copy to avoid modifying original
    
    // Calculate: remainder = state % range, then add min
    __asm__ volatile (
        "udiv %0, %1, %2\n\t"            // temp = state_val / range (stored in remainder)
        "msub %0, %0, %2, %1\n\t"        // remainder = state_val - (temp * range)
        "add %0, %0, %3"                 // remainder = remainder + min
        : "=&r" (remainder)
        : "r" (state_val), "r" (range), "r" ((uint32_t)min)
        : "cc"
    );

    return (int)remainder;
}

int main(int argc, const char * argv[]) {
    cout << "----------------------------------------\n";
    cout << "Welcome to the assembly randomizer!\n";
    cout << "----------------------------------------\n";
    
    bool quit = false;
    while (!quit) {
        cout << "Main menu:\n";
        cout << "Random number (r/R)\n";
        cout << "Quit (q/Q)\n";
        cout << "What would you like to do?\n";
        char choice;
        cin >> choice;
        switch (choice) {
            case 'r':
            case 'R':
                int min, max;
                cout << "Enter the minimum: ";
                cin >> min;
                cout << "Enter the maximum: ";
                cin >> max;
                cout << randomizeNumberInAssembly(min, max) << endl;
                break;
            case 'q':
            case 'Q':
                quit = true;
                break;
            default:
                cout << "Invalid option, try again\n";
                break;
        }
    }
    return EXIT_SUCCESS;
}