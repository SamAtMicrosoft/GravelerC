//
// GravelerC.cpp
// 
// Created by SamAtMicrosoft 2024.
// 
// An implementation of the ShoddyCast challenge for Pikasprey's Graveler
// Softlock.
// 
// MIT open source license: use for any purpose.
//

#include <Windows.h>
#include <time.h>
#include <iostream>
#include <cassert>
#include <random>

//
// This global represents the remaining iterations or a "work queue". Threads
// "take work" from this queue by decrementing the number of iterations.
//

volatile LONG64 RemainingIterations;

//
// This global represents the "batch size" for each worker thread. Although
// it is not marked as "const", it must be treated as "read only" by the worker
// thread.
//

LONG64 BatchSize;

//
// The problem parameters say there are 231 rolls.
//

const ULONG Rolls = 231;

struct CALCUATION_WORKER_PARAMS {
    LONG MaxOnesObserved;
};

DWORD
GetPhysicalProcessorCount (
    ULONG &ProcessorCount
    );

DWORD
CalculationWorker (
    void* ThreadParameter
    );

int main()
{
    DWORD Error;
    ULONG ProcessorCoreCount = 0;
    ULONG ThreadIndex;
    HANDLE* ThreadHandles;
    CALCUATION_WORKER_PARAMS* ThreadParams;
    BOOLEAN ThreadStartError;
    LONG Result;

    //
    // Used to log progress.
    //

    LONG64 IterationsToLog;
    std::time_t CurrentTime =std::time(nullptr);
    char TimeBuffer[26];
    int TimeFormatResult;

    ULONG LoggingIntervalMs = 10000;


    //
    // Execution parameters including the total number of simulations and
    // the size of the batches run by the worker threads.
    //

    LONG64 TotalIterations = 1000 * 1000 * 1000;
    BatchSize = 1000;

    //
    // Set the starting number of iterations. This is done using an interlocked
    // operation because this memory can be set from multiple threads.
    //

    InterlockedExchange64(&RemainingIterations, TotalIterations);
    std::cout << "Running " << TotalIterations << " simulations\n";

    //
    // Initialize buffers in order to know if they have been successfully
    // allocated.
    //

    ThreadHandles = NULL;
    ThreadParams = NULL;

    //
    // Retrieve the number of pysical processors on this system. Using physical
    // cores is better for threads which never yield, but it also allows for
    // other processes on the system to remain responsive via the virtual cores.
    //

    Error = GetPhysicalProcessorCount(ProcessorCoreCount);

    if (Error != ERROR_SUCCESS) {
        goto Exit;
    }

    std::cout << "Using " << ProcessorCoreCount << " threads \n";

    CurrentTime = std::time(nullptr);
    TimeFormatResult = ctime_s(TimeBuffer,
                               sizeof(TimeBuffer),
                               &CurrentTime);

    if (TimeFormatResult != 0) {
        std::cout << "Failed to format date time " << TimeFormatResult << "\n";
        Error = TimeFormatResult;
        goto Exit;
    }

    std::cout << "Starting at time " << TimeBuffer;

    //
    // Allocate space to store the thread handles.
    //

    ThreadHandles = (HANDLE*)
        malloc(ProcessorCoreCount * sizeof(*ThreadHandles));

    if (!ThreadHandles) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    RtlZeroMemory(ThreadHandles, ProcessorCoreCount * sizeof(*ThreadHandles));

    //
    // Allocate space for the thread parameters.
    //

    ThreadParams = (CALCUATION_WORKER_PARAMS*)
        malloc(ProcessorCoreCount * sizeof(*ThreadParams));

    if (!ThreadParams) {
        goto Exit;
    }

    RtlZeroMemory(ThreadParams, ProcessorCoreCount * sizeof(*ThreadParams));

    //
    // Avoid an errand "use of uninitialized variable" which the compiler
    // thinks can happen if ProcessorCoreCount is somehow zero.
    //

    ThreadStartError = FALSE;

    //
    // Start the worker threads.
    //

    for (ThreadIndex = 0; ThreadIndex < ProcessorCoreCount; ThreadIndex += 1) {
        ThreadHandles[ThreadIndex] = CreateThread(NULL,
                                                  0,
                                                  CalculationWorker,
                                                  &ThreadParams[ThreadIndex],
                                                  0,
                                                  NULL);

        if(!ThreadHandles[ThreadIndex]) {

            //
            // Thread start has failed. This is challenging because one or more
            // worker threads may have already started. Demonstrate safely
            // handling this condition.
            //

            //
            // Cancel work by setting the RemainingIterations to zero.
            //

            InterlockedExchange64(&RemainingIterations, 0);
            ThreadStartError = TRUE;
            Error = GetLastError();

            std::cout << "Terminating work due to error " << Error <<
                         " while starting threads.";

            //
            // Do not attempt to start other threads.
            //

            break;
        }

        ThreadStartError = FALSE;
    }

    //
    // Wait for each thread to exit or report status.
    //

    ThreadIndex = 0;

    while (TRUE) {

        if (ThreadIndex == ProcessorCoreCount) {

            //
            // All threads have exited.
            //

            break;
        }

        if (!ThreadHandles[ThreadIndex]) {

            //
            // This thread was not started.
            //

            ThreadIndex += 1;
            continue;
        }

        //
        // This thread is running. Wait for it to exit or for the logging
        // interval to elapse.
        //

        Error = WaitForSingleObject(ThreadHandles[ThreadIndex], 
                                    LoggingIntervalMs);

        if (Error == STATUS_WAIT_0) {

            //
            // The worker thread is finished.
            //

            ThreadIndex += 1;
            continue;
        }

        if (Error == WAIT_TIMEOUT) {

            //
            // Time to log a status update.
            //

            IterationsToLog = InterlockedExchangeAdd64(&RemainingIterations, 0);

            CurrentTime = std::time(nullptr);
            TimeFormatResult = ctime_s(TimeBuffer, 
                                       sizeof(TimeBuffer), 
                                       &CurrentTime);

            if (TimeFormatResult != 0) {
                std::cout << "Failed to get time due to error " << 
                              TimeFormatResult << "\n";

                std::cout << IterationsToLog << " iterations remain.\n";
                continue;
            }
            std::cout << "At time " << TimeBuffer << IterationsToLog
                      << " iterations remain.\n";

            //
            // Continue to wait on this thread.
            //

            continue;
        }

        //
        // All legal returns of WaitForSingleObject have been checked and this
        // code should be impossilbe to reach.
        //

        assert(!"Invald result while waiting for thread exit");
    }

    if (ThreadStartError) {

        //
        // There are no results to report due to an earlier failure to start
        // a worker thread.
        //

        goto Exit;
    }

    //
    // All work is completed. Get the results.
    //

    Error = ERROR_SUCCESS;
    Result = 0;

    for (ThreadIndex = 0; ThreadIndex < ProcessorCoreCount; ThreadIndex += 1) {
        Result = max(ThreadParams[ThreadIndex].MaxOnesObserved, Result);
    }

    std::cout << "The final result is " << Result;

Exit:

    if (ThreadHandles) {
        free(ThreadHandles);
    }

    if (ThreadParams) {
        free(ThreadParams);
    }

    return Error;
}

DWORD
GetPhysicalProcessorCount (
    ULONG &ProcessorCount
    )
{
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* Buffer = NULL;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* Current = NULL;
    DWORD ReturnLength = 0;
    DWORD Error;
    BOOL Ret;

    ProcessorCount = 0;

    //
    // Use GetLogicalProcessorInformationEx to get the number of processors.
    //
    // The first call returns the size needed for the second call.
    //

    Ret = GetLogicalProcessorInformationEx(RelationProcessorCore,
                                           Buffer,
                                           &ReturnLength);

    //
    // GetLogicalProcessorInformationEx must have failed because the provided
    // buffer was 0.
    //

    assert(!Ret);

    Error = GetLastError();

    if (Error != ERROR_INSUFFICIENT_BUFFER) {
        goto Exit;
    }

    Buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(ReturnLength);

    if (!Buffer) {

        //
        // Allocations can fail because the system is out of memory.
        //

        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    Ret = GetLogicalProcessorInformationEx(RelationProcessorCore,
                                           Buffer,
                                           &ReturnLength);

    if (!Ret) {
        Error = GetLastError();
        goto Exit;
    }

    //
    // Start at the top of the Buffer.
    //

    Current = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) Buffer;

    while(ReturnLength != 0) {
        if (Current->Relationship == RelationProcessorCore) {
            ProcessorCount += 1;
        }

        ReturnLength -= Current->Size;

        //
        // Advance the current pointer by the size of the prior item. Must
        // use BYTE math because Size is in Bytes.
        //

        Current = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)
                    ((BYTE*)Current + Current->Size);
    };

    //
    // Success.
    //

    Error = ERROR_SUCCESS;

Exit:

    //
    // Ensure the buffer is not leaked.
    //

    if (Buffer) {
        free(Buffer);
    }

    return Error;
}

DWORD
CalculationWorker (
    void* ThreadParameter
    )
{

    //
    // Entry point for the worker thread.
    //

    LONG64 LocalReaminingIterations;
    LONG64 CurrentBatch;
    LONG BatchNumber;
    LONG RollNumber;
    LONG OnesThisAttempt;
    CALCUATION_WORKER_PARAMS* Params;

    Params = (CALCUATION_WORKER_PARAMS*) ThreadParameter;

    //
    // Using the STL libraries implementation of random because it's so useful.
    //

    std::random_device RandomDevice;
    std::mt19937 Rand(RandomDevice());
    std::uniform_int_distribution<> Roll(1, 4);

    //
    // Keep track of the maximum number of zeros seen per set of rolls.
    //

    LONG MaxSeenByThread = 0;

    UNREFERENCED_PARAMETER(ThreadParameter);

    while (TRUE) {

        //
        // Try to take a BatchSize amount of work from the remaining work.
        //

        LocalReaminingIterations = InterlockedAdd64(&RemainingIterations,
                                                    0 - BatchSize);

        if (LocalReaminingIterations + BatchSize < 0) {

            //
            // All work is already done. Threads should now exit.
            //

            goto Exit;

        } else if (LocalReaminingIterations <= 0) {

            //
            // There was equal or less than the BatchSize amount of work
            // remaining in the queue meaning and this is the last batch.
            //

            CurrentBatch = LocalReaminingIterations + BatchSize;

        } else {

            //
            // There was more work remaining than the batch size.
            //

            CurrentBatch = BatchSize;
        }

        for (BatchNumber = 0; BatchNumber < CurrentBatch; BatchNumber += 1) {

            OnesThisAttempt = 0;

            //
            // Do the rolls for this attempt.
            //

            for (RollNumber = 0; RollNumber < Rolls; RollNumber += 1) {
                if (Roll(Rand) == 1) {
                    OnesThisAttempt += 1;
                }
            }

            //
            // Update the maximum number of ones seen.
            //

            MaxSeenByThread = max(OnesThisAttempt, MaxSeenByThread);
        }
    }

Exit:

    //
    // Write the result into the thread parameters.
    //

    Params->MaxOnesObserved = MaxSeenByThread;

    return ERROR_SUCCESS;
}