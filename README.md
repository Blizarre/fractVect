# fractVect
Fractal generation using AVX2 instructions

I wanted to see how difficult it was to use intrinsics to speed-up computations. It was much easier than anticipated, and the results are really great with a 6.6X speedup, from 200ms in x86 to 30ms. in AVX2.

At startup, the fractal generator uses only x86 instructions. Press the 'o' key to switch between X86, AVX2 (8 word) and SSE (4 words) instructions.
