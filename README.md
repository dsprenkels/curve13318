# Curve13318 research

Code written for my master's thesis: https://dsprenkels.com/files/thesis-20190311.pdf

## Dependencies

- `make`
- `nasm`
- `gcc` or `clang`

## Benchmarking instructions

- First, to get reliable benchmarking results, set up your target:
    - Disable TurboBoost
    - Disable all the HyperThreading cores
    - Set the CPU scaling to 'performance'
- Then, run (on an idle machine): `make bench`

