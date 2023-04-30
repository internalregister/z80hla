# Change Log

## [1.4] - 2023-04-29

### Fixed

* Being able to print the correct current address after a _data_ _from_ statement
* Listing with correct addresses in _data_ _from_ and _data_ of _struct_ statements

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

