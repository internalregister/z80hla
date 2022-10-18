# Change Log
 
## [1.1] - 2022-10-18
 
### Added  

* new instruction `breakif`
* new directive `#assembleall_on` and `#assembleall_off`
* new directive `#jrinloops_on` and `#jrinloops_off`
* new data declaration with `of` keyword
* new separator `.|`
* allow referencing addresses of elements using struct types directly

### Changed
 
* ifs and loops are no longer generated with `jr` instructions by default

### Fixed

* bug assigning values to data or struct elements of type `dword`
* bug of assembler exiting in a non controlled way when no bytes were generated
* bug with address going over 0xFFFF

