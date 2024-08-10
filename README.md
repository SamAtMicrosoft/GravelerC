# GravelerC

This repository contains an implementation of the ShoddyCast Graveler calculation in C with multithreading and maximum single machine performance.

I've written this sample in my spare time and it's not part of my work.

## Initial Results

I was able to get 99 "paralysis" instances in a billion simulations of 231 rolls. It took under 3 minutes.

```
PS C:\Users\sakarim\source\repos\GravelerC\GravelerC\x64\Release> .\GravelerC.exe
Running 1000000000 simulations
Using 8 threads
Starting at time Sat Aug 10 15:30:49 2024
At time Sat Aug 10 15:30:59 2024
901109000 iterations remain.
At time Sat Aug 10 15:31:09 2024
802173000 iterations remain.
At time Sat Aug 10 15:31:19 2024
707916000 iterations remain.
At time Sat Aug 10 15:31:29 2024
606273000 iterations remain.
At time Sat Aug 10 15:31:39 2024
503651000 iterations remain.
At time Sat Aug 10 15:31:49 2024
402145000 iterations remain.
At time Sat Aug 10 15:31:59 2024
302210000 iterations remain.
At time Sat Aug 10 15:32:09 2024
200020000 iterations remain.
At time Sat Aug 10 15:32:19 2024
97883000 iterations remain.
The final result is 99
```

Running this simulation will turn your PC into a space heater! It's currently the peak of Summer here, and I'd prefer not to leave this running for the full hour to confirm the results.

## Implementation Notes

I've chose because C++ it is one of the most performant languages. It compiles into the machine code for AMD64 and executes directly on the CPU.

There are many ways to write this code "correctly" within C++. Alternative implementations could use exceptions or more STL which could make the code more compact. In some places in the code, I've gone beyond what's strictly needed to demonstrate error handling in C. Alternative versions could crash on errors and remove the need for any cleanup.

I've chosen this style because I'm a Windows kernel driver developer by trade and the non-crash error handling is what I practice in my work.

I chosen to leave your computer responsive while the code runs. This means it'll run slower if you're checking email or watching YouTube in the background.

##  Comparing this to Pokemon

The random number generation in Pokemon is completely different than what I have here. I've chosen to use a secure random "twister" algorithm which is much closer to "true random" than what Pokemon uses.

Pokemon's random number generation works more like reading random numbers out of a printed book. The random numbers are all known in advance. Every time Pokemon needs a random number, it gets the next one. This means a "true" solution to the problem would involve dumping this list of random numbers and seeing if any subsequence of the random numbers gives what is needed.

## Running this yourself

- Install Visual Studio 2024, you should be able to use the free Community Edition.
- Make sure to include Visual C++ features during installation.
- Open the `GravelerC\GravelerC.sln` solution.
- Use the "Start Debugging" to compile and run.
- For maximum performance:
    - Set to "Relase" instead of debug.
    - Start a new command prompt.
    - Start the application yourself from the `x64\Release` folder in the command prompt.

If you want to learn more C++, give this a try for yourself!

- Use breakpoints to step through the code.
- Try changing the constants. What happens if you use more CPUs than the system has?