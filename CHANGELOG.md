# Change Log

## [1.4] - 2023-05-13

### Changed

* `ldh` instruction now accepts values between 0x00 and 0xff and between 0xff00 and 0xffff

### Fixed

* Being able to print the correct current address after a `data` `from` statement
* Listing with correct addresses in `data` `from` and `data` of a `struct` type statements
* Some GB instructions were not being assembled

## [1.3] - 2022-11-01
 
### Added  

* new instructions `continue`and `continueif`

### Fixed

* VIM syntax not detecting end of preprocessor tags in some circumstances

## [1.2] - 2022-10-23
 
### Added  

* Support for arguments for inline code blocks

### Fixed

* bug when showing certain expression in output listing file

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

