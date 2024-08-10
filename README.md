# GravelerC

This repository contains an implementation of the ShoddyCast Graveler calculation in C with multithreading and maximum single machine performance.

I've written this sample in my spare time and it's not part of my work.

## Implementation Notes

I've chosed because C++ it is one of the most performant langues because it compiles into the machine code for AMD64 and executes directly on the CPU. These langues does not have any overhead for garbage collection or JIT compilation.

There are many ways to write this code "correctly" within C++. Alternative implementations could use exceptions or more STL which could make the code more compact. In some places in the code, I've gone beyond what's strictly needed to demonstrate error handling in C. Alternative versions could crash on errors and remove the need for any cleanup.

I've chosen this style because I'm a Windows driver developer by trade and the C-style with non-crash error handling is what I practice in my work.

## Running this yourself

- Install Visual Studio 2024, you should be able to use the free Community Edition.
- Make sure to include Visual C++ features during installation.
- Open the `GravelerC\GravelerC.sln` solution.
- Use the "Start Debugging" to compile and run.
- For maximum performance:
    - Set to "Relase" instead of debug.
    - Start a new command prompt.
    - Start the application yourself from the `x64\Release` folder in the command prompt.

## Initial Results

On a modern desktop compuiter, I estimate this will complete the one billion simulations in **about an hour**.

Running this simulation will turn your PC into a space heater! It's currently the peak of summer here, and I'd prefer not to leave this running for the full hour to confirm the results.